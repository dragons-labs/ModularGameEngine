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

#include "rendering/markers/ObjectHighlight.h"

#include "LogSystem.h"
#include <OgreItem.h>
#include <OgreEntity.h>
#include <OgreHlms.h>
#include <Hlms/Pbs/OgreHlmsPbsDatablock.h>
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>
#include <OgreNameGenerator.h>

void MGE::ObjHighlightManager::enable(Ogre::SceneNode* node, const Ogre::ColourValue& color) {
	HighlightObject* hObj = new HighlightObject();
	
	enable(node, color, hObj);
	
	auto objHighlightInfo = highlightObjects.find(node);
	if (objHighlightInfo == highlightObjects.end()) {
		// only for first highlight remember orginal materials info
		highlightObjects[node] = hObj;
	} else {
		// otherwise delete previous highlight materials (using pointers in hObj);
		for (auto& iter : hObj->orgDatablocks) {
			delete iter.second;
		}
	}
}

Ogre::HlmsDatablock* MGE::ObjHighlightManager::replaceDatablock(HighlightObject* hObj, Ogre::Renderable* renderable, const Ogre::ColourValue& color) {
	static Ogre::NameGenerator materialNamesGenerator("ObjHighligh_MaterialDatablock_");
	
	Ogre::HlmsDatablock* datablock = renderable->getDatablock();
	hObj->orgDatablocks[renderable] = datablock;
	
	datablock = datablock->clone(materialNamesGenerator.generate());
	auto type = datablock->getCreator()->getType();
	if (type == Ogre::HLMS_UNLIT) {
		Ogre::HlmsUnlitDatablock* datablockUnlit = dynamic_cast<Ogre::HlmsUnlitDatablock*>(datablock);
		if (datablockUnlit) {
			datablockUnlit->setColour(color);
			datablockUnlit->setUseColour(true);
		} else {
			LOG_WARNING("ObjHighlightManager: invalid HLMS_UNLIT cast");
		}
	} else if (type == Ogre::HLMS_PBS) {
		Ogre::HlmsPbsDatablock* datablockUnlit = dynamic_cast<Ogre::HlmsPbsDatablock*>(datablock);
		if (datablockUnlit) {
			datablockUnlit->setEmissive(Ogre::Vector3(
				std::fmin(color.r * 0.15, 0.5), std::fmin(color.g * 0.15, 0.5), std::fmin(color.b * 0.15, 0.5)
			));
		} else {
			LOG_WARNING("ObjHighlightManager: invalid HLMS_PBS cast");
		}
	}
	
	renderable->setDatablock(datablock);
	return datablock;
}

void MGE::ObjHighlightManager::enable(Ogre::SceneNode* node, const Ogre::ColourValue& color, HighlightObject* hObj) {
	auto objIter   = node->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* movable = objIter.getNext();
		if (movable->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			Ogre::v1::Entity* entity = static_cast<Ogre::v1::Entity*>(movable);
			
			size_t numSubEntities = entity->getNumSubEntities ();
			for (size_t i=0; i < numSubEntities; ++i) {
				replaceDatablock(hObj, entity->getSubEntity(i), color);
			}
		} else if (movable->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			Ogre::Item* item = static_cast<Ogre::Item*>(movable);
			
			size_t numSubItems = item->getNumSubItems();
			for (size_t i=0; i < numSubItems; ++i) {
				replaceDatablock(hObj, item->getSubItem(i), color);
			}
		}
	}
	
	while(childIter.hasMoreElements()) {
		enable( static_cast<Ogre::SceneNode*>(childIter.getNext()), color, hObj );
	}
}

void MGE::ObjHighlightManager::disable(Ogre::SceneNode* node) {
	auto objHighlightInfo = highlightObjects.find(node);
	if (objHighlightInfo != highlightObjects.end()) {
		// restore oryginal material datablocks and delete highlight materials
		for (auto& iter : objHighlightInfo->second->orgDatablocks) {
			auto tmp = iter.first->getDatablock();
			iter.first->setDatablock(iter.second);
			delete tmp;
		}
		// remove form map of highlighted objects
		delete objHighlightInfo->second;
		highlightObjects.erase(objHighlightInfo);
	} else {
		LOG_DEBUG("disable highlight of not highlighted object");
	}
}
