/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "modules/gui/TextInfo.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "XmlUtils.h"

#include "gui/utils/CeguiString.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "modules/rendering2texture/WebBrowser.h"

/*--------------------- TextReport ---------------------*/

MGE::TextReport::TextReport(const std::string_view& _name, ReportType _type) :
	name               (_name),
	type               (_type),
	msgPerPage         (20),
	autoSplit          (false),
	wordWrap           (true),
	noDuplicatedOnPrev (false),
	displayFromBack    (false),
	addToFront         (true),
	defaultAutoNewLine (true)
{}

void MGE::TextReport::addMessage(const std::string_view& msg, bool autoNewLine) {
	std::string surfix;
	if (type == MGE::TextReport::TXT && msg != "[pagebreak]") {
		surfix = "[;]";
		if (autoNewLine && msg.substr(msg.length()-1, 1) != "\n")
			surfix += "\n";
	}
	
	if (addToFront)
		entries.push_front(msg + surfix);
	else
		entries.push_back(msg + surfix);
	
	MGE::TextInfo::getPtr()->onReportUpdate(this);
}

/**
@page XMLSyntax_Misc

@subsection XMLNode_StoreReport Report store/restore syntax

Report is stored as @c \<Report\> xml node with next subnodes:
	- @c \<name\> unique name of report
	- @c \<type\> type of of report, integer value (see @ref MGE::TextReport::ReportType)
	- @c \<msgPerPage\> number of message per page (when using autoSplit should be big, default 20)
	- @c \<autoSplit\> automatic split report per pages (@ref XML_Bool, default: false)
	- @c \<wordWrap\> automatic word wrap (@ref XML_Bool, default: true)
	- @c \<noDuplicatedOnPrev\> block print duplicated entries on goto previous page (@ref XML_Bool, default: false)
	- @c \<displayFromBack\> when true display report in reverse order (@ref XML_Bool, default: false)
	- @c \<addToFront\> when true add message at begin of list (@ref XML_Bool, default: true)
	- @c \<defaultAutoNewLine\> default value of automatic add new line at end of adding message in addMessage (@ref XML_Bool, default: true)
	- @c \<header\> header string for HTML report
	- @c \<footer\> footer string for HTML report
	- @c \<entries\> set of @c \<item\> subnodes with raport entries

@ref XMLNode_TextInfoExample : @c \<Report\> nodes inside @c \<ReportsList\>.
*/

bool MGE::TextReport::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	xmlNode.append_child("name") << name;
	xmlNode.append_child("type") << static_cast<int>(type);
	xmlNode.append_child("msgPerPage") << msgPerPage;
	xmlNode.append_child("autoSplit") << autoSplit;
	xmlNode.append_child("wordWrap") << wordWrap;
	xmlNode.append_child("noDuplicatedOnPrev") << noDuplicatedOnPrev;
	xmlNode.append_child("displayFromBack") << displayFromBack;
	xmlNode.append_child("addToFront") << addToFront;
	xmlNode.append_child("defaultAutoNewLine") << defaultAutoNewLine;
	xmlNode.append_child("header") << header;
	xmlNode.append_child("footer") << footer;
	xmlNode.append_child("entries") << entries;
	return true;
}

bool MGE::TextReport::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	type = static_cast<ReportType>( MGE::XMLUtils::getValue<int>(xmlNode.child("type"), type) );
	msgPerPage         = MGE::XMLUtils::getValue(xmlNode.child("msgPerPage"),         msgPerPage);
	autoSplit          = MGE::XMLUtils::getValue(xmlNode.child("autoSplit"),          autoSplit);
	wordWrap           = MGE::XMLUtils::getValue(xmlNode.child("wordWrap"),           wordWrap);
	noDuplicatedOnPrev = MGE::XMLUtils::getValue(xmlNode.child("noDuplicatedOnPrev"), noDuplicatedOnPrev);
	displayFromBack    = MGE::XMLUtils::getValue(xmlNode.child("displayFromBack"),    displayFromBack);
	addToFront         = MGE::XMLUtils::getValue(xmlNode.child("addToFront"),         addToFront);
	defaultAutoNewLine = MGE::XMLUtils::getValue(xmlNode.child("defaultAutoNewLine"), defaultAutoNewLine);
	
	header = xmlNode.child("header").text().as_string(header.c_str());
	footer = xmlNode.child("footer").text().as_string(footer.c_str());
	
	auto entryNode=xmlNode.child("entries");
	if (entryNode) {
		MGE::XMLUtils::getListOfValues<std::string>(entryNode, &entries);
	}
	return true;
}


