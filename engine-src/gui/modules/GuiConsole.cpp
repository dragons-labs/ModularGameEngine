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

#include "gui/modules/GuiConsole.h"

#include "LogSystem.h"
#include "ScriptsSystem.h"
#include "ConfigParser.h"

#include "input/InputSystem.h"
#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"

MGE::GUIConsole::GUIConsole() {
	LOG_INFO("Initialise GUIConsole");
	
	consoleWin = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("Console.layout");
	MGE::GUISystem::getPtr()->getMainWindow()->addChild(consoleWin);
	
	gEditbox = static_cast<CEGUI::MultiLineEditbox*>(consoleWin->getChild( "Command" ));
	gHistoryBox = static_cast<CEGUI::MultiLineEditbox*>(consoleWin->getChild( "History" ));
	gEnterToSubmit = static_cast<CEGUI::ToggleButton*>(consoleWin->getChild( "EnterToSubmit" ));
	
	consoleWin->getChild( "Submit" )->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::GUIConsole::handleSubmit, this));
	
	MGE::GUISystem::getPtr()->setTranslatedText(consoleWin);
	MGE::GUISystem::getPtr()->setTranslatedText(gEnterToSubmit);
	MGE::GUISystem::getPtr()->setTranslatedText(consoleWin->getChild( "Submit" ));
	
	static_cast<CEGUI::FrameWindow*>(consoleWin)->getCloseButton()->subscribeEvent(
		CEGUI::PushButton::EventClicked,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::handleHide, this)
	);
	
	gEditbox->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::mouseUp, this)
	);
	
	gHistoryBox->subscribeEvent(
		CEGUI::EditboxBase::EventTextSelectionChanged,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::selectionChanged, this)
	);
	gEditbox->subscribeEvent(
		CEGUI::EditboxBase::EventTextSelectionChanged,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::selectionChanged, this)
	);
	
	consoleWin->subscribeEvent(
		CEGUI::Window::EventKeyDown,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::keyDown, this)
	);
	
	gEditbox->subscribeEvent(
		CEGUI::Window::EventKeyDown,
		CEGUI::Event::Subscriber(&MGE::GUIConsole::editboxKeyDown, this)
	);
	
	gPythonMode     = false;
	gHistory        = &gHistoryStandard;
	gHistoryCurrent = gHistory->end();
	
	MGE::ScriptsSystem::getPtr()->setScriptOutputListener(
		"CONSOLE",
		std::bind(&MGE::GUIConsole::getPythonOutput, this, std::placeholders::_1)
	);
	
	MGE::InputSystem::getPtr()->hightPriorityKeyPressedListener.addListener(
		MGE::InputSystem::KeyPressedListenerFunctor(std::bind(&MGE::GUIConsole::priorityKeyPressed, this, std::placeholders::_1), reinterpret_cast<uintptr_t>(this)),
		64
	);
	
	consoleWin->hide();
}

MGE::GUIConsole::~GUIConsole() {
	LOG_INFO("Destroy GUIConsole");
	MGE::GUISystem::getPtr()->getMainWindow()->removeChild(consoleWin);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_GUIConsole \<GUIConsole\>

@c \<GUIConsole\> is used for setup <b>Game Console (GUI based)</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(GUIConsole) {
	return new MGE::GUIConsole();
}


void MGE::GUIConsole::toggleVisibility() {
	if (consoleWin->isVisible()) {
		hide();
	} else {
		show();
	}
}

void MGE::GUIConsole::show() {
	LOG_DEBUG("show console");
	consoleWin->show();
	gEditbox->activate();
}

void MGE::GUIConsole::hide() {
	LOG_DEBUG("hide console");
	consoleWin->hide();
}

bool MGE::GUIConsole::priorityKeyPressed(const OIS::KeyEvent& arg) {
	if (arg.key == OIS::KC_SCROLL && !MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift)) { // ScrollLock          ==> show / hide console
		toggleVisibility();
		return true;
	}
	return false;
}

void MGE::GUIConsole::addTextToConsole(const CEGUI::String& text, bool addNewLine) {
	// append new text to history output and scroll history output
	if (addNewLine)
		gHistoryBox->setText(gHistoryBox->getText() + text + '\n');
	else
		gHistoryBox->setText(gHistoryBox->getText() + text);
	gHistoryBox->setCaretIndex(static_cast<size_t>(-1));
}

