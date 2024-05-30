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

#include "data/structs/components/ObjectOwner.h"

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "LogSystem.h"
#include "Engine.h"

/*--------------------- ObjectOwner : setup and create ---------------------*/

MGE::ObjectOwner::ObjectOwner() :
	lastUpdateTime(std::chrono::steady_clock::now())
{}

MGE_ACTOR_COMPONENT_CREATOR(MGE::ObjectOwner, ObjectOwner) {
	typeIDs->insert(MGE::ObjectOwner::classID);
	return new MGE::ObjectOwner();
}

bool MGE::ObjectOwner::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	for (auto& iter : ownedObjects) {
		auto xmlStoreNode = xmlNode.append_child("OwnedObject");
		xmlStoreNode.append_attribute("currentQuantity") << iter.second.currentQuantity;
		xmlStoreNode.append_attribute("plannedQuantity") << iter.second.plannedQuantity;
		iter.first->storeToXML(xmlStoreNode, true);
	}
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_ObjectOwner ObjectOwner

Store / restore from its @c \<Component\> node set of @c \<OwnedObject\> subnodes. Each @c \<OwnedObject\> have attributes:
  - @c currentQuantity
  - @c plannedQuantity
  .
and subnode:
  - @ref XMLNode_ActorName xor @ref XMLNode_PrototypeRef

See @ref MGE::ObjectOwner for details.
*/
bool MGE::ObjectOwner::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	for (auto xmlSubNode : xmlNode.children("OwnedObject")) {
		MGE::NamedObject* gameObj = MGE::NamedObject::get( xmlSubNode );
		if (!gameObj) {
			LOG_ERROR("No object info in OwnedObject");
		} else {
			ownedObjects[gameObj] = {
				xmlSubNode.attribute("currentQuantity").as_int(),
				xmlSubNode.attribute("plannedQuantity").as_int()
			};
		}
	}
	return true;
}


/*--------------------- ObjectOwner : other stuff ---------------------*/

void MGE::ObjectOwner::set(MGE::NamedObject* obj, short current, short planned) {
	if (current <= 0 && planned <= 0) {
		ownedObjects.erase(obj);
	} else {
		ownedObjects[obj] = {current, planned};
	}
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
}

void MGE::ObjectOwner::update(MGE::NamedObject* obj, int current, int future) {
	auto iter = ownedObjects.find(obj);
	if (iter != ownedObjects.end()) {
		iter->second.plannedQuantity += future;
		iter->second.currentQuantity += current;
		if (iter->second.plannedQuantity <= 0 && iter->second.currentQuantity <= 0) {
			ownedObjects.erase(iter);
		}
	} else if (current > 0 || future > 0) {
		set(obj, current, future);
	} else {
		LOG_WARNING("call ObjectOwner::update with object not in ownedObjects and current < 0 and future < 0");
		return;
	}
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
}

void MGE::ObjectOwner::resetPlanned() {
	for (auto& iter : ownedObjects) {
		iter.second.plannedQuantity = iter.second.currentQuantity;
	}
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
}
