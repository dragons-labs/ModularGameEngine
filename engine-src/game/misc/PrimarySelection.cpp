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

#include "game/misc/PrimarySelection.h"

#include "Engine.h"
#include "ConfigParser.h"
#include "data/utils/OgreUtils.h"

#include "with.h"

#include "data/structs/BaseActor.h"
#include "data/structs/ActorMessages.h"
#include "data/structs/components/3DWorld.h"

#include "game/actorComponents/SelectableObject.h"

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_PrimarySelection \<PrimarySelection\>

@c \<PrimarySelection\> is used for setup <b>Primary Selection</b>, have following (optional) subnodes:
	- @c \<Marker\> - configuration of marker for selected objects (see @ref XMLNode_VisualMarkerSettingsSet for syntax info)

For configuration of selection marker (used while selecting) see @ref XMLNode_Selection.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(PrimarySelection) {
	return new MGE::PrimarySelection(xmlNode);
}

void MGE::PrimarySelection::onSelectionChanged() {
	LOG_DEBUG("PrimarySelection::onSelectionChanged");
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( SelectionChangeEventMsg() );
}

void MGE::PrimarySelection::markSelection(MGE::BaseActor* obj, bool selection, int mode) {
	MGE::World3DObject* w3d = obj->getComponent<MGE::World3DObject>();
	if (!w3d) {
		LOG_WARNING("markSelection for no World3DObject actor: " + obj->getName());
		return;
	}
	
	if (selection) {
		LOG_DEBUG("select: " << obj->getName() << " / " << obj);
		MGE::VisualMarkersManager::getPtr()->showMarker( w3d->getOgreSceneNode(), NULL, MGE::PrimarySelection::getPtr()->markerSettings );
	} else {
		LOG_DEBUG("deselect: " << obj->getName() << " / " << obj);
		MGE::VisualMarkersManager::getPtr()->hideMarker(w3d->getOgreSceneNode());
	}
}

bool MGE::PrimarySelection::canSelect(MGE::BaseActor* obj, int mode) {
	const MGE::SelectableObject* selectableObj = obj->getComponent<MGE::SelectableObject>();
	if (!selectableObj)
		return false;
	
	MGE::SelectableObject::status_t selMask = MGE::SelectableObject::IS_SELECTABLE | MGE::SelectableObject::IS_HIDDEN | MGE::SelectableObject::IS_UNAVAILABLE | MGE::SelectableObject::IS_ACTION_TARGET;
	return (selectableObj->status & selMask) == MGE::SelectableObject::IS_SELECTABLE;
}

MGE::PrimarySelection::PrimarySelection(const pugi::xml_node& xmlNode) :
	MGE::Unloadable(250)
{
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::ActorDestroyEventMsg::MsgType,
		std::bind(&MGE::PrimarySelection::onActorDestroy, this, std::placeholders::_1),
		this
	);

	WITH_NOT_NULL(MGE::Selection::getPtr())->setSelectionMode( MGE::Selection::GET_OBJECTS, &selectedObjects );
	
	// set default values for selection marker
	markerSettings.markerType      = MGE::VisualMarker::OBBOX | MGE::VisualMarker::BOX_PROPORTIONAL_THICKNESS | MGE::VisualMarker::CORNER_BOX;
	markerSettings.materialName    = MGE::OgreUtils::getColorDatablock(Ogre::ColourValue(.916, 0.88, 0.23));
	markerSettings.linesThickness  = 0.04;
	
	// configure selection marker settings from XML config
	markerSettings.loadFromXML( xmlNode.child("Marker") );
}

MGE::PrimarySelection::~PrimarySelection() {
	WITH_NOT_NULL(MGE::Selection::getPtr())->setSelectionMode( MGE::Selection::NONE );
	
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::ActorDestroyEventMsg::MsgType,
		std::bind(&MGE::PrimarySelection::onActorDestroy, this, std::placeholders::_1),
		this
	);
}

bool MGE::PrimarySelection::unload() {
	selectedObjects.selection.clear();
	return true;
}

void MGE::PrimarySelection::onActorDestroy(const MGE::EventMsg* eventMsg) {
	auto actorMsg = static_cast<const MGE::ActorDestroyEventMsg*>(eventMsg);
	
	selectedObjects.unselect(actorMsg->actor);
	onSelectionChanged();
}