void MGE::GUIConsole::addConsoleCmd(const std::string& key, const std::string& desc, CmdDelegate f) {
	ConsoleCommand* cmd = &(gConsoleCommandMap[key]);
	cmd->desc = desc;
	cmd->sf = NULL;
	cmd->f = f;
}
void MGE::GUIConsole::addConsoleCmd(const std::string& key, const std::string& desc, CmdDelegateStatic f, void* a) {
	ConsoleCommand* cmd = &(gConsoleCommandMap[key]);
	cmd->desc = desc;
	cmd->sf = f;
	cmd->sfa = a;
}

void MGE::GUIConsole::addConsoleScript(const std::string& key, const std::string& desc, const std::string& script) {
	ConsoleCommand* cmd = &(gConsoleCommandMap[key]);
	cmd->desc = desc;
	cmd->sn = script;
}

void MGE::GUIConsole::getPythonOutput(const std::string& str) {
	addTextToConsole(STRING_TO_CEGUI(str), false);
	//LOG_INFO("python say: " << str);
}

bool MGE::GUIConsole::parseCmd(CEGUI::String cmd_args) {
	CEGUI::String cmd, args;
	CEGUI::String::size_type pos = cmd_args.find(" ");
	CEGUI::String::size_type len = cmd_args.length();
	if (pos != CEGUI::String::npos) {
		cmd  = cmd_args.substr(0, pos);
		args = cmd_args.substr(pos+1, len-(pos+1));
	} else {
		cmd  = cmd_args.substr(0, len);
		args = "";
	}
	
	//
	// python interpreter ...
	//
	if (gPythonMode) {
		if (cmd == "exit") {
			LOG_INFO("run console command in python mode: " << cmd);
			addTextToConsole("## Exit from python mode");
			addCmdToHistory(cmd_args);
			gPythonMode     = false;
			gHistory        = &gHistoryStandard;
			gHistoryCurrent = gHistory->end();
			return true;
		} else {
			LOG_INFO("run python code from console: " << cmd_args);
			MGE::ScriptsSystem::getPtr()->runStringInThread(
				"import threading\nthreading.current_thread().setName('CONSOLE')\n\n" + STRING_FROM_CEGUI(cmd_args)
			);
			addCmdToHistory(cmd_args);
			return true;
		}
	}
	LOG_INFO("run console command \"" << cmd << "\" with args string \"" << args << "\"");
	
	//
	// builtin console commenads ...
	//
	bool ret = false;
	if (cmd == "cmdlist") {
		addTextToConsole("builtin:");
		addTextToConsole("  cmdlist - list all console commands");
		addTextToConsole("  python - interactive python console");
		addTextToConsole("");
		addTextToConsole("registered:");
		for (auto& iter : gConsoleCommandMap) {
			addTextToConsole("  " + iter.first + " - " + iter.second.desc);
		}
		addCmdToHistory(cmd_args);
		return true;
	}
	if (cmd == "python") {
		addTextToConsole("## Enter to python mode, type \"exit\" to back standard console mode");
		addCmdToHistory(cmd_args);
		gPythonMode     = true;
		gHistory        = &gHistoryPython;
		gHistoryCurrent = gHistory->end();
		return true;
	}
	
	//
	// registered console commenads ...
	//
	auto iter = gConsoleCommandMap.find( STRING_FROM_CEGUI(cmd) );
	if (iter == gConsoleCommandMap.end()) {
		addTextToConsole("Command not found, see cmdlist for command list");
		return false;
	}
	if (! iter->second.sn.empty()) {
		ret = MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>(
			iter->second.sn.c_str(), false,
			STRING_FROM_CEGUI(cmd).c_str(), STRING_FROM_CEGUI(args).c_str()
		);
	} else if (iter->second.sf) {
		ret = iter->second.sf(this, STRING_FROM_CEGUI(cmd), STRING_FROM_CEGUI(args), iter->second.sfa);
	} else {
		ret = iter->second.f(this, STRING_FROM_CEGUI(cmd), STRING_FROM_CEGUI(args));
	}
	
	if (ret)
		addCmdToHistory(cmd_args);
	
	return ret;
}

