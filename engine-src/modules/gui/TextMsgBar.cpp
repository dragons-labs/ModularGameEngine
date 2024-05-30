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

#include "modules/gui/TextMsgBar.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "XmlUtils.h"

#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"

#include "physics/TimeSystem.h"
#include "modules/gui/TextInfo.h"

/*--------------------- constructor, destructor, load() and restore() ---------------------*/

MGE::TextMsgBar::TextMsgBar(
	CEGUI::Window* msg_win, MGE::TextReport* log, bool autohide, int refresh, int txtExtraBufSize
) :
	MGE::SaveableToXML<TextMsgBar>(201, 301),
	msgWin(msg_win), refreshPeriod(refresh), logReport(log), autoHideMsgWin(autohide), hasTimer(false), txtOutBufLen(0)
{
	LOG_INFO("Initialise TextMsgBar");
	
	// configure window
	msgWin->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::TextMsgBar::handleClick, this)
	);
	if(autoHideMsgWin) {
		msgWin->hide();
	} else {
		msgWin->show();
	}
	
	// calculate window width in in code points / characters
	txtMinBufSize   = -2;
	int width = msgWin->getPixelSize().d_width;
	int lastHorzExtent = -1, horzExtent = 0;
	while (horzExtent <= width && horzExtent > lastHorzExtent) {
		printf("%d %d %d\n", CEGUI::PropertyHelper<int>::fromString(msgWin->getProperty("HorzExtent")), width, txtMinBufSize);
		++txtMinBufSize;
		txtEmptyBuf.append(" ");
		msgWin->setText(txtEmptyBuf);
		lastHorzExtent = horzExtent;
		horzExtent = CEGUI::PropertyHelper<int>::fromString(msgWin->getProperty("HorzExtent"));
	}
	txtEmptyBuf     = CEGUI::String(txtMinBufSize, ' ');
	txtMinBufSize  += txtExtraBufSize;
	txtOutBuf       = txtEmptyBuf;
}

MGE::TextMsgBar::~TextMsgBar() {
	hasTimer = false;
	txtOutBufLen = 0;
	
	MGE::TimeSystem::getPtr()->gameTimer->stopTimer("INFO_TEXT_MSG_TIMER");
	for (auto& msg : msgQueue) {
		delete msg;
	}
	
	CEGUI::WindowManager::getSingleton().destroyWindow(msgWin);
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_TextMsgBar \<TextMsgBar\>

@c \<TextMsgBar\> is used for used for enabled and configure GUI message bar, have following (optional) subnodes:
	- @c \<ReportName\>   - name of raport used to archive messages (as xml node value), default: empty (disable store messages history)
	- @c \<AutoHide\>     - @ref XML_Bool (as xml node value), when true automatically hide dialog windows while no active dialog, default: true
	- @c \<Refresh\>      - message bar refresh (text scroll) period in ms, default: 200
	- @c \<ExtraBufSize\> - extra size (more than width of the message bar) of output text buffer (in char), default: 0
	- @c \<WinLayout\>    - filename of layout file for dialog window, default: TextMsgBar.layout
*/

MGE::TextMsgBar* MGE::TextMsgBar::create(const pugi::xml_node& xmlNode) {
	LOG_INFO("Load / create TextMsgBar based on config xml node");
	
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
	
	bool autohide     = xmlNode.child("AutoHide").text().as_bool(true);
	int  refresh      = xmlNode.child("Refresh").text().as_int(200);
	int  extraBufSize = xmlNode.child("ExtraBufSize").text().as_int(0);
	
	return new MGE::TextMsgBar(
		MGE::GUISystem::getPtr()->createGUIWindow(
			xmlNode.child("WinLayout").text().as_string("TextMsgBar.layout"), "TextMsgBar", NULL
		),
		log, autohide, refresh, extraBufSize
	);
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(TextMsgBar) {
	return MGE::TextMsgBar::create(xmlNode);
}

bool MGE::TextMsgBar::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	LOG_INFO("store TextMsgBar data");
	
	xmlNode.append_child("hasTimer") <<  hasTimer;
	if (hasTimer) {
		xmlNode.append_child("txtOutBufStr") <<  STRING_FROM_CEGUI(txtOutBuf);
		xmlNode.append_child("txtOutBufLen") <<  txtOutBufLen;
		auto xmlSubNode = xmlNode.append_child("MsgQueue");
		for (const auto& iter : msgQueue) {
			auto xmlSubSubNode = xmlSubNode.append_child("Message");
			iter->storeToXML( xmlSubSubNode, false );
		}
	}
	return true;
}

bool MGE::TextMsgBar::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("restore TextMsgBar data");
	
	hasTimer = xmlNode.child("hasTimer").text().as_bool();
	if (hasTimer) {
		txtOutBuf = xmlNode.child("name").text().as_string();
		txtOutBufLen = xmlNode.child("").text().as_int();
		
		for (auto xmlSubNode : xmlNode.child("MsgQueue").children("Message")) {
			msgQueue.push_back(new Message(xmlSubNode));
		}
		
		msgWin->show();
		MGE::TimeSystem::getPtr()->gameTimer->addTimerCpp(
			refreshPeriod,
			std::bind( &MGE::TextMsgBar::refresh, this ),
			"INFO_TEXT_MSG_TIMER"
		);
	}
	return true;
}

