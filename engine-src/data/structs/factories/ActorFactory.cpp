/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/factories/ActorFactory.h"

#include "Engine.h"
#include "data/structs/factories/PrototypeFactory.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/ActorMessages.h"
#include "SceneLoader.h"
#include "data/utils/OgreUtils.h"
#include "data/LoadingSystem.h"
#include "data/structs/components/3DWorld.h"
#include "data/QueryFlags.h"
#include "data/utils/NamedSceneNodes.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreSceneManager.h>

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Actor \<actor\>

@c \<actor\> is used for creating actors, should be subnode of @ref XMLNode_Node and has the following subnodes:
  - @ref XMLNode_PrototypeRef for specify prototype used to create actor 
  - @ref XMLNode_Property
  - set of @ref XMLNode_Component subnodes
*/
MGE::BaseActor* MGE::ActorFactory::processActorXMLNode(
	const pugi::xml_node&       xmlNode,
	const MGE::LoadingContext*  /*context*/,
	const MGE::SceneObjectInfo& parent
) {
	MGE::BasePrototype*    gameObjProto = NULL;
	MGE::BaseActor*        gameObj;
	
	auto xmlTagNode = xmlNode.child("Prototype");
	if (xmlTagNode) {
		gameObjProto = MGE::PrototypeFactory::getPtr()->getPrototype(xmlTagNode);
	}
	
	gameObj = MGE::ActorFactory::getPtr()->_createActor(gameObjProto, parent.node);
	gameObj->restoreFromXML(xmlNode, nullptr);
	
	return gameObj;
}

MGE::BaseActor* MGE::ActorFactory::createActor(
	const MGE::BasePrototype* prototype,
	Ogre::String name,
	Ogre::Vector3 position,
	const Ogre::Quaternion& rotation,
	bool findFreePositionOnGround,
	Ogre::SceneManager* scnMgr
) {
	static Ogre::NameGenerator msNameGenerator("UnnamedActor_");
	
	if (name.empty()) {
		do {
			name = msNameGenerator.generate();
		} while (getActor(name));
		LOG_DEBUG("using name: " << name);
	} else if (getActor(name)) {
		LOG_ERROR("Actor with name " + name + " already exists");
		return NULL;
	}
	
	if (!scnMgr) {
		scnMgr = MGE::LoadingSystem::getPtr()->getGameSceneManager();
	}
	
	MGE::BaseActor* gameObj = _createActor(
		prototype,
		MGE::NamedSceneNodes::createSceneNode(name, scnMgr->getRootSceneNode(), Ogre::SCENE_DYNAMIC, position, rotation)
	);
	
	MGE::World3DObject* gameObj3Dworld = gameObj->getComponent<MGE::World3DObject>();
	if (gameObj3Dworld)
		gameObj3Dworld->updateCachedTransform();
	
	if (findFreePositionOnGround) {
		if (gameObj3Dworld) {
			gameObj3Dworld->findAndSetFreePositionOnGround(position);
		} else {
			LOG_WARNING("findFreePositionOnGround is true, but object does not have World3DObject");
		}
	}
	
	return gameObj;
}

MGE::BaseActor* MGE::ActorFactory::reCreateActor(
	MGE::BaseActor*           actor,
	const MGE::BasePrototype* prototype
) {
	Ogre::SceneNode* mainSceneNode = actor->getComponent<MGE::World3DObject>()->getOgreSceneNode();
	destroyActor(actor, false);
	MGE::OgreUtils::recursiveDeleteSceneNode(mainSceneNode, false);
	return _createActor(prototype, mainSceneNode);
}

#include "data/utils/OgreSceneObjectInfo.h"


MGE::BaseActor* MGE::ActorFactory::_createActor(
	const MGE::BasePrototype* prototype,
	Ogre::SceneNode* mainSceneNode
) {
	LOG_INFO("ActorFactory", "Creating actor " << mainSceneNode->getName());
	
	// 1. create Actor object
	MGE::BaseActorImpl* gameObjImpl = new MGE::BaseActorImpl(mainSceneNode->getName(), prototype);
	MGE::BaseActor*     gameObj     = static_cast<MGE::BaseActor*>(gameObjImpl);
	
	// 2. load config from prototype
	if (prototype) {
		LOG_VERBOSE("ActorFactory", "Loading setting from prototype for " << mainSceneNode->getName());
		// open prototype config xml file and get prototype xml node
		pugi::xml_document xmlDoc;
		pugi::xml_node     xmlNode = MGE::BasePrototype::getPrototypeXML(prototype->getLocationInfo(), xmlDoc);
		
		if (xmlNode) {
			// load scene elements from XML node
			MGE::LoadingContext context(mainSceneNode->getCreator());
			MGE::SceneLoader::getPtr()->parseSceneXMLNode(
				xmlNode,
				&context,
				{mainSceneNode, nullptr}
			);
			
			// create actor Components, but not configure it
			MGE::ComponentFactory::getPtr()->restoreComponents(
				xmlNode.child("ActorComponents"), &(gameObjImpl->components), gameObj, mainSceneNode, false
			);
			
			// set scale from prototype to mainSceneNode
			Ogre::Vector3 scale = MGE::XMLUtils::getValue(xmlNode.child("scale"), Ogre::Vector3::UNIT_SCALE);
			mainSceneNode->setScale(scale * mainSceneNode->getScale());
		}
		LOG_VERBOSE("ActorFactory", "Finished loading setting from prototype for " << mainSceneNode->getName());
	} else {
		LOG_WARNING("NULL prototype for: " + gameObj->getName());
	}
	
	// 3.1. get (or create and attach) World3DObject object
	MGE::World3DObject* gameObj3Dworld = gameObj->getComponent<MGE::World3DObject>(MGE::World3DObject::classID, MGE::World3DObject::classID);
	
	// 3.2. set mainSceneNode in World3DObject object and (internally) get full AABB
	gameObj3Dworld->setOgreSceneNode(mainSceneNode);
	
	// 3.3. update query / collision flags
	auto iter1 = static_cast<Ogre::SceneNode*>(mainSceneNode)->getAttachedObjectIterator();
	while(iter1.hasMoreElements()) {
		Ogre::MovableObject* m = iter1.getNext();
		m->setQueryFlags(m->getQueryFlags() | MGE::QueryFlags::GAME_OBJECT);
	}
	
	// 3.4 set bindings to MGE::BaseActor in Ogre::SceneNode
	MGE::Any::setToBindings(mainSceneNode, gameObj);
	
	LOG_INFO("ActorFactory", "Init components for actor " << mainSceneNode->getName());
	
	// 3.5. call init() on actor components created from prototype
	for (auto& iter : gameObjImpl->components) {
		iter.second->init(gameObj);
	}
	
	// 4. register Actor object in global allActors map (name->pointer)
	allActors[gameObjImpl->name] = gameObj;
	
	// 5. send event message
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorCreatedEventMsg(gameObj), gameObj );
	
	LOG_INFO("ActorFactory", "Actor " << mainSceneNode->getName() << " created successfully");
	
	// 6. return created Actor object
	return gameObj;
}

