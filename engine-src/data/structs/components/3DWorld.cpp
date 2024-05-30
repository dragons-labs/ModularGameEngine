/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/components/3DWorld.h"

#include "Engine.h"
#include "physics/Raycast.h"
#include "physics/PathFinder.h"
#include "data/utils/OgreUtils.h"
#include "physics/utils/OgreColisionBoundingBox.h"
#include "data/property/XmlUtils_Ogre.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "data/utils/NamedSceneNodes.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>


/*--------------------- World3DObject ---------------------*/

Ogre::Vector3 MGE::World3DObject::getWorldPosition() const {
	return getOgreSceneNode()->_getDerivedPosition();
}

Ogre::Quaternion MGE::World3DObject::getWorldOrientation() const {
	return getOgreSceneNode()->_getDerivedOrientation();
}

Ogre::AxisAlignedBox MGE::World3DObject::getWorldOrientedAABB() const {
	Ogre::AxisAlignedBox retAABB( getAABB() );
	Ogre::Matrix4 xform;
	xform.makeTransform(Ogre::Vector3::ZERO, getOgreSceneNode()->_getDerivedScale(), getOgreSceneNode()->_getDerivedOrientation());
	retAABB.transformAffine(xform);
	return retAABB;
}

Ogre::Vector3 MGE::World3DObject::getWorldDirection() const {
	//return getOrientation() * Vector3::NEGATIVE_UNIT_Z;
	return - getWorldOrientation().zAxis();
}

void MGE::World3DObject::setWorldPosition(const Ogre::Vector3& position) {
	return getOgreSceneNode()->_setDerivedPosition(position);
}

void MGE::World3DObject::setWorldPositionOnGround(Ogre::Vector3& position) {
	MGE::RayCast::ResultsPtr res = MGE::RayCast::searchVertical(getOgreSceneNode()->getCreator(), position.x, position.z);
	if (res->hasGround) {
		position.y += res->groundPoint.y;
		LOG_DEBUG("setPosition_onGround: ground.y=" << res->groundPoint.y << ", new position is: " << position);
	} else {
		LOG_WARNING("setPosition_onGround: not found ground");
	}
	setWorldPosition(position);
}

void MGE::World3DObject::findAndSetFreePositionOnGround(Ogre::Vector3& position) {
	LOG_DEBUG(" - initial position is: " << position);
	// 1. get correct y
	position.y = 0;
	setWorldPositionOnGround(position);
	updateCachedTransform();
	
	// 2. seacrh free position
	std::pair<bool, Ogre::Vector3> res = MGE::RayCast::findFreePosition(getOgreSceneNode(), getAABB());
	position = res.second;
	LOG_DEBUG(" - findFreePosition results is: " << res.first << " / " << position);
	
	// 3. put on ground at final (free) position
	position.y = 0;
	setWorldPositionOnGround(position);
	updateCachedTransform();
	LOG_DEBUG(" - final position is: " << getWorldPosition());
}

void MGE::World3DObject::setWorldOrientation(const Ogre::Quaternion& orientation) {
	getOgreSceneNode()->_setDerivedOrientation(orientation);
}

void MGE::World3DObject::setWorldDirection(Ogre::Vector3 direction) {
	getOgreSceneNode()->setDirection(direction, Ogre::Node::TS_WORLD, Ogre::Vector3::NEGATIVE_UNIT_Z);
	/*
	direction.y = 0;
	
	Ogre::Vector3 src = Ogre::Vector3::NEGATIVE_UNIT_Z;
	// when we want use rotate() instead of setOrientation():
	//   src = getOgreSceneNode()->getOrientation() * src; src.y = 0; src.normalise();
	
	setOrientation( src.getRotationTo(direction) );
	*/
}

void MGE::World3DObject::updateCachedTransform(bool updateAABB, bool recursive, bool updateParent) {
	MGE::OgreUtils::updateCachedTransform(getOgreSceneNode(), updateAABB, recursive, updateParent);
}

int16_t MGE::World3DObject::canMove(
	const Ogre::Vector3& start, const Ogre::Vector3& end,
	float& speedModifier, float& squaredLength, float& heightDiff,
	std::forward_list<MGE::BaseActor*>* triggers,
	Ogre::MovableObject** collisionWith
) const {
	return MGE::PathFinder::IS_NOT_MOVABLE;
}


/*--------------------- World3DObjectImpl : setup and create ---------------------*/

MGE::World3DObjectImpl::World3DObjectImpl(MGE::NamedObject* parent) {
	mainSceneNode = MGE::NamedSceneNodes::getSceneNode( parent->getName() ); // we should have correct mainSceneNode when (others) components are created
}

MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(MGE::World3DObjectImpl, World3D)


bool MGE::World3DObjectImpl::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_child("position")    <<  getOgreSceneNode()->getPosition();
	xmlNode.append_child("orientation") <<  getOgreSceneNode()->getOrientation();
	xmlNode.append_child("scale")       <<  getOgreSceneNode()->getScale();
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_World3DObject World3DObject

Store / restore from its @c \<Component\> node following subnodes:
  - @c \<position\>
  - @c \<orientation\>
  - @c \<scale\>

Used to override actor mainSceneNode transform in save files (in config it's better set correct this params in @ref XMLNode_Node).
*/
bool MGE::World3DObjectImpl::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	pugi::xml_node xmlSubNode;
	bool needUpdate = false;
	
	if ( (xmlSubNode = xmlNode.child("position")) ) {
		needUpdate = true;
		getOgreSceneNode()->setPosition(MGE::XMLUtils::getValue<Ogre::Vector3>(xmlSubNode));
	}
	
	if ( (xmlSubNode = xmlNode.child("orientation")) ) {
		needUpdate = true;
		getOgreSceneNode()->setOrientation(MGE::XMLUtils::getValue<Ogre::Quaternion>(xmlSubNode));
	}
	
	if ( (xmlSubNode = xmlNode.child("scale")) ) {
		needUpdate = true;
		getOgreSceneNode()->setScale(MGE::XMLUtils::getValue<Ogre::Vector3>(xmlSubNode));
	}
	
	if (needUpdate)
		updateCachedTransform();
	
	return true;
}


/*--------------------- World3DObjectImpl : other stuff ---------------------*/

Ogre::SceneNode*   MGE::World3DObjectImpl::getOgreSceneNode() const {
	return mainSceneNode;
}

const Ogre::AxisAlignedBox& MGE::World3DObjectImpl::getAABB() const {
	return aabb;
}

void MGE::World3DObjectImpl::setOgreSceneNode(Ogre::SceneNode* node) {
	mainSceneNode = node;
	MGE::OgreColisionBoundingBox::getLocalAABB(mainSceneNode, &aabb);
}