/*--------------------- message bar usage ---------------------*/

void MGE::TextMsgBar::addMessage(const std::string_view& text, int count, int priority, unsigned int colorARGB, const std::string_view& audio) {
	LOG_DEBUG("addMessage: " << text);
	
	auto insertPointIter = msgQueue.end();
	for (auto iter = msgQueue.begin(); iter != msgQueue.end(); ++iter) { // don't use `for(auto& it : set)` because we need get insertPointIter as iterator
		if ((*iter)->wasShown == true || (*iter)->priority > priority) {
			insertPointIter = iter;
			break;
		}
	}
	msgQueue.insert(insertPointIter, new Message(text, count, priority));
	
	if (logReport)
		logReport->addMessage(text);
	
	if (!hasTimer) {
		msgWin->show();
		hasTimer = true;
		MGE::TimeSystem::getPtr()->gameTimer->addTimerCpp(
			refreshPeriod,
			std::bind( &MGE::TextMsgBar::refresh, this ),
			"INFO_TEXT_MSG_TIMER"
		);
	}
}

bool MGE::TextMsgBar::refresh() {
	while (txtOutBufLen < txtMinBufSize) {
		if (!msgQueue.empty()) {
			// get new string from queue
			Message* msg = msgQueue.front();
			msgQueue.pop_front();
			
			// add to txtOutBuf
			txtOutBuf = txtOutBuf + STRING_TO_CEGUI(msg->txt) + ";  [;]";
			
			// calculate bufor length
			txtOutBufLen = txtOutBuf.length();
			size_t tagStart = 0, tagEnd = 0;
			while ((tagStart = txtOutBuf.find("[", tagEnd)) != CEGUI::String::npos) {
				if (txtOutBuf.substr(tagStart-1, 1) != "\\") {
					tagEnd = txtOutBuf.find("]", tagStart);
					if (tagEnd != CEGUI::String::npos) {
						txtOutBufLen -= tagEnd - tagStart + 1;
					} else {
						break;
					}
				} else {
					txtOutBufLen -= 1;
					tagEnd = tagStart + 1;
				}
			}
			
			// add to end of queue or remove
			if (--msg->count > 0) {
				msg->wasShown = true;
				msgQueue.push_back(msg);
			} else {
				delete msg;
			}
		} else if (txtOutBufLen == 1) {
			// no more messages to show ... stopping and clearing
			hasTimer = false;
			if (autoHideMsgWin)
				msgWin->hide();
			txtOutBuf = txtEmptyBuf;
			return false;
		} else {
			break;
		}
	}
	
	msgWin->setText(txtOutBuf);
	
	if (txtOutBuf.substr(0, 1) == "[") {
		size_t pos = 0;
		do {
			pos = txtOutBuf.find("]", pos) + 1;
		} while (txtOutBuf.substr(pos, 1) == "[");
		
		if (txtOutBuf.substr(pos-3, 3) == "[;]") {
			txtOutBuf.erase(0, pos);
		} else if (txtOutBuf.substr(pos, 2) == "\\[") {
			txtOutBuf.erase(pos, 2);
			txtOutBufLen -= 1;
		} else {
			txtOutBuf.erase(pos, 1);
			txtOutBufLen -= 1;
		}
	} else if (txtOutBuf.substr(0, 2) == "\\[") {
		txtOutBuf.erase(0, 2);
		txtOutBufLen -= 1;
	} else {
		txtOutBuf.erase(0, 1);
		txtOutBufLen -= 1;
	}
	
	return true;
}

bool MGE::TextMsgBar::handleClick(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	if (mbargs.d_button == CEGUI::MouseButton::Left && logReport) {
		MGE::TextInfo::getPtr()->setCurrentReport(logReport);
		MGE::TextInfo::getPtr()->show();
	}
	
	return true;
}

/*--------------------- Message struct ---------------------*/

MGE::TextMsgBar::Message::Message(const std::string_view& t, int c, int p) :
	txt      (t),
	count    (c),
	priority (p),
	wasShown (false)
{}

MGE::TextMsgBar::Message::Message(const pugi::xml_node& xmlNode) :
	txt      (xmlNode.child("txt").text().as_string()),
	count    (xmlNode.child("count").text().as_int()),
	priority (xmlNode.child("priority").text().as_int()),
	wasShown (xmlNode.child("wasShown").text().as_bool(false))
{}

bool MGE::TextMsgBar::Message::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_child("txt") << txt;
	xmlNode.append_child("count") << count;
	xmlNode.append_child("priority") << priority;
	xmlNode.append_child("wasShown") << wasShown;
	
	return true;
}