void MGE::ActorFactory::destroyActor(MGE::BaseActor* obj, bool deleteSceneNode) {
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorDestroyEventMsg(obj), obj );
	
	auto iter = MGE::ActorFactory::allActors.find( obj->getName() );
	if (iter != MGE::ActorFactory::allActors.end()) {
		MGE::ActorFactory::allActors.erase(iter);
	}
	
	if (deleteSceneNode) {
		MGE::OgreUtils::recursiveDeleteSceneNode(
			obj->getComponent<MGE::World3DObject>()->getOgreSceneNode(),
			false
		);
	}
	delete obj;
}

bool MGE::ActorFactory::unload() {
	LOG_INFO("Destroy all actors (without its scene nodes)");
	
	auto iter = allActors.begin();
	while(iter != allActors.end()) {
		MGE::BaseActor* a = iter->second;
		++iter;
		delete a;
	}
	allActors.clear();
	return true;
}

bool MGE::ActorFactory::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	LOG_INFO("store all actors");
	
	for (auto& iter : allActors) {
		iter.second->storeToXML(xmlNode, onlyRef);
	}
	return true;
}

bool MGE::ActorFactory::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("restore actors info");
	
	std::set<MGE::BaseActor*> preloaded;
	for (auto& iter : allActors) {
		preloaded.insert(iter.second);
	}
	
	// restoring all Actors for completing map MGE::ActorFactory::allActors
	for (auto xmlSubNode : xmlNode) {
		std::string gameObjName = xmlSubNode.attribute("name").as_string();
		MGE::BaseActor* gameObj = getActor(gameObjName);
		MGE::BasePrototype* gameObjProto = MGE::PrototypeFactory::getPtr()->getPrototype(xmlSubNode.child("Prototype"));
		
		if (!gameObj) {
			LOG_DEBUG("Create: " << gameObjName << " from " << gameObjProto->getName());
			gameObj = createActor(gameObjProto, gameObjName, Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY, false);
		} else if (gameObj->getPrototype() != gameObjProto) {
			LOG_DEBUG("Change prototype for: " << gameObjName << " to " << gameObjProto->getName() << " => recreate Actor");
			gameObj = reCreateActor(gameObj, gameObjProto);
		} else {
			LOG_DEBUG("Mark as correct: " << gameObjName);
			preloaded.erase(gameObj);
		}
	}
	
	// remove unwanted Actors
	for (auto& iter : preloaded) {
		delete iter;
	}
	
	// restore() of actor need a complete map MGE::ActorFactory::allActors
	// (because add pointers to some other actors to internal lists based on its names)
	// so must be call in separate loop
	for (auto xmlSubNode : xmlNode) {
		std::string_view gameObjName = xmlSubNode.attribute("name").as_string();
		MGE::BaseActor* gameObj = getActor(gameObjName);
		LOG_DEBUG("Restore: " << gameObjName);
		gameObj->restoreFromXML(xmlSubNode, nullptr);
	}
	return true;
}


MGE::ActorFactory::ActorFactory() : MGE::SaveableToXML<ActorFactory>(401, 601) {
	LOG_INFO("Create ActorFactory");
	// register load scene element listener
	MGE::SceneLoader::getPtr()->addSceneNodesCreateListener(
		"actor", reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::ActorFactory::processActorXMLNode)
	);
}

MGE::ActorFactory::~ActorFactory() {
	MGE::SceneLoader::getPtr()->remSceneNodesCreateListener(
		reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::ActorFactory::processActorXMLNode)
	);
}

void MGE::ActorFactory::findActors(const Ogre::Vector3& point, float range, std::multimap<float, MGE::BaseActor*>* results) {
	float srange = range * range;
	for (auto& iter : allActors) {
		MGE::World3DObject* w3dObj = iter.second->getComponent<MGE::World3DObject>();
		if (w3dObj) {
			float sdist = w3dObj->getWorldPosition().squaredDistance(point);
			if (sdist < srange) {
				results->insert(std::make_pair(sdist, iter.second));
			}
		}
	}
}
