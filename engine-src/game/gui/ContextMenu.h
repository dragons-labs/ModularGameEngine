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

#include "input/SelectionContextMenu.h"
#include "rendering/markers/VisualMarkers.h"

namespace MGE { struct Action; }
namespace MGE { struct ActionPrototype; }

#include <CEGUI/CEGUI.h>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Context menu for selected actors actions
 */
class ContextMenu : 
	public MGE::SelectionContextMenu,
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::Singleton<ContextMenu>
{
public:
	/// @copydoc MGE::SelectionContextMenu::showContextMenu
	void showContextMenu(const Ogre::Vector2& mousePos, CEGUI::Window* tgrWin, MGE::RayCast::ResultsPtr clickSearch) override;
	
	/// @copydoc MGE::SelectionContextMenu::hideContextMenu
	void hideContextMenu() override;
	
	/// constructor
	ContextMenu(const pugi::xml_node& xmlNode, CEGUI::Window* parent = NULL);
	
protected:
	/// destructor
	virtual ~ContextMenu(void);
	
	/// pointer to contextmenu window
	CEGUI::PopupMenu*         menuWin;
	
	/// pointer to default parent window
	CEGUI::Window*            defaultParent;
	
	/// pointer to current parent window
	CEGUI::Window*            curentParent;
	
	/// click mouse position relative to current parent window
	Ogre::Vector2             clickMousePos;
	
	/// action selected from context menu
	MGE::Action* action;
	
	/// type of Target for which the waiting
	int waitForTargetType;
	
	/// when true don't close or change menu before get choise in current display menu
	bool forceWaitForMenuChoice;
	
	/// settings set for selection marker
	MGE::VisualMarkerSettingsSet targetSelectionMarkerSettings;
	
	/// enum for @ref addItemToMenu callback argument values
	enum CallbackTypes { NONE, ACTION, TARGET_DONE, SWITCH_SELECTION_MODE };
	
	/**
	 * @brief adding @a id identyficated item to menu vith text @a name
	 * 
	 * @param     menu      pointer to PopupMenu to add this item
	 * @param     name      item text
	 * @param     ptr       pointer (typical ActionPrototype pointer) to set in item user data ptr field
	 * @param     id        optional numeric id to set in item user id field
	 * @param     callback  index of callback function
	 * 
	 * @return function return pointer to new created item, this pointer will be delete by parent window
	 */
	CEGUI::MenuItem* addItemToMenu(CEGUI::PopupMenu* menu, const std::string_view& name, void* ptr, unsigned int id, int callback);
	
	/**
	 * @brief short version of addItemToMenu call, used to add internal action to top level context menu
	 *
	 * @param     name      item text
	 * @param     callback  index of callback function
	 * @param     id        optional numeric id of subaction
	 * 
	 * @return function return pointer to new created item, this pointer will be delete by parent window
	 */
	inline CEGUI::MenuItem* addInternalActionToMenu(const std::string_view& name, int callback, int id = 0) {
		return addItemToMenu(menuWin, name, NULL, id, callback);
	}
	
	/**
	 * @brief auxiliary function for show(), showContextMenu(), etc - fixing menu position and show menu
	 */
	void fixPositionAndShow();
	
	/**
	 * @brief check compatibility of target object with @a _actionProto
	 * 
	 * @param _clickSearch raycast result for get actor list
	 * @param _actionProto action prototype to check compatibility of target(s)
	 * @param _action      when not NULL add compatible target to action->targetObjects
	 * 
	 * @return
	 *   @li 0 when target haven't actor matching _actionProto target filter
	 *   @li 1 when target have actor matching _actionProto target filter
	 */
	int checkTargetActorCompatibility(
		MGE::RayCast::ResultsPtr _clickSearch,
		MGE::ActionPrototype* _actionProto,
		MGE::Action* _action = NULL
	);
	
	/// enum for @ref setSelectionMode type argument values
	enum SelectionTypes { PRIMARY, TARGET };
	
	/**
	 * @brief set selection mode
	 * 
	 * @param type
	 *   - PRIMARY  set selection to primary selection set (@ref MGE::PrimarySelection)
	 *              default value for selection mode is GET_OBJECTS
	 *   - TARGET   set selection to action target set (@ref actionTargetObjects)
	 *              default value for selection mode is setting based on @ref waitForTargetType
	 *              selected points are stored in targetPoints in @ref action
	 * @param selMode
	 *     when \>= 0 enforce selection mode to this value
	 */
	void setSelectionMode(int type, int selMode = -1);
	
	/**
	 * @brief show waitng messages for target getting
	 */
	void showTargetWaitMessage();
	
	/**
	 * @brief add selected action to action queue of actors from primary selection set
	 */
	void addActionToQueue();
	
	
	/**
	 * @brief handle and run action selected in context menu
	 * 
	 * @param[in] args - OIS Event detail/description
	 */
	bool handleAction(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle and run action after completed actionTarget
	 * 
	 * @param[in] args - OIS Event detail/description
	 */
	bool handleTargetDone(const CEGUI::EventArgs& args);
	
	/**
	 * @brief handle for switch target get modes
	 * 
	 * @param[in] args - OIS Event detail/description
	 */
	bool handleSwitchSelectionMode(const CEGUI::EventArgs& args);
	
	/// struct for managing target objects selection
	struct TargetSelection;
	friend struct TargetSelection;
	
	/// store open menu raycast search results until close or open new menu
	MGE::RayCast::ResultsPtr clickSearch;
	
	/// store selection results for selecting targest actors
	TargetSelection* actionTargetObjects;
};

/// @}

}
