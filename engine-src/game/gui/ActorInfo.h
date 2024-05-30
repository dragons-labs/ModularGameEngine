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

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "gui/GuiGenericWindows.h"

#include "game/misc/ActorFilter.h"
#include "data/structs/components/ObjectOwner.h"

namespace MGE { struct EventMsg; }
namespace MGE { struct Action; }
namespace MGE { struct ActionQueue; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Window with information about current selected actor.
 *
 * There are three sub windows:
 *  - "Actor", used to show selected actor info:
 *    - image
 *    - name
 *    - health level
 *    - length of action queue
 *  - "Info", can work in several modes:
 *    - default mode
 *      - show selected actor owned objects list (filtered based on ActorInfo config \<ItemsFilter\> tag)
 *      - support call EXIT action on each owned object (by click on them in list)
 *      - show health level of each owned object
 *    - getToolMode (when current action is SELECT_TOOL and have valid target)
 *      - show target actor owned objects list (filtered by action targetFilter)
 *      - support call GET_TOOL action on each owned object (by click on them in list)
 *    - queue list mode (switched by click on queue length info in "Actor" sub-window)
 *      - show queue list of selected actor
 *      - support show action info in "Target" sub-window
 *  - "Target", used to show current (or selected in "Info" sub-window) action target and tool:
 *    - target image
 *    - target health level
 *    - tool image
 *
 * There are controls for:
 *  - game time and speed info and settings:
 *    - game time info
 *    - game speed setting
 *    - pause / unpause switch and info
 *  - show "WorldInfoWindow" (world map, minimap, raport, units list window)
 *  - show main menu
 * 
 * Window support minimalization by double clicking on it.
 */
class ActorInfo :
	public MGE::GenericWindows::BaseWindowOwner,
	public MGE::MainLoopListener,
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::Singleton<ActorInfo>
{
public:
	/**
	 * @brief metod to set healthLevel on progress bar (support progress bar color selection)
	 * 
	 * @param[in] progressBar  pointer to progress bar
	 * @param[in] object       actor or prototype to get health level
	 */
	void setHealthLevel(CEGUI::ProgressBar* progressBar, MGE::NamedObject* object);
	
	/// @copydoc MGE::GenericWindows::BaseWindowOwner::show
	void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
	
	/**
	 * @brief switch visible actor info window
	 */
	void toggleVisibility();
	
	/// @copydoc MGE::MainLoopListener::update
	/// (used to update actor health)
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// callback function for MGE::GameSpeedChangeEventMsg event message
	void gameSpeedUpdate(const MGE::EventMsg* eventMsg = NULL);
	
	/// callback function for MGE::PrimarySelection::SelectionChangeEventMsg event message
	void onSelectionUpdate(const MGE::EventMsg*);
	
	/// constructor
	ActorInfo(const pugi::xml_node& xmlNode);
	
protected:
	/// constructor
	ActorInfo(MGE::GenericWindows::BaseWindow* baseWin);
	
	/// destructor
	~ActorInfo();
	
private:
	// info about selected actor
	MGE::BaseActor*             actor;
	MGE::ActionQueue*           actionQueue;
	MGE::ObjectOwner*           objectOwner;
	MGE::NamedObject*           toolObject;
	MGE::BaseActor*             targetObject;
	bool                        actorIsTargetActor;
	// when false show info about first action from actionQueue
	// otherwise show info about selected in "Info" sub-win action
	bool                        showManualSelectedActionInfo;
	
	// info about action queue
	int                         qLen;
	std::chrono::time_point<std::chrono::steady_clock> qUpdateTime;
	
	// info about "items list"
	MGE::ObjectOwner*           listedObjectOwner;
	std::chrono::time_point<std::chrono::steady_clock> itemsUpdateTime;
	bool                        getToolMode;
	const MGE::ActorFilter*     itemFilter;
	MGE::ActorFilter            itemStandardFilter;
	
	// page breaks in "items list"
	MGE::ObjectOwner::iterator  itemsIter;
	int                         itemsSubIter;
	int                         itemsSubMax;
	int                         itemsSubAvailable;
	
	// functions for "action queue list"
	void showActionsQueueList(bool restoreSelection = true);
	bool actionQueueHandle(const CEGUI::EventArgs& args);
	bool actionClickHandle(const CEGUI::EventArgs& args);
	
	// functions for "items list"
	void showOwnedObjectList(bool firstPage = true);
	bool firstItemListPage(const CEGUI::EventArgs& args);
	bool nextItemListPage(const CEGUI::EventArgs& args);
	bool clickItemList(const CEGUI::EventArgs& args);
	
	// functions for action target and tool info
	void showActionTargetAndTool(MGE::Action* action, bool force = false);
	
	// functions for window buttons
	bool mainMenuHandle(const CEGUI::EventArgs& args);
	bool showMapHandle(const CEGUI::EventArgs& args);
	bool pauseHandle(const CEGUI::EventArgs& args);
	bool speedIncHandle(const CEGUI::EventArgs& args);
	bool speedDecHandle(const CEGUI::EventArgs& args);
	
	// real update functions ...
	void fullUpdate();
	void update(bool force);
	
	// pointers to window elements
	CEGUI::ProgressBar*            actorHealthLevel;
	CEGUI::ProgressBar*            targetHealthLevel;
	CEGUI::Window*                 itemList;
	CEGUI::ListWidget*             actionQueueList;
	CEGUI::Window*                 timeInfo;
	
	bool                           needFullUpdate;
	
	/// struct for items in actionQueueList
	struct ActionItem : public CEGUI::StandardItem {
		MGE::Action* action;
		ActionItem(const CEGUI::String& text, MGE::Action* a);
		virtual bool operator==(const CEGUI::GenericItem& other) const override;
	};
};

/// @}

}
