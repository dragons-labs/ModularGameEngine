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
#include "ListenerSet.h"
#include "StringTypedefs.h"
#include "ModuleBase.h"
#include "input/InputSystem.h"
#include "data/LoadingScreen.h"

#include <CEGUI/CEGUI.h>

namespace MGE { class OpenFileDialog; }

namespace MGE {

/// @addtogroup GUI_Modules
/// @{
/// @file

/**
 * @brief Main menu (main or full-pause menu)
 */
class MainMenu : 
	public MGE::Module,
	public MGE::Singleton<MainMenu>,
	public MGE::InputSystem::Listener,
	public MGE::LoadingScreen
{
public:
	/// base class for MainMenu listeners
	class Listener {
	public:
		/**
		 * @brief call when main menu handle click action on unknow (not supported in @ref MGE::MainMenu class) button
		 *        Each listener must be register with all used by it @a buttonName as key
		 * 
		 * @param[in] buttonName             name of clicked button for identify action
		 */
		virtual bool runMainMenuAction(const std::string& buttonName) = 0;
		
		/// destructor
		virtual ~Listener() {}
	};

	/// main modes of main menu
	enum MenuMode {
		/// show standard menu
		STANDARD = 0,
		/// show end game message whith return to menu
		END_GAME,
		/// show load screen
		LOADING_SCREEN,
		/// subdialog
		SUB_DIALOG
	};
	
	/**
	 * @brief show main menu and pause game
	 * 
	 * @param[in] mode        mode of main menu
	 * @param[in] title       subject of information (only for some modes)
	 * @param[in] text        information text (only for some modes)
	 * @param[in] argb_clor   color of subject (only for some modes)
	 */
	void show(
		MenuMode mode                     = STANDARD,
		const std::string_view& title     = MGE::EMPTY_STRING_VIEW,
		const std::string_view& text      = MGE::EMPTY_STRING_VIEW,
		const std::string_view& argb_clor = MGE::EMPTY_STRING_VIEW
	);
	
	/// @copydoc MGE::LoadingSystem::setLoadingScreenImage
	virtual void setLoadingScreenImage(const std::string_view& imageName, const std::string_view& imageGroup) override;
	
	/// @copydoc MGE::LoadingSystem::setLoadingScreenProgress
	virtual void setLoadingScreenProgress(float progress, const std::string_view& info) override;
	
	/// @copydoc MGE::LoadingSystem::showLoadingScreen
	virtual void showLoadingScreen() override;
	
	/// @copydoc MGE::LoadingSystem::hideLoadingScreen
	virtual void hideLoadingScreen() override;
	
	/**
	 * @brief hide MainMenu (MainMenu) and unpause game
	 */
	void hide(bool after_reload = false);
	
	/**
	 * @brief show/hide/switch mode MainMenu (MainMenu) due to Esc press
	 */
	void injectEsc();
	
	/**
	 * @brief return true if MainMenu is visible
	 */
	bool isVisible() const;
	
	/**
	 * @brief toggle MainMenu visibility
	 */
	void toggleVisibility();
	
	/**
	 * @brief set full pause
	 */
	void setFullPause(bool pause);
	
	/**
	 * @brief return info about full pause game by MainMenu
	 * 
	 * when return true game is paused by MainMenu
	 */
	bool isFullPaused() const;
	
	/**
	 * @brief run MainMenu action (based on buttonName) listener interfaces, see @ref MGE::ListenerSet for detail
	 */
	MGE::ClassPtrListenerSet<MGE::MainMenu::Listener, std::string_view, std::string> runMainMenuActionListeners;
	
	/**
	 * @brief high priority key pressed listener function
	 */
	bool priorityKeyPressed(const OIS::KeyEvent& arg);
	
	/**
	 * @brief standard key pressed listener function
	 */
	virtual bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
	
	/// constructor
	MainMenu();
	
protected:
	friend class Singleton;
	
	/// destructor
	~MainMenu();
	
	/**
	 * @brief handle button click and run associate action
	 * 
	 * @param[in] args  OIS Event detail/description
	 */
	bool handleAction(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle module selection change and show module info
	 * 
	 * @param[in] args  OIS Event detail/description
	 */
	bool mapsListSelectionChanged(const CEGUI::EventArgs& args);
	
	/**
	 * @brief show module-load window with @a dir directory
	 * 
	 * @param[in] dir   director with modules info .ini files
	 */
	void showLoadMapMenu(const std::string& dir);
	
	/// store information about paussing game when showing menu
	/// @note true == MainMenu call pause(), false == game was previous paused
	bool pausedOnShow;
	
	/// store information about paussing game does by MainMenu
	bool fullPaused;
	
	/// when true load-save dialog should work in editor mode (insted of "game save" mode)
	bool editorLoadSaveMode;
	
	/// pointer to window object
	CEGUI::Window* menuWin;
	// pointers to MainMenu sub-window (menu modes window) - standard mode
	CEGUI::Window* mainMenu;
	// pointers to MainMenu sub-window (menu modes window) - load map mode
	CEGUI::Window* loadMapMenu;
	// pointers to MainMenu sub-window (menu modes window) - end game mode
	CEGUI::Window* endGameMenu;
	// pointers to MainMenu sub-window (menu modes window) - end game mode
	CEGUI::Window* loadingScreen;
	// pointers to elements of "load map" subwindow
	CEGUI::ListWidget* mapsList;
	// pointers to open file dialog subsystem
	MGE::OpenFileDialog* openFileDialog;
	
	// current main menu mode
	MenuMode currMode;
	
	/// struct for items in mapsList
	struct MapEntryItem : public CEGUI::StandardItem {
		std::string fileName;
		MapEntryItem(const CEGUI::String& text, const std::string& file);
		virtual bool operator==(const CEGUI::GenericItem& other) const override ;
	};
};

/// @}

}