/*--------------------- TextInfo : constructor, destructor, load() ---------------------*/

MGE::TextInfo::TextInfo(MGE::GenericWindows::BaseWindow* _baseWin, const std::string_view& _winName) :
	MGE::GenericWindows::BaseWindowOwner(_baseWin),
	MGE::SaveableToXML<TextInfo>(701, 303),
	winName(_winName)
{
	CEGUI::Window* win = getWindow()->getChild(STRING_TO_CEGUI(winName));
	
	LOG_INFO("Initialise TextInfo");
	
	textBox = win->getChild( "TextReport" );
	textBox->subscribeEvent(
		CEGUI::Window::EventSized, CEGUI::Event::Subscriber(&MGE::TextInfo::handleResize, this)
	);
	
	htmlBox = win->getChild( "HtmlReport" );
	htmlBoxSize = htmlBox->getPixelSize();
	
	htmlBrowser = new MGE::WebBrowser("HtmlReport", htmlBoxSize.d_width, htmlBoxSize.d_height, "about:blank", MGE::InteractiveTexture::OnGUIWindow);
	try {
		htmlBox->setProperty("Image", STRING_TO_CEGUI(htmlBrowser->getImageName()));
	} catch (...) {}
	
	nextButton  =   static_cast<CEGUI::PushButton*>(win->getChild( "Next" ));
	nextButton->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::TextInfo::handleNext, this)
	);
	MGE::GUISystem::getPtr()->setTranslatedText(nextButton);
	
	prevButton  =   static_cast<CEGUI::PushButton*>(win->getChild( "Prev" ));
	prevButton->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::TextInfo::handlePrev, this)
	);
	MGE::GUISystem::getPtr()->setTranslatedText(prevButton);
	
	reportSelection = static_cast<CEGUI::Combobox*>(win->getChild( "ReportName" ));
	reportSelection->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::TextInfo::handleRaportName, this)
	);
	reportSelection->getDropList()->setTextColour(
		CEGUI::PropertyHelper<CEGUI::Colour>::fromString( reportSelection->getProperty("DefaultItemTextColour") )
	);
	
	currentReport = NULL;
}

