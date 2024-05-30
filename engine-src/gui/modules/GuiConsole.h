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

#pragma   once

#include "BaseClasses.h"
#include "ModuleBase.h"

#include <CEGUI/CEGUI.h>
#include <OISKeyboard.h>

#include <list>
#include <functional>

namespace MGE {

/// @addtogroup GUI_Modules
/// @{
/// @file

/**
 * @brief Gui text console for debuging, developer and cheat command
 */
class GUIConsole :
	public MGE::Module,
	public MGE::Singleton<GUIConsole> {
public:
	/**
	 * @brief add text to console "output" window
	 * 
	 * @param[in] text        text to add
	 * @param[in] addNewLine  when true (default) add new line at end of string
	 */
	void addTextToConsole(const CEGUI::String& text, bool addNewLine = true);
	
	/**
	 * @brief type of function to implement console command
	 * 
	 * This is 3 argument function or class member function. As arguments receives:
	 *  @li console pointer (MGE::GUIConsole*)
	 *  @li console command
	 *  @li console command arguments
	 */
	
	typedef std::function<bool(MGE::GUIConsole* console, const std::string& cmd, const std::string& args)> CmdDelegate;
	
	/**
	 * @brief function to register console command
	 * 
	 * @param[in] key         console command handled by function
	 * @param[in] desc        console command description (for cmdlist command)
	 * @param[in] f           pointer to function (see @ref CmdDelegate)
	 * 
	 * Example:
		\code{.cpp}
		CameraControls::CameraControls() {
			MGE::GUIConsole::CmdDelegate camConsHand = std::bind(
				&CameraControls::consoleCameraControl,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3
			);
			MGE::GUIConsole::getPtr()->addConsoleCmd ( "camera", "camera info and control", camConsHand );
		}
		bool CameraControls::consoleCameraControl(MGE::GUIConsole* console, const std::string& c_cmd, const std::string& c_args) {
			if (c_cmd != "camera") 
				return false;
			
			std::string cmd;
			std::stringstream args;
			
			// parse console command arguments (c_args) to camera subcommand and subcommand arguments
			int pos = c_args.find(" ");
			if (pos > 0) {
				cmd = c_args.substr(0,pos);
				args << c_args.substr(pos+1);
			} else {
				cmd = c_args;
			}
			
			// select subcomand
			if (cmd.empty()) {
				console->addTextToConsole("USAGE: camera [help|command ...]");
				return true;
			} else if (cmd == "setPosition") {
				float x, y, z;
				args >> x >> y >> z;
				if (args.rdstate()  == std::ifstream::eofbit) {
					setPosition(Ogre::Vector3(x, y, z));
					return true;
				}
			}
			
			// no success when parsing onsole command arguments print info
			console->addTextToConsole("Invalid argument. Try `camera help` for usage info");
			return false;
		}
		\endcode
	*/
	void addConsoleCmd(const std::string& key, const std::string& desc, CmdDelegate f);
	
	/**
	 * @brief type of function to implement console command
	 * 
	 * This is 4 argument static function. As arguments receives:
	 *  @li console pointer (MGE::GUIConsole*)
	 *  @li console command
	 *  @li console command arguments
	 *  @li pointer void* submitted during registration function
	 */
	typedef bool (*CmdDelegateStatic)(MGE::GUIConsole* console, const std::string& cmd, const std::string& args, void* ptr);
	
	/**
	 * @brief function to register console command
	 * 
	 * @param[in] key         console command handled by function
	 * @param[in] desc        console command description (for cmdlist command)
	 * @param[in] f           pointer to function (see @ref CmdDelegateStatic)
	 * @param     a           pointer void* passed as 4 arument to command function
	 */
	void addConsoleCmd(const std::string& key, const std::string& desc, CmdDelegateStatic f, void* a);
	
	/**
	 * @brief function to register console script
	 * 
	 * @param[in] key         console command handled by function
	 * @param[in] desc        console command description (for cmdlist command)
	 * @param[in] script      script name
	 */
	void addConsoleScript(const std::string& key, const std::string& desc, const std::string& script);
	
	/**
	 * @brief return true if console is visible
	 */
	inline bool isVisible() const {
		return consoleWin->isVisible();
	}
	
	/**
	 * @brief toggle console visibility
	 */
	void toggleVisibility();
	
	/**
	 * @brief show console
	 */
	void show();
	
	/**
	 * @brief show console
	 */
	void hide();
	
	/**
	 * @brief high priority key pressed listener function
	 */
	bool priorityKeyPressed(const OIS::KeyEvent& arg);
	
	/// constructor - create GUI console
	GUIConsole();
	
protected:
	/// destructor - destroy GUI console
	~GUIConsole();
	
	/// helper function to get python stdout output
	void getPythonOutput(const std::string& str);
	
	/// for command registration
	struct ConsoleCommand {
		std::string desc;
		CmdDelegate f;
		CmdDelegateStatic sf;
		std::string sn;
		void* sfa;
	};
	std::map<std::string, ConsoleCommand> gConsoleCommandMap;
	
	/// command input history list for standard mode
	std::list<CEGUI::String>            gHistoryStandard;
	/// command input history list for python mode
	std::list<CEGUI::String>            gHistoryPython;
	/// pointer to currently used history list (gHistoryStandard or gHistoryPython)
	std::list<CEGUI::String>*           gHistory;
	/// command input history list current position iterator
	std::list<CEGUI::String>::iterator  gHistoryCurrent;
	
	/// pointers to gui elements - main window
	CEGUI::Window*                      consoleWin;
	/// pointers to gui elements - command input box
	CEGUI::MultiLineEditbox*            gEditbox;
	/// pointers to gui elements - command output box
	CEGUI::MultiLineEditbox*            gHistoryBox;
	/// pointers to gui elements - switch multilien input vs exec by Enter key
	CEGUI::ToggleButton*                gEnterToSubmit;
	
	/// handle submit command by button-clik or press enter (when enabled)
	bool handleSubmit(const CEGUI::EventArgs& args);
	
	/// parse and run command, put output to console window
	/// (called from handleSubmit)
	bool parseCmd(CEGUI::String cmd_args);
	
	/// handle console hide action from "close button"
	bool handleHide(const CEGUI::EventArgs& args);
	
	/// add command string to history
	void addCmdToHistory(CEGUI::String cmd_args);
	/// get older (than the current one) history entry and put to EditBox
	/// (called from editboxKeyDown)
	void historyUp();
	/// get newer (than the current one) history entry and put to EditBox
	/// (called from editboxKeyDown)
	void historyDown();
	
	/// callback function for KeyDown event
	/// (hide console on ESC key pressed)
	bool keyDown(const CEGUI::EventArgs& args);
	/// callback function for KeyDown event on EditBox widget
	/// (reaction to arrow up/down and enters keys)
	bool editboxKeyDown(const CEGUI::EventArgs& args);
	
	/// callback function for KeyUp event
	/// (middle button paste)
	bool mouseUp(const CEGUI::EventArgs& args);
	/// callback function for TextSelectionChanged event  on EditBox widget
	/// (selection copy)
	bool selectionChanged(const CEGUI::EventArgs& args);
	
	/// when true console works in Python mode
	bool gPythonMode;
};

/// @}

}
