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

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "LogSystem.h"

#include <OgreStringConverter.h>

MGE::ComponentFactory::ComponentFactory() {
	LOG_INFO("Create ComponentFactory");
	nameToID["REMOVED"] = 0;
	idToNames[0] = "REMOVED";
	MGE::ComponentFactoryRegistrar::getPtr()->initAll(this);
}

void MGE::ComponentFactory::registerComponent(
	int classID, const std::string& stringClassID, ComponentCreator creatorFunction
) {
	auto iterA = registeredComponents.find(classID);
	if (iterA != registeredComponents.end()) {
		throw std::runtime_error("Component with classID=" + std::to_string(classID) + " already registred. StringID: " + idToNames[classID] + " vs " + stringClassID);
		return;
	}
	
	auto iterB = nameToID.find(stringClassID);
	if (iterB != nameToID.end()) {
		throw std::runtime_error("Component with stringClassID=" + stringClassID + " already registred");
		return;
	}
	
	LOG_INFO("Register component " << stringClassID << " with numeric classID: " << classID);
	registeredComponents[classID] = creatorFunction;
	nameToID[stringClassID] = classID;
	idToNames[classID] = stringClassID;
}

MGE::BaseComponent* MGE::ComponentFactory::createComponent(
	int classID,
	ComponentsCollection* components,
	MGE::NamedObject* parent,
	const pugi::xml_node& config
) {
	auto iter = registeredComponents.find(classID);
	if (iter == registeredComponents.end()) {
		LOG_ERROR("Can't find create function for Component with classID=" << classID);
		return NULL;
	}
	
	std::set<int> typeIDs;
	MGE::BaseComponent* newComponent = registeredComponents[classID](parent, config, &typeIDs, classID);
	for (auto& iter2 : typeIDs) {
		if ((*components)[iter2] != 0 && (*components)[iter2] != newComponent) {
			LOG_ERROR(
				"Previous registered diffrent component object for typeID=" << iter2 <<
				" oldClassID=" << (*components)[iter2]->getClassID() << " newClassID=" << newComponent->getClassID() <<
				" ... skip register for this typeID"
			);
			continue;
		}
		(*components)[iter2] = newComponent;
	}
	
	return newComponent;
}


void MGE::ComponentFactory::storeComponents(
	pugi::xml_node& xmlNode,
	const ComponentsCollection* mapPtr
) {
	std::set<MGE::BaseComponent*> stored;
	for (auto& iter : *mapPtr) {
		auto xmlStoreNode = xmlNode.append_child("Component");
		xmlStoreNode.append_attribute("classID") << getName(iter.second ? iter.second->getClassID() : 0);
		xmlStoreNode.append_attribute("typeID")  << getName(iter.first);
		if (iter.second && stored.find(iter.second) == stored.end()) {
			// components pointer can occur in mapPtr many times
			// full store only on first occur
			iter.second->storeToXML(xmlStoreNode);
			stored.insert(iter.second);
		}
	}
}

void MGE::ComponentFactory::restoreComponents(
	const pugi::xml_node& xmlNode,
	ComponentsCollection* mapPtr,
	MGE::NamedObject* parent,
	Ogre::SceneNode* sceneNode,
	bool callInit
) {
	ComponentsCollection createdComponents;
	for (auto xmlSubNode : xmlNode.children("Component")) {
		int classID = getID(xmlSubNode.attribute("classID").as_string());
		auto xmlAttrib = xmlSubNode.attribute("typeID");
		int typeID  = xmlAttrib ? getID(xmlAttrib.as_string()) : classID;
		if (classID < 0 || typeID < 0) {
			LOG_WARNING("unknown classID or typeID value in <Component/>");
			continue;
		}
		
		LOG_INFO("restore component with classID=" << classID << " for typeID=" << typeID);
		
		// check if component for this typeID exist
		MGE::BaseComponent* component = NULL;
		auto iter = mapPtr->find(typeID);
		if (iter != mapPtr->end()) {
			component = iter->second;
		}
		
		if (component && classID != component->getClassID()) {
			LOG_INFO("remove old component registered for this typeID, it use diffrent class ID" << component->getClassID());
			// removed component (classID == 0) or component with changed classID (we remove and recreate it)
			iter->second = 0;
			bool existWithOtherTypeID = false;
			for (auto& iter2 : *mapPtr) {
				if (iter2.second == component) {
					existWithOtherTypeID = true;
					break;
				}
			}
			if (!existWithOtherTypeID)
				delete component;
			component = 0;
		}
		
		if (classID == 0) {
			// removed component ... don't recreate or restore
			continue;
		}
		
		if (!component) {
			LOG_INFO("create new component for classID " << classID);
			auto iter2 = createdComponents.find(classID);
			if (iter2 != createdComponents.end()) {
				// we have restored this component, so only add to collection with other typeID
				(*mapPtr)[typeID] = iter2->second;
			} else {
				// create component using XML config, after this will be call restore() on this same XML node
				component = MGE::ComponentFactory::getPtr()->createComponent( classID, mapPtr, parent, xmlSubNode );
			}
		}
		
		if (component) {
			// restore from config/save xml
			if (xmlSubNode.first_child()) {
				// call restore only when non empty config/save xml node
				component->restoreFromXML( xmlSubNode, parent, sceneNode );
			}
			if (callInit) {
				// call init, skip only when pre-load from prototype 
				component->init(parent);
			}
			// add to maps
			(*mapPtr)[typeID] = component;
			createdComponents[classID] = component;
		}
	}
}

void MGE::ComponentFactory::clearMap(
	ComponentsCollection* mapPtr
) {
	std::set<MGE::BaseComponent*> mapPtrSet;
	for (auto& iter : *mapPtr) {
		mapPtrSet.insert(iter.second);
	}
	for (auto& iter : mapPtrSet) {
		delete iter;
	}
	mapPtr->clear();
}

void MGE::ComponentFactory::removeFromMap(
	int typeID,
	ComponentsCollection* mapPtr
) {
	// find for component by typeID
	auto iter = mapPtr->find(typeID);
	if (iter != mapPtr->end()) {
		// check if it's not registered with other typeID
		bool existWithOtherTypeID = false;
		for (auto& iter2 : *mapPtr) {
			if (iter2.second == iter->second) {
				existWithOtherTypeID = true;
				break;
			}
		}
		// if not delete component
		if (!existWithOtherTypeID)
			delete iter->second;
		// always set value in map to ZERO
		// (DO NOT delete map entry!)
		iter->second = 0;
	}
}

int MGE::ComponentFactory::getID(null_end_string str) const {
	auto iter = nameToID.find(str);
	if (iter != nameToID.end())
		return iter->second;
	else
		return strtoll(str, NULL, 0);
}

std::string MGE::ComponentFactory::getName(int typeID) const {
	auto iter = idToNames.find(typeID);
	if (iter != idToNames.end())
		return iter->second;
	else
		return Ogre::StringConverter::toString(typeID);
}