bool MGE::GUIConsole::handleSubmit(const CEGUI::EventArgs&) {
	// get text out of the editbox
	CEGUI::String edit_text(gEditbox->getText());
	
	while (edit_text.back() == '\n')
		edit_text.pop_back();
	LOG_DEBUG("AAA" << edit_text.data());
	if (!edit_text.empty()) {
		CEGUI::String prefix("> ");
		if (gPythonMode)
			prefix = ">> ";
		
		CEGUI::String::size_type end, start = 0;
		while (true) {
			end = edit_text.find("\n", start);
			if (end != CEGUI::String::npos) {
				addTextToConsole(prefix + edit_text.substr(start, end-start));
				start = end + 1;
			} else {
				addTextToConsole(prefix + edit_text.substr(start));
				break;
			}
		}
		LOG_DEBUG("BBB" << edit_text.data());
		// parse and run command
		if (parseCmd(edit_text)) {
			// erase text in text entry box.
			gEditbox->setText("");
			
			// reset history position
			gHistoryCurrent = gHistory->end();
		} else {
			gEditbox->setText(edit_text);
		}
	}

	// re-activate the text entry box
	gEditbox->activate();

	return true;
}

bool MGE::GUIConsole::handleHide(const CEGUI::EventArgs& args) {
	consoleWin->hide();
	return true;
}

void MGE::GUIConsole::addCmdToHistory(CEGUI::String cmd_args) {
	// add this entry to the command history buffer and reset history position
	if ( gHistory->size() == 0 || gHistory->back() != cmd_args ) {
		gHistory->push_back(cmd_args);
	}
}

void MGE::GUIConsole::historyUp() {
	LOG_DEBUG("historyUp");
	
	if (gHistoryCurrent != gHistory->begin()) {
		--gHistoryCurrent;
		gEditbox->setText(*gHistoryCurrent);
		gEditbox->setCaretIndex(static_cast<size_t>(-1));
	}
	gEditbox->activate();
}

void MGE::GUIConsole::historyDown() {
	LOG_DEBUG("historyDown");
	
	if (gHistoryCurrent != gHistory->end()) {
		++gHistoryCurrent;
	}
	if (gHistoryCurrent != gHistory->end()) {
		gEditbox->setText(*gHistoryCurrent);
		gEditbox->setCaretIndex(static_cast<size_t>(-1));
	} else {
		gEditbox->setText("");
	}
	gEditbox->activate();
}

bool MGE::GUIConsole::keyDown(const CEGUI::EventArgs& args) {
	auto scancode = static_cast<const CEGUI::KeyEventArgs&>(args).d_key;
	
	switch( scancode ) {
		case CEGUI::Key::Scan::Esc:
			consoleWin->hide();
			return true;
		default:
			return false;
	}
}
bool MGE::GUIConsole::editboxKeyDown(const CEGUI::EventArgs& args) {
	auto scancode = static_cast<const CEGUI::KeyEventArgs&>(args).d_key;
	
	static CEGUI::EventArgs EmptyEventArgs;
	switch( scancode ) {
		case CEGUI::Key::Scan::ArrowUp:
			historyUp();
			return true;
		case CEGUI::Key::Scan::ArrowDown:
			historyDown();
			return true;
		case CEGUI::Key::Scan::Return:
			if (gEnterToSubmit->isSelected()) {
				handleSubmit(EmptyEventArgs);
				return true;
			} else {
				return false;
			}
		case CEGUI::Key::Scan::NumpadEnter:
			handleSubmit(EmptyEventArgs);
			return true;
		default:
			return false;
	}
}

bool MGE::GUIConsole::mouseUp(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	if (mbargs.d_button == CEGUI::MouseButton::Middle) {
		CEGUI::MultiLineEditbox* win = static_cast<CEGUI::MultiLineEditbox*>(mbargs.window);
		
		CEGUI::String oldText(win->getText().substr());
		CEGUI::String::size_type oldTextLen = oldText.length();
		CEGUI::String::size_type pos = win->getCaretIndex();
		CEGUI::String newText(CEGUI::System::getSingleton().getClipboard()->getText());
		
		LOG_DEBUG("insert: " << newText << " @position=" << pos);
		win->setText(
			oldText.substr(0, pos) + newText + oldText.substr(pos, oldTextLen-pos)
		);
		win->setCaretIndex(pos + newText.length());
	}
	return false;
}

bool MGE::GUIConsole::selectionChanged(const CEGUI::EventArgs& args) {
	auto wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	CEGUI::MultiLineEditbox* win = static_cast<CEGUI::MultiLineEditbox*>(wargs.window);
	
	CEGUI::String selText(win->getText().substr(
		win->getSelectionStart(),
		win->getSelectionLength()
	));
	
	//if (selText.length() != 0) {
		LOG_DEBUG("copy selection: " << selText);
		CEGUI::System::getSingleton().getClipboard()->setText(selText);
	//}
	return false;
}