MGE::TextInfo::~TextInfo() {
	LOG_INFO("destroy TextDialog");
	currentReport = NULL;
	for (auto& iter : reports) {
		delete iter;
	}
	reports.clear();
	
	// window->remClient() is in (automatic called) BaseWindowOwner destructor ... and can destroy baseWin too
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_TextInfo \<TextInfo\>

@c \<TextInfo\> is used for enabled and configure GUI (sub)widow with text/html raports. It have subnodes:
	- @c \<WinLayout\> with name of layout file (default empty)
	- @ref XMLNode_BaseWin (used when @c \<WinLayout\> is not set or is empty string)
	- @c \<WinName\> with name of created window (default TextInfo)
	- @c \<Content\> with initial reports content, it have subnodes:
		- @c \<ReportsList\> with set of @c \<Report\> nodes, each @c \<Report\> node use @ref XMLNode_StoreReport
		- @c \<currentReportName\> with name of active (currently show) report

@subsubsection XMLNode_TextInfoExample Example
@code{.xml}
	<TextInfo>
		<BaseWin name="WorldInfoWindow" type="TabsWindow" layoutFile="WorldInfoWindow.layout" />
		<!--
		<WinName>Name of BaseWin SubWin</WinName>
		<WinLayout>layout filename for own window instead of BaseWin, to be used can't have BaseWin tag</WinLayout>
		-->
		<Content> <!-- full TextInfo save syntax is supported -->
			<ReportsList>
				<Report>
					<name>html log</name>
					<type>1</type>
					<displayFromBack>1</displayFromBack>
					<addToFront>1</addToFront>
					<header>&lt;html&gt;&lt;body&gt;</header>
					<footer>&lt;/html&gt;&lt;/body&gt;</footer>
					<entries>
						<item>&lt;h1&gt;Hello World 1 !!!&lt;/h1&gt;</item>
						<item>[pagebreak]</item>
						<item>&lt;h2&gt;AA AA&lt;/h2&gt;&lt;h3&gt;BB BB&lt;/h3&gt;</item>
					</entries>
				</Report>
				<Report>
					<name>txt log</name>
					<entries>
						<item>cdd[image-width='100'][aspect-lock='true'][image='FireTruck_B.png']yyy\n\naa[image-width='100'][image-height='10000'][image='FireChief.png'][;]\n\nBB[image='FireChief.png']</item>
						<item>[colour='ff00ff00']Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod</item>
						<item>tempor[colour='FFFF0000'] \\[incididunt]ut [font='DejaVuMono-bold']labore[;] et dolore magna aliqua.</item>
					</entries>
				</Report>
				<Report>
					<name>url log</name>
					<type>2</type>
					<entries>
						<item>http://www.opcode.eu.org/</item>
						<item>[pagebreak]</item>
						<item>https://www.ogre3d.org/</item>
					</entries>
				</Report>
			</ReportsList>
			<currentReportName>html log</currentReportName>
		</Content>
	</TextInfo>
@endcode <br/>
*/

MGE::TextInfo* MGE::TextInfo::create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("Load / create TextInfo based on config xml node");
	MGE::TextInfo* textInfoPtr;
	
	auto _layoutName = xmlNode.child("WinLayout").text().as_string();
	auto _winName = xmlNode.child("WinName").text().as_string("TextInfo");
	
	if (_layoutName[0] != '\0') {
		textInfoPtr = new MGE::TextInfo( new MGE::GenericWindows::BaseWindow(_layoutName, "TextInfo", NULL), _winName );
	} else {
		MGE::GenericWindows::BaseWindow* baseWin = MGE::GenericWindows::Factory::getPtr()->get(xmlNode);
		if(!baseWin) {
			throw std::logic_error("Could not create base window for TextInfo");
		}
		textInfoPtr = new MGE::TextInfo( baseWin, _winName );
	}
	
	auto contentNode = xmlNode.child("Content");
	if (contentNode) {
		textInfoPtr->restoreFromXML( contentNode, context );
	}
	return textInfoPtr;
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(TextInfo) {
	return MGE::TextInfo::create(xmlNode, context);
}

/*--------------------- TextInfo : store/restore ---------------------*/

bool MGE::TextInfo::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const  {
	LOG_INFO("store TextInfo data");
	
	auto xmlSubNode = xmlNode.append_child("ReportsList");
	for (const auto& iter : reports) {
		auto xmlSubSubNode = xmlSubNode.append_child("Report");
		iter->storeToXML( xmlSubSubNode, false );
	}
	
	if (currentReport)
		xmlNode.append_child("CurrentReportName") <<  currentReport->name;
	
	return true;
}

bool MGE::TextInfo::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	if (context->preLoad)
		return false;
	
	LOG_INFO("restore TextInfo data");
	
	for (auto xmlSubNode : xmlNode.child("ReportsList").children("Report")) {
		std::string_view reportName = xmlSubNode.child("name").text().as_string();
		if (reportName.empty())
			continue;
		MGE::TextReport* report = getReport(reportName);
		report->restoreFromXML( xmlSubNode, nullptr );
	}
	
	setCurrentReport( xmlNode.child("CurrentReportName").text().as_string() );
	
	return true;
}


/*--------------------- TextInfo : other stuff ---------------------*/

void MGE::TextInfo::show(const CEGUI::String& name) {
	if (name.empty())
		window->show(winName);
	else
		window->show(name);
}

MGE::TextReport* MGE::TextInfo::getReport(const std::string_view& name, bool create) {
	for (auto& iter : reports) {
		if (iter->name == name) {
			return iter;
		}
	}
	
	if (create) {
		LOG_INFO("Create new TextReport with name: " + name);
		MGE::TextReport* report = new MGE::TextReport(name);
		reports.push_back( report );
		CEGUI::StandardItem* item = new CEGUI::StandardItem(STRING_TO_CEGUI(name), 0);
		reportSelection->addItem( item );
		return report;
	} else {
		return NULL;
	}
}

bool MGE::TextInfo::setCurrentReport(const std::string_view& name) {
	for (auto& iter : reports) {
		if (iter->name == name) {
			setCurrentReport(iter);
			return true;
		}
	}
	
	return false;
}

void MGE::TextInfo::setCurrentReport(MGE::TextReport* report) {
	currentReport = report;
	reportSelection->setText(STRING_TO_CEGUI(report->name));
	
	switch(currentReport->type) {
		case MGE::TextReport::TXT:
			textBox->show();
			htmlBox->hide();
			break;
		case MGE::TextReport::HTML:
		case MGE::TextReport::URL:
			textBox->hide();
			htmlBox->show();
			break;
	}
	initReport();
}

