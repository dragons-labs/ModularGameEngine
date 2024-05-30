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

#include "data/structs/BaseActor.h"

#include "data/structs/BasePrototype.h"
#include "data/structs/BaseComponent.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/components/3DWorld.h"

#include "data/property/G11n.h"

#include "data/property/PropertySet.h"

#include "LogSystem.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>

MGE::BaseActor* MGE::BaseActor::get(const Ogre::SceneNode* node) {
	if (!node)
		return NULL;
	
	return MGE::Any::getFromBindings(node).getValue<MGE::BaseActor*>(NULL);
}

MGE::BaseActor* MGE::BaseActor::get(const Ogre::MovableObject* movable) {
	if (!movable)
		return NULL;
	
	return MGE::Any::getFromBindings(movable->getParentSceneNode()).getValue<MGE::BaseActor*>(NULL);
}


const std::string& MGE::BaseActorImpl::getType() const {
	return MGE::BaseActor::TypeName();
}

const std::string& MGE::BaseActorImpl::getName() const {
	return name;
}

const MGE::BasePrototype* MGE::BaseActorImpl::getPrototype() const {
	return prototype;
}

const MGE::Any& MGE::BaseActorImpl::getProperty(const std::string_view& key) const {
	const MGE::Any& ret = properties.getProperty(key);
	if (ret.isEmpty() && prototype) {
		return prototype->getProperty(key);
	}
	
	LOG_DEBUG("getProperty for " << key << " return: " << ret);
	return ret;
}

size_t MGE::BaseActorImpl::remProperty(const std::string_view& key) {
	if (prototype && prototype->hasProperty(key)) {
		properties.addProperty(static_cast<std::string>(key), MGE::Any::EMPTY, true);
		return -1;
	} else {
		return properties.remProperty(key);
	}
}

bool MGE::BaseActorImpl::addProperty(const std::string_view& key, const MGE::Any& val, bool replace) {
	return properties.addProperty(key, val, replace);
}

bool MGE::BaseActorImpl::setProperty(const std::string_view& key, const MGE::Any& val) {
	return properties.setProperty(key, val);
}

bool MGE::BaseActorImpl::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	auto xmlStoreNode = xmlNode.append_child("Actor");
	xmlStoreNode.append_attribute("name") << name;
	
	if (onlyRef)
		return true;
	
	if (prototype) {
		prototype->storeToXML(xmlStoreNode.append_child("Prototype"));
	}
	
	properties.storeToXML(xmlStoreNode);
	
	MGE::ComponentFactory::getPtr()->storeComponents(xmlStoreNode, &components);
	return true;
}

bool MGE::BaseActorImpl::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("Restore actor " + getName());
	
	properties.clearAll();
	properties.restoreFromXML(xmlNode, MGE::G11n::getLang(), true);
	LOG_VERBOSE("Restore actor " + getName(), "Actor properties: " << properties);
	
	Ogre::SceneNode* sceneNode = NULL;
	MGE::World3DObject* w3d = static_cast<MGE::World3DObject*>(
		getComponent(MGE::World3DObject::classID)
	);
	if (w3d)
		sceneNode = w3d->getOgreSceneNode();
	
	LOG_INFO("Restore actor " + getName(), "restoring components");
	MGE::ComponentFactory::getPtr()->restoreComponents(xmlNode, &components, this, sceneNode);
	
	LOG_INFO("Restore actor " + getName(), "done");
	return true;
}

const MGE::BaseComponent* MGE::BaseActorImpl::getComponent(int typeID) const {
	auto iter = components.find(typeID);
	if (iter != components.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

MGE::BaseComponent* MGE::BaseActorImpl::getComponent(int typeID, int classID) {
	auto iter = components.find(typeID);
	if (iter != components.end()) {
		return iter->second;
	} else if (classID != 0) {
		return MGE::ComponentFactory::getPtr()->createComponent(classID, &components, this);
	} else {
		return NULL;
	}
}

MGE::BaseActorImpl::BaseActorImpl(
	const std::string&           _name,
	const MGE::BasePrototype* _prototype
) : 
	name(_name), prototype(_prototype)
{}

MGE::BaseActorImpl::~BaseActorImpl() {
	LOG_DEBUG("delete actor " << name);
	MGE::ComponentFactory::clearMap(&components);
}
