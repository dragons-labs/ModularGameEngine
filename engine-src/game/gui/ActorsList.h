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

namespace MGE { struct EventMsg; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Window with list of selectable actors
 */
class ActorsList :
	public MGE::GenericWindows::BaseWindowOwner,
	public MGE::MainLoopListener,
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::Singleton<ActorsList>
{
public:
	/// @copydoc MGE::GenericWindows::BaseWindowOwner::show
	void show(const CEGUI::String& name = CEGUI::String::GetEmpty()) override;
	
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// callback function for events message
	void updateOnEvent(const MGE::EventMsg*);
	
	/// constructor
	ActorsList(const pugi::xml_node& xmlNode);
	
protected:
	/// destructor
	~ActorsList(void);
 
private:
	/// Internal use in constructor.
	CEGUI::Combobox* _configureFilter(MGE::null_end_string name, const pugi::xml_node& xmlNode);
	
	/// Internal use in constructor.
	void init(MGE::GenericWindows::BaseWindow* baseWin, uint64_t _defMask, uint64_t _defCmpVal);
	
	void doUpdate();
	
	bool onShow(const CEGUI::EventArgs& args);
	bool onHide(const CEGUI::EventArgs& args);
	bool unitsListSelectionChanged(const CEGUI::EventArgs& args);
	bool unitsListDoubleClick(const CEGUI::EventArgs& args);
	
	CEGUI::MultiColumnList*         unitsList;
	bool                            needUpdate;
	bool                            isVisible;
	bool                            onUpdate;
	
	uint64_t                        defMask;
	uint64_t                        defCmpVal;
	std::vector<MGE::ActorFilter>   filters;
	CEGUI::Combobox*                filterA;
	CEGUI::Combobox*                filterB;
	
	bool handleFilter(const CEGUI::EventArgs& args);
};

/// @}

}
