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

#include "gui/modules/OnScreenInfo.h"

#include "XmlUtils.h"

#include "gui/GuiSystem.h"
#include "gui/utils/CeguiString.h"

#include "physics/GameSpeedMessages.h"
#include "physics/TimeSystem.h"

#include "LogSystem.h"


/**
@page XMLSyntax_MainConfig

@subsection XMLNode_OnScreenInfo \<OnScreenInfo\>

@c \<OnScreenInfo\> is used for setup <b>On Screen Info</b>, have following (optional) subnodes:
  - @c \<ShowOnScreenInfoOnPause\> enable and configure display on pause message via OnScreenInfo. It can contain following (optional) subnodes:
    - @c \<OnScreenInfoPauseText\>
      - text to show in on screen info
      - default "Game Paused"
    - @c \<OnScreenInfoKey\>
      - key value for protecting hide/override on screen info by other system
      - default 0 (no key protection, can be override by other info without providing key)
    - @c \<OnScreenInfoWidth\>
      - width of on screen info window
      - default 0 (autodetect)
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(OnScreenInfo) {
	auto osd = new MGE::OnScreenInfo();
	
	if (auto onPause = xmlNode.child("ShowOnScreenInfoOnPause")) {
		LOG_INFO("Configure OSD on pause info");
		
		std::string onScreenInfoText = MGE::XMLUtils::getValue(onPause.child("OnScreenInfoPauseText"), "Game Paused");
		int onScreenInfoKey = MGE::XMLUtils::getValue(onPause.child("OnScreenInfoWidth"), 0);
		int onScreenInfoWidth = MGE::XMLUtils::getValue(onPause.child("OnScreenInfoWidth"), 0);
		
		auto gameSpeedUpdate = [=] (const MGE::EventMsg*, void *) {
			if (MGE::TimeSystem::getPtr()->gameIsPaused())
				osd->showOnScreenText(onScreenInfoText, onScreenInfoKey, onScreenInfoWidth);
			else
				osd->hideOnScreenText(onScreenInfoKey);
		};
		
		// subscribe for events message
		MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
			MGE::GameSpeedChangeEventMsg::MsgType,
			gameSpeedUpdate,
			osd
		);
		
		gameSpeedUpdate(nullptr, nullptr);
	}
	return osd;
}


MGE::OnScreenInfo::OnScreenInfo() {
	LOG_INFO("Create onScreen info window");
	onScreenInfo = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("OnScreenInfo.layout");
	MGE::GUISystem::getPtr()->getMainWindow()->addChild(onScreenInfo);
	onScreenInfo->hide();
	onScreenInfoCode = 0;
}

MGE::OnScreenInfo::~OnScreenInfo() {
	MGE::GUISystem::getPtr()->getMainWindow()->removeChild(onScreenInfo);
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(this);
}

bool MGE::OnScreenInfo::showOnScreenText(const std::string_view& txt, int code, int width) {
	if (code == -2)
		code = onScreenInfoCode = 0;
	
	if (onScreenInfoCode == 0 || onScreenInfoCode == code) {
		onScreenInfoCode = code;
		onScreenInfo->setText(STRING_TO_CEGUI(txt));
		if (width != 0) {
			onScreenInfo->setWidth(CEGUI::UDim(0.0, width));
		}
		onScreenInfo->show();
		return true;
	} else if (code == -1) {
		if (!onScreenInfo->getText().empty()) {
			onScreenInfo->show();
			return true;
		} else {
			return false;
		}
	} else {
		LOG_WARNING("Can't show OnScreenInfo \"" << txt << "\" - call with diffrent key");
		return false;
	}
}

bool MGE::OnScreenInfo::hideOnScreenText(int code) {
	if (onScreenInfoCode == 0 || onScreenInfoCode == code) {
		onScreenInfoCode = 0;
		onScreenInfo->hide();
		return true;
	} else if (code == -2) {
		onScreenInfo->hide();
	}
	return false;
}

bool MGE::OnScreenInfo::isOnScreenText() {
	return onScreenInfo->isVisible();
}
