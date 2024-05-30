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

#include "game/actorComponents/Trigger.h"

#include "ScriptsSystem.h"
#include "data/utils/OgreUtils.h"
#include "rendering/utils/VisibilityFlags.h"
#include "data/QueryFlags.h"

#include "data/structs/BaseActor.h"
#include "data/structs/components/3DWorld.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"

#include "game/actorComponents/World3DMovable.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

MGE::Trigger::Trigger(MGE::NamedObject* parent) {
}

MGE::Trigger::~Trigger() {
}

MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(MGE::Trigger, Trigger)


/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Trigger Trigger

Store / restore from its @c \<Component\> node required subnodes:
  - @c TriggerType numeric id of triger type (see @ref MGE::Trigger::TrigerTypes, string or numeric value converted via @ref MGE::Trigger::stringToTrigerType)
  - @c ScriptName name of script to run for some trigger types
and optional subnodes:
  - @c \<SpeedModifier\> for add entry to @ref MGE::Trigger::speedModifiers (used for modify actor speed when triggerType == CHECK_SPEED_MAP) with attributes:
    - @c movableType sub type of movable actor (see @ref MGE::World3DMovable::SubTypes, string or numeric value converted via @ref MGE::World3DMovable::stringToSubType)
    - @c value value will be multiply with standard actor speed

See @ref MGE::Trigger for details.
*/
bool MGE::Trigger::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	LOG_INFO("restore Trigger Component");
	
	auto triggerTypeXML = xmlNode.child("TriggerType");
	if (!triggerTypeXML)
		return true; // setup can be omitted in save xml format
	
	triggerType = stringToTrigerType( triggerTypeXML.text().as_string() );
	scriptName  = xmlNode.child("ScriptName").text().as_string();
	for (auto xmlSubNode : xmlNode.children("SpeedModifier")) {
		speedModifiers[
			MGE::World3DMovable::stringToSubType(
				xmlSubNode.attribute("movableType").as_string()
			)
		] = xmlSubNode.attribute("value").as_float(1.0);
	}
	return true;
}

void MGE::Trigger::init(MGE::NamedObject* parent) {
	LOG_INFO("init Trigger Component");
	
	// remove COLLISION_OBJECT from parent QueryFlags and add TRIGGER to it
	MGE::OgreUtils::recursiveUpdateQueryFlags(
		parent->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
		~MGE::QueryFlags::COLLISION_OBJECT, MGE::QueryFlags::TRIGGER
	);
	/// set VisibilityFlags to TRIGGERS
	MGE::OgreUtils::recursiveUpdateVisibilityFlags(
		parent->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
		0, MGE::VisibilityFlags::TRIGGERS
	);
}

void MGE::Trigger::runTrigger(MGE::BaseActor* actor) const {
	DEBUG2_LOG(" RUN trigger: " << scriptName);
	switch(triggerType) {
		case RUN_SCRIPT:
		case RUN_ACTION_SCRIPT:
			MGE::ScriptsSystem::getPtr()->runObjectWithVoid(
				(scriptName/* + "_run"*/).c_str(), pybind11::cast(actor)
			);
			break;
	}
}

float MGE::Trigger::getSpeedModifier(MGE::BaseActor* actor) const {
	DEBUG2_LOG(" CHECK trigger: " << scriptName);
	switch(triggerType) {
		case CHECK_SPEED_MAP:
		{
			MGE::World3DMovable* movableActor = actor->getComponent<MGE::World3DMovable>();
			if(!movableActor) {
				LOG_DEBUG("Check crossing CHECK_SPEED_MAP trigger for non movable actor is not supported");
				return 0;
			}
			try {
				return speedModifiers.at(movableActor->getSubType());
			} catch (std::out_of_range) {
				return 1.0;
			}
		}
		/*
		case RUN_SCRIPT:
		case RUN_SPEED_SCRIPT:
			return MGE::ScriptsSystem::getPtr()->runObjectWithCast<float>(
				(scriptName + "_check").c_str(), 0.0, pybind11::cast(actor)
			);
		*/ /// @todo TODO.5 this has issue with parallel path finding and GIL
		case NO_ACCESS:
			return 0;
		default:
			return 1.0;
	}
}
