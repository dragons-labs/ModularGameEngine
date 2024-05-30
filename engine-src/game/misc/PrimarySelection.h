/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
#include "MessagesSystem.h"
#include "ModuleBase.h"
#include "input/SelectionSet.h"
#include "rendering/markers/VisualMarkers.h"

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Window with information about current selected actor
 */
class PrimarySelection :
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::Singleton<PrimarySelection>
{
public:
	/// struct for primary selection change message
	/// message will be called once after finish selection update
	struct SelectionChangeEventMsg;
	
	/// type name for @ref selectedObjects
	typedef MGE::SelectionSet<MGE::BaseActor*, MGE::QueryFlags::GAME_OBJECT, MGE::PrimarySelection, void> SelectionSet;
	
	/// set of currently selected scene object
	SelectionSet selectedObjects;
	
	/// @copydoc MGE::SelectionSetTemplate::canSelect
	static bool canSelect(MGE::BaseActor* obj, int mode);
	
	/// @copydoc MGE::SelectionSetTemplate::markSelection
	static void markSelection(MGE::BaseActor* obj, bool selection, int mode);
	
	/// @copydoc MGE::SelectionSetTemplate::onSelectionChanged
	static void onSelectionChanged();
	
	/// callback function for MGE::ActorDestroyEventMsg event message
	void onActorDestroy(const MGE::EventMsg* eventMsg);
	
	/// constructor
	PrimarySelection(const pugi::xml_node& xmlNode);
	
	/// destructor
	~PrimarySelection();
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
protected:
	/// settings set for selection marker
	MGE::VisualMarkerSettingsSet markerSettings;
};

struct MGE::PrimarySelection::SelectionChangeEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "SelectionChange"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
};

/// @}

}
