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

#include "gui/GuiGenericWindows.h"

#include "LogSystem.h"
#include "XmlUtils.h"

#include "gui/GuiSystem.h"
#include "gui/utils/CeguiString.h"
#include "gui/InputAggregator4CEGUI.h"

MGE::GenericWindows::BaseWindow* MGE::GenericWindows::Factory::get(const std::string_view& name) {
	auto iter = baseWindowsMap.find(name);
	if (iter != baseWindowsMap.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

MGE::GenericWindows::BaseWindow* MGE::GenericWindows::Factory::get(const std::string_view& name, const std::string_view& type, const CEGUI::String& layout) {
	auto iter = baseWindowsMap.find(name);
	if (iter != baseWindowsMap.end()) {
		return iter->second;
	} else {
		return create(type, layout);
	}
}

/**
@page XMLSyntax_Misc

@subsection XMLNode_BaseWin \<BaseWin\>

@c \<BaseWin\> is used for indicate parent window (@ref MGE::GenericWindows::BaseWindow) for GUI element based on @ref MGE::GenericWindows::BaseWindowOwner. It have 3 attributes:
	- @c name - name of window to use or create
	- @c type - type of window to create, use only when window with @a name do not exist, can be:
		- @c MinimizableWindow - with minimization option - two size window (@ref MGE::GenericWindows::MinimizableWindow)
		- @c ClosableWindow - window with close button (@ref MGE::GenericWindows::ClosableWindow)
		- @c TabsWindow - ClosableWindow with multiple tabs (@ref MGE::GenericWindows::TabsWindow)
	- @c layoutFile - layout filename for creating window, use only when window with @a name do not exist
*/

MGE::GenericWindows::BaseWindow* MGE::GenericWindows::Factory::get(const pugi::xml_node& xmlNode) {
	MGE::GenericWindows::BaseWindow* baseWin = NULL;
	auto baseWinNode = xmlNode.child("BaseWin");
	if (baseWinNode) {
		std::string_view baseWinName   = baseWinNode.attribute("name").as_string();
		std::string_view baseWinType   = baseWinNode.attribute("type").as_string();
		std::string_view baseWinLayout = baseWinNode.attribute("layoutFile").as_string();
		
		baseWin = get(baseWinName, baseWinType, STRING_TO_CEGUI(baseWinLayout));
	}
	return baseWin;
}

MGE::GenericWindows::BaseWindow* MGE::GenericWindows::Factory::create(const std::string_view& type, const CEGUI::String& layout) {
	if (type == "MinimizableWindow") {
		return new MGE::GenericWindows::MinimizableWindow(layout);
	} else if (type == "ClosableWindow") {
		return new MGE::GenericWindows::ClosableWindow(layout);
	} else if (type == "TabsWindow") {
		return new MGE::GenericWindows::TabsWindow(layout);
	}
	return NULL;
}


MGE::GenericWindows::BaseWindow::BaseWindow(const CEGUI::String& layout, const std::string_view& moduleName, CEGUI::Window* parent) {
	LOG_INFO("Create GUIBaseWindow window for " + moduleName + " based on: " + layout);
	
	window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(layout);
	if (parent == NULL) {
		parent  = MGE::GUISystem::getPtr()->getMainWindow();
	}
	parent->addChild(window);
	
	numOfClients = 0;
	MGE::GenericWindows::Factory::getPtr()->baseWindowsMap[STRING_FROM_CEGUI(window->getName())] = this;
}

MGE::GenericWindows::BaseWindow::~BaseWindow(void) {
	MGE::GenericWindows::Factory::getPtr()->baseWindowsMap.erase(STRING_FROM_CEGUI(window->getName()));
	CEGUI::WindowManager::getSingleton().destroyWindow(window);
}

void MGE::GenericWindows::BaseWindow::remClient() {
	if (--numOfClients <= 0) {
		LOG_INFO("Destroy BaseWindow " +  STRING_FROM_CEGUI(window->getName()) + " due to remove last Client");
		delete this;
	}
}

MGE::GenericWindows::MinimizableWindow::MinimizableWindow(const CEGUI::String& layoutFile) : MGE::GenericWindows::BaseWindow(layoutFile) {
	defaultSize = window->getSize();
	normalPosition = window->getPosition();
	hidePosition = CEGUI::PropertyHelper<CEGUI::UVector2>::fromString(window->getUserString("MinimizedPosition"));
	// normalPos + CEGUI::UVector2(window->getWidth() - CEGUI::UDim(0.0, 20), CEGUI::UDim(0.0, 20) - window->getHeight());
	
	window->setPosition(hidePosition);
	hidden = true;
	
	window->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::GenericWindows::MinimizableWindow::handleHide, this)
	);
	
	window->show();
}

void MGE::GenericWindows::MinimizableWindow::show(const CEGUI::String& name) {
	hidden = false;
	window->setPosition(normalPosition);
}

void MGE::GenericWindows::MinimizableWindow::hide() {
	window->setPosition(hidePosition);
	window->setSize(defaultSize);
	hidden = true;
}

bool MGE::GenericWindows::MinimizableWindow::handleHide(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	if (mbargs.d_generatedClickEventOrder == 2) {
		if (hidden) {
			show();
		} else {
			hide();
		}
	}
	return true;
}


MGE::GenericWindows::ClosableWindow::ClosableWindow(const CEGUI::String& layoutFile) : MGE::GenericWindows::BaseWindow(layoutFile) {
	(static_cast<CEGUI::FrameWindow*>(window))->getCloseButton()->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::GenericWindows::ClosableWindow::handleClose, this)
	);
	
	window->hide();
}

bool MGE::GenericWindows::ClosableWindow::handleClose(const CEGUI::EventArgs& args) {
	window->hide();
	return true;
}

MGE::GenericWindows::TabsWindow::TabsWindow(const CEGUI::String& layoutFile) : MGE::GenericWindows::ClosableWindow(layoutFile) {
	LOG_INFO("Creating TabsWindow for: " + layoutFile);
	
	CEGUI::Window* subWin = window->getChild("TabSwitching");
	CEGUI::Window* button;
	for (size_t i = 0; i < subWin->getChildCount(); ++i) {
		button = subWin->getChildAtIndex( i );
		LOG_INFO( "Parsing sub window: " << button->getName() );
		
		button->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::GenericWindows::TabsWindow::handleTabSwitch, this)
		);
		MGE::GUISystem::getPtr()->setTranslatedText(button);
	}
	currTab = NULL;
}

bool MGE::GenericWindows::TabsWindow::handleTabSwitch(const CEGUI::EventArgs& args) {
	const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	
	if (currTab)
		currTab->hide();
	
	currTab = window->getChild( wargs.window->getName() );
	if (currTab) {
		currTab->show();
	}
	return true;
}

bool MGE::GenericWindows::TabsWindow::switchToTab(const CEGUI::String& name) {
	if (currTab)
		currTab->hide();
	
	currTab = window->getChild(name);
	if (currTab) {
		currTab->show();
		return true;
	} else {
		LOG_WARNING("Not found tab: " + name);
		return false;
	}
}
