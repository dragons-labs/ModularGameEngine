/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "modules/gui/TextDialog.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "XmlUtils.h"
#include "with.h"

#include "ScriptsSystem.h"
#include "physics/TimeSystem.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"

#include "modules/gui/TextInfo.h"

/*--------------------- constructor, destructor, load() and restore() ---------------------*/

MGE::TextDialog::TextDialog(
	CEGUI::Window* win, MGE::TextReport* log, bool autohide, bool autopasue
) :
	MGE::SaveableToXML<TextDialog>(202, 302),
	defaultAutoPause(autopasue), autoHideDialogWin(autohide), logReport(log), dialogWin(win)
{
	LOG_INFO("Initialise TextDialog");
	
	if (autoHideDialogWin)
		dialogWin->hide();
	else
		dialogWin->show();
	
	// textbox window
	textBox = static_cast<CEGUI::MultiLineEditbox*>(dialogWin->getChild( "Text" ));
	textBox->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::TextDialog::handleClick, this)
	);
	textBox->show();
	
	// dialog answer window
	answerBox = dialogWin->getChild( "Answers" );
	answerButton = answerBox->getChild( "Submit" );
	answerButton->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::TextDialog::handleAnswer, this)
	);
	MGE::GUISystem::getPtr()->setTranslatedText(answerButton);
	answerList = static_cast<CEGUI::ListWidget*>(answerBox->getChild( "Answers" ));
	answerBox->hide();
	
	// imagebox window
	imageBox = dialogWin->getChild( "Info" )->getChild( "Image" );
	baseXPosition = textBox->getPosition().d_x;
	baseWidth = textBox->getWidth();
	
	// misc
	currState  = OFF;
	startPause = false;
	#ifdef USE_OGGSOUND
	dialogSound = NULL;
	#endif
}

MGE::TextDialog::TextDialog(const CEGUI::String& dialogWinLayout, MGE::TextReport* log, bool autohide, bool autopause, CEGUI::Window* parent) :
	TextDialog(MGE::GUISystem::getPtr()->createGUIWindow(dialogWinLayout, "TextDialog", parent), log, autohide)
{}

MGE::TextDialog::TextDialog(MGE::TextReport* log, bool autohide, bool autopause, CEGUI::Window* parent) :
	TextDialog("DialogMenu.layout", log, autohide, parent)
{}


MGE::TextDialog::~TextDialog(void) {
	LOG_INFO("destroy TextDialog");
	unsetImage(true, true);
	CEGUI::WindowManager::getSingleton().destroyWindow(dialogWin);
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_TextDialog \<TextDialog\>

@c \<TextDialog\> is used for used for enabled and configure GUI dialog system, have following (optional) subnodes:
	- @c \<ReportName\> - name of raport used to archive dialogs (as xml node value), default: empty (disable store dialogs history)
	- @c \<AutoHide\>   - @ref XML_Bool (as xml node value), when true automatically hide dialog windows while no active dialog, default: true
	- @c \<AutoPause\>  - @ref XML_Bool (as xml node value), when true automatically pause game when start dialog, default: true
	- @c \<WinLayout\>  - filename of layout file for dialog window, default: DialogMenu.layout
*/

MGE::TextDialog* MGE::TextDialog::create(const pugi::xml_node& xmlNode) {
	LOG_INFO("Load / create TextDialog based on config xml node");
	
	MGE::TextReport* log = NULL;
	std::string_view reportName = xmlNode.child("ReportName").text().as_string();
	if (!reportName.empty()) {
		MGE::TextInfo* textInfo = MGE::TextInfo::getPtr();
		if (!textInfo) {
			LOG_ERROR("not empty ReportName for TextMsgBar, but TextInfo does not exist");
		} else {
			log = textInfo->getReport(reportName);
		}
	}
	
	bool autohide                 = xmlNode.child("AutoHide").text().as_bool(true);
	int  autopause                = xmlNode.child("AutoPause").text().as_bool(true);
	
	return new MGE::TextDialog(
		MGE::GUISystem::getPtr()->createGUIWindow(
			xmlNode.child("WinLayout").text().as_string("DialogMenu.layout"), "TextDialog", NULL
		),
		log, autohide, autopause
	);
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(TextDialog) {
	return MGE::TextDialog::create(xmlNode);
}

bool MGE::TextDialog::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	LOG_INFO("restore TextDialog data");
	
	currScript = xmlNode.child("currScript").text().as_string();
	currStep = xmlNode.child("currStep").text().as_int();
	currImage = xmlNode.child("currImage").text().as_string();
	currImageGroup = xmlNode.child("currImageGroup").text().as_string();
	
	currState  = OFF;
	
	if (!currScript.empty())
		runDialog(currScript, currStep);
	
	if (!currImage.empty())
		setImage(currImage, currImageGroup);
	
	return true;
}

bool MGE::TextDialog::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	LOG_INFO("store TextDialog data");
	
	xmlNode.append_child("currScript") << currScript;
	xmlNode.append_child("currStep")   << currStep;
	xmlNode.append_child("currImage")      << STRING_FROM_CEGUI(currImage);
	xmlNode.append_child("currImageGroup") << STRING_FROM_CEGUI(currImageGroup);
	
	return true;
}

/*--------------------- dialog creating and running ---------------------*/

