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

#include "game/actorComponents/Target.h"

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"

#include "data/structs/components/3DWorld.h"
#include "data/utils/OgreUtils.h"
#include "rendering/utils/VisibilityFlags.h"
#include "data/QueryFlags.h"

MGE_ACTOR_COMPONENT_CREATOR(MGE::Target, Target) {
	if (!config)
		throw std::logic_error("Can't create Target without XML config");
	
	typeIDs->insert(MGE::Target::classID);
	typeIDs->insert(createdForID);
	return new MGE::Target(parent, config, createdForID);
}


/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Target Target

Use subnodes:
  - @c collide for unset or not COLLISION_OBJECT flag (default false → unset COLLISION_OBJECT → target actor do not collide with other objects)
  - @c visible for set or not VisibilityFlags to TARGETS (default false → set VisibilityFlags to TARGETS → target actor will not be visible)
*/

MGE::Target::Target(MGE::NamedObject* parent, const pugi::xml_node& xmlNode, int createdForID) : 
	MGE::ScriptComponent(parent, xmlNode, createdForID)
{}

MGE::Target::~Target() {
}

bool MGE::Target::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	LOG_INFO("restore Target Component");
	
	collide = xmlNode.child("collide").text().as_bool(false);
	visible = xmlNode.child("visible").text().as_bool(false);
	
	MGE::ScriptComponent::restoreFromXML(xmlNode, parent, sceneNode);
	
	return true;
}

void MGE::Target::init(MGE::NamedObject* parent) {
	LOG_INFO("init Target Component");
	
	if ( ! collide ) {
		// remove COLLISION_OBJECT from parent QueryFlags and add TARGET to it
		MGE::OgreUtils::recursiveUpdateQueryFlags(
			parent->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
			~MGE::QueryFlags::COLLISION_OBJECT, MGE::QueryFlags::TARGET
		);
	} else {
		// add TARGET to parent QueryFlags
		MGE::OgreUtils::recursiveUpdateQueryFlags(
			parent->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
			0xffffffff, MGE::QueryFlags::TARGET
		);
	}
	
	if ( ! visible ) {
		/// set VisibilityFlags to TARGETS
		MGE::OgreUtils::recursiveUpdateVisibilityFlags(
			parent->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
			0, MGE::VisibilityFlags::TRIGGERS
		);
	}
}