void MGE::TextInfo::onReportUpdate(MGE::TextReport* report, bool force) {
	if (currentReport == report) {
		if (prevButton->isDisabled() || force)
			initReport();
	}
}

void MGE::TextInfo::initReport() {
	if (currentReport->wordWrap)
		textBox->setProperty("HorzFormatting", "WordWrapLeftAligned");
	else
		textBox->setProperty("HorzFormatting", "LeftAligned");
	
	if (currentReport->displayFromBack) {
		pagedTextEnd = currentReport->entries.end();
	} else {
		pagedTextEnd = currentReport->entries.begin();
	}
	if (!currentReport->entries.empty()) {
		nextButton->enable();
		printNext();
	} else {
		textBox->setText("");
		nextButton->disable();
	}
	prevButton->disable();
}

void MGE::TextInfo::display(const CEGUI::String& outBuf) {
	switch(currentReport->type) {
		case MGE::TextReport::TXT:
			LOG_DEBUG("TextInfo render TEXT: " << STRING_FROM_CEGUI(outBuf));
			textBox->setText(outBuf);
			return;
		case MGE::TextReport::HTML:
			LOG_DEBUG("TextInfo render HTML: " << currentReport->header + STRING_FROM_CEGUI(outBuf) + currentReport->footer);
			htmlBrowser->loadString(currentReport->header + STRING_FROM_CEGUI(outBuf) + currentReport->footer);
			return;
		case MGE::TextReport::URL:
			LOG_DEBUG("TextInfo render URL:  " << STRING_FROM_CEGUI(outBuf));
			htmlBrowser->loadURL(STRING_FROM_CEGUI(outBuf));
			return;
	}
}

std::list<std::string>::iterator MGE::TextInfo::goForward(std::list<std::string>::iterator iter, int count, CEGUI::PushButton* btn, bool show, bool noAutoSplit, std::list<std::string>::iterator end) {
	CEGUI::String outBuf;
	
	for (int i=0; i<count; ++i) {
		std::string_view entryTxt(*iter);
		
		if (entryTxt == "[pagebreak]") {
			if (++iter == currentReport->entries.end() && btn) { // [pagebreak] is last element
				btn->disable();
				break;
			} else if (show) {
				break;
			} else if (i != 0) {	// when only move forward list iterator
				--iter;             // we must stop on [pagebreak] (or before it), not on element after [pagebreak]
				break;
			}
		}
		
		if (noAutoSplit || i==0) {
			outBuf += STRING_TO_CEGUI(entryTxt);
		} else {
			// simulate write, check VertExtent and if OK append iter to outBuf
			textBox->setText(outBuf + STRING_TO_CEGUI(entryTxt));
			if (CEGUI::PropertyHelper<int>::fromString(textBox->getProperty("VertExtent")) >= CEGUI::PropertyHelper<int>::fromString(textBox->getProperty("VertScrollPageSize"))) {
				break;
			}
			outBuf += STRING_TO_CEGUI(entryTxt);
		}
		
		if (++iter == end) {
			if (btn && iter == currentReport->entries.end())
				btn->disable();
			break;
		}
	}
	
	if (show) {
		display(outBuf);
	} else if (!noAutoSplit) {
		textBox->setText("");
	}
	
	return iter;
}

std::list<std::string>::iterator MGE::TextInfo::goBack(std::list<std::string>::iterator iter, int count, CEGUI::PushButton* btn, bool show, bool noAutoSplit, std::list<std::string>::iterator end) {
	CEGUI::String outBuf;
	
	for (int i=0; i<count; ++i) {
		--iter;
		std::string_view entryTxt(*iter);
		
		if (entryTxt == "[pagebreak]") {
			if (iter == currentReport->entries.begin() && btn) { // [pagebreak] is first element
				btn->disable();
				break;
			} else if (show) {
				break;
			} else if (i != 0) {	// when only move back list iterator
				++iter;             // we must stop after on element after [pagebreak] (not on [pagebreak] or previous element)
				break;
			}
		}
		
		if (noAutoSplit || i==0) {
			outBuf += STRING_TO_CEGUI(entryTxt);
		} else {
			// simulate write, check VertExtent and if OK append iter to outBuf
			textBox->setText(outBuf + STRING_TO_CEGUI(entryTxt));
			if (CEGUI::PropertyHelper<int>::fromString(textBox->getProperty("VertExtent")) >= CEGUI::PropertyHelper<int>::fromString(textBox->getProperty("VertScrollPageSize"))) {
				// compare with VertScrollPageSize in any step, because VertScrollPageSize changes when show HorzScrollBar
				// alternative: textBox->getChild("__auto_vscrollbar__")->isVisible()
				++iter;
				break;
			}
			outBuf += STRING_TO_CEGUI(entryTxt);
		}
		
		if (iter == end) {
			if (btn && iter == currentReport->entries.begin())
				btn->disable();
			break;
		}
	}
	
	if (show) {
		display(outBuf);
	} else if (!noAutoSplit) {
		textBox->setText("");
	}
	
	return iter;
}