void MGE::TextDialog::runDialog(const std::string_view& script, int step, bool autopause) {
	LOG_INFO("runDialog");
	if (currState == WAIT_FOR_ANSWER) {
		answerBox->hide();
		answerList->clearList();
	} else if (currState == SHOW_TEXT) {
		textBox->hide();
		textBox->setText("");
	}
	
	MGE::TimeSystem::getPtr()->realtimeTimer->stopTimer("DIALOG_MENU_TIMER");
	
	if (!script.empty()) {
		LOG_INFO("run script: " << script << " with: " << step);
		
		currScript = script;
		currStep   = step;
		currState  = RUN_SCRIPT;
		
		if (autoHideDialogWin)
			dialogWin->show();
		
		if (!startPause && autopause) {
			startPause = true;
			MGE::TimeSystem::getPtr()->pause();
		}
		
		MGE::ScriptsSystem::getPtr()->runObjectWithVoid( currScript.c_str(), currStep );
	} else {
		LOG_INFO("end dialog");
		
		currScript.clear();
		currState  = OFF;
		unsetImage();
		
		if (autoHideDialogWin)
			dialogWin->hide();
		
		if (startPause) {
			MGE::TimeSystem::getPtr()->unpause();
		}
	}
}

void MGE::TextDialog::runDialog(const std::string_view& initScript, int initStep) {
	runDialog(initScript, initStep, defaultAutoPause);
}

void MGE::TextDialog::showText(const std::string_view& text, const  Ogre::String& audio, int timeout, const std::string_view& callbackScript, int step) {
	LOG_INFO("showText: " + text + " audio=" + audio);
	currState = SHOW_TEXT;
	
	textBox->setText(STRING_TO_CEGUI(text));
	textBox->show();
	
	if (logReport && !text.empty())
		logReport->addMessage(text);
	
	#ifdef USE_OGGSOUND
	WITH_NOT_NULL(MGE::AudioSystem::getPtr(), WITH_AS, !audio.empty()) {
		LOG_INFO("Dialog menu play sound: " + audio);
		WITH_AS->destroySound(dialogSound);
		dialogSound = WITH_AS->createSound("DialogSound", audio);
		if (dialogSound) {
			WITH_AS->setSoundAsBackground(dialogSound, 1.0);
			dialogSound->play();
		}
	}
	#endif
	
	LOG_DEBUG("set next script: " << callbackScript << " with step=" << step);
	nextScript = callbackScript;
	nextStep   = step;
	
	if (timeout != 0) {
		LOG_DEBUG("start timer with timeout=" << timeout);
		MGE::TimeSystem::getPtr()->realtimeTimer->addTimerCpp(
			timeout,
			std::bind( &MGE::TextDialog::handleTimer, this ),
			"DIALOG_MENU_TIMER", false, false
		);
	}
}

void MGE::TextDialog::addAnswer(const CEGUI::String& text, int id) {
	CEGUI::StandardItem* item = new CEGUI::StandardItem(text, id);
	answerList->addItem( item );
}

void MGE::TextDialog::showAnswers(const std::string_view& callbackScript) {
	LOG_INFO("showAnswers, next script: " << callbackScript);
	
	currState = WAIT_FOR_ANSWER;
	answerBox->show();
	nextScript = callbackScript;
}


void MGE::TextDialog::setImage(const CEGUI::String& name, const CEGUI::String& group) {
	LOG_INFO("setImage: " << name);
	currImage = name;
	currImageGroup = group;
	
	if (!imageBox->getParent()->isVisible()) {
		CEGUI::UDim newXPosition = imageBox->getParent()->getHeight() + baseXPosition;
		CEGUI::UDim newWidth = baseWidth - newXPosition;
		
		textBox->setWidth(newWidth);
		textBox->setXPosition(newXPosition);
		answerBox->setWidth(newWidth);
		answerBox->setXPosition(newXPosition);
		imageBox->getParent()->show();
	}
	
	if (!CEGUI::ImageManager::getSingleton().isDefined(currImage)) {
		CEGUI::ImageManager::getSingleton().addBitmapImageFromFile(
			currImage,
			currImage,
			currImageGroup
		);
	}
	
	imageBox->setProperty("Image", currImage);
}

void MGE::TextDialog::unsetImage(bool hide, bool unload) {
	LOG_INFO("unsetImage");
	currImage.clear();
	
	if (hide) {
		imageBox->getParent()->hide();
		textBox->setXPosition(baseXPosition);
		textBox->setWidth(baseWidth);
		answerBox->setXPosition(baseXPosition);
		answerBox->setWidth(baseWidth);
	}
	
	if (unload) {
		CEGUI::ImageManager::getSingleton().destroy(imageBox->getProperty("Image"));
	}
	
	imageBox->setProperty("Image", "");
}


bool MGE::TextDialog::handleClick(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	if (mbargs.d_button == CEGUI::MouseButton::Left) {
		if (logReport) {
			MGE::TextInfo::getPtr()->setCurrentReport(logReport);
			MGE::TextInfo::getPtr()->show();
		}
	} else if (mbargs.d_button == CEGUI::MouseButton::Right && currState == SHOW_TEXT) {
		runDialog(nextScript, nextStep);
	}
	
	return true;
}

bool MGE::TextDialog::handleAnswer(const CEGUI::EventArgs&) {
	LOG_INFO("handleAnswer");
	CEGUI::StandardItem* item = answerList->getFirstSelectedItem();
	if (item) {
		const CEGUI::String& text = item->getText();
		if (logReport && !text.empty())
			logReport->addMessage(STRING_FROM_CEGUI(text));
		runDialog(nextScript, item->getId());
	}
	return true;
}

bool MGE::TextDialog::handleTimer() {
	runDialog(nextScript, nextStep);
	return false;
}