void MGE::TextInfo::printNext() {
	bool noAutoSplit;
	switch(currentReport->type) {
		case MGE::TextReport::TXT:
			textBox->setText("");
			noAutoSplit = !currentReport->autoSplit;
			break;
		case MGE::TextReport::HTML:
		case MGE::TextReport::URL:
			noAutoSplit = true;
			break;
	}
	prevButton->enable();
	
	pagedTextStart = pagedTextEnd;
	if (currentReport->displayFromBack) {
		pagedTextEnd   = goBack(pagedTextEnd, currentReport->msgPerPage, nextButton, true, noAutoSplit);
	} else {
		pagedTextEnd   = goForward(pagedTextEnd, currentReport->msgPerPage, nextButton, true, noAutoSplit);
	}
}

void MGE::TextInfo::printPrev() {
	bool noAutoSplit;
	switch(currentReport->type) {
		case MGE::TextReport::TXT:
			textBox->setText("");
			noAutoSplit = !currentReport->autoSplit;
			break;
		case MGE::TextReport::HTML:
		case MGE::TextReport::URL:
			noAutoSplit = true;
			break;
	}
	nextButton->enable();
	
	std::list<std::string>::iterator endIter;
	if (currentReport->displayFromBack) {
		if (currentReport->noDuplicatedOnPrev)
			endIter = pagedTextStart;
		else
			endIter = currentReport->entries.begin();
		pagedTextStart = goForward(pagedTextStart, currentReport->msgPerPage, prevButton, false, noAutoSplit);
		pagedTextEnd   = goBack(pagedTextStart, currentReport->msgPerPage, NULL, true, noAutoSplit, endIter);
	} else {
		if (currentReport->noDuplicatedOnPrev)
			endIter = pagedTextStart;
		else
			endIter = currentReport->entries.end();
		pagedTextStart = goBack(pagedTextStart, currentReport->msgPerPage, prevButton, false, noAutoSplit);
		pagedTextEnd   = goForward(pagedTextStart, currentReport->msgPerPage, NULL, true, noAutoSplit, endIter);
	}
}

bool MGE::TextInfo::handleRaportName(const CEGUI::EventArgs& args) {
	setCurrentReport(reportSelection->getText().c_str());
	return true;
}

bool MGE::TextInfo::handleResize(const CEGUI::EventArgs& args) {
	LOG_DEBUG("handleResize " << textBox->isVisible() << " " << textBox->isEffectiveVisible() << " / " << htmlBox->isVisible() << " " << htmlBox->isEffectiveVisible());
	
	if (textBox->isEffectiveVisible()) {
		if (currentReport->autoSplit) {
			textBox->setText("");
			nextButton->enable();
			if (currentReport->displayFromBack) {
				pagedTextEnd   = goBack(pagedTextStart, currentReport->msgPerPage, nextButton, true, false);
			} else {
				pagedTextEnd   = goForward(pagedTextStart, currentReport->msgPerPage, nextButton, true, false);
			}
		}
		return true;
	}
	if (htmlBox->isEffectiveVisible()) {
		CEGUI::Sizef newSize = htmlBox->getPixelSize();
		float x = newSize.d_width - htmlBoxSize.d_width;
		float y = newSize.d_height - htmlBoxSize.d_height;
		if (x<-16 || x>16 || y<-16 || y>16)  {
			htmlBoxSize = newSize;
			htmlBrowser->resize(htmlBoxSize.d_width, htmlBoxSize.d_height);
		}
		return true;
	}
	return false;
}

bool MGE::TextInfo::handleNext(const CEGUI::EventArgs& args) {
	printNext();
	return true;
}

bool MGE::TextInfo::handlePrev(const CEGUI::EventArgs& args) {
	printPrev();
	return true;
}
