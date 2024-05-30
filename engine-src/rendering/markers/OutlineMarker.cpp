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

#include "rendering/markers/OutlineMarker.h"

#include "LogSystem.h"
#include "data/utils/OgreUtils.h"
#include "rendering/utils/RenderQueueGroups.h"

#include <OgreSceneManager.h>
#include <OgreItem.h>
#include <OgreEntity.h>

void MGE::OutlineVisualMarker::recursiveCreateStencilGlowNode(Ogre::SceneNode* src, Ogre::SceneNode* dst, const Ogre::String& material) {
	auto objIter   = src->getAttachedObjectIterator();
	auto childIter = src->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			if (m->getRenderQueueGroup() != MGE::RenderQueueGroups::DEFAULT_OBJECTS_V1)
				continue;
			Ogre::v1::Entity* mm = static_cast<Ogre::v1::Entity*>(m)->clone();
			m->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OBJECT_V1);
			mm->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OUTLINE_V1);
			mm->setDatablock(material);
			dst->attachObject(mm);
		} else if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			if (m->getRenderQueueGroup() != MGE::RenderQueueGroups::DEFAULT_OBJECTS_V2)
				continue;
			Ogre::Item* mm = dst->getCreator()->createItem( static_cast<Ogre::Item*>(m)->getMesh() );
			m->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OBJECT_V2);
			mm->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OUTLINE_V2);
			mm->setDatablock(material);
			dst->attachObject(mm);
		}
	}
	
	while(childIter.hasMoreElements()) {
		Ogre::SceneNode* srcChild = static_cast<Ogre::SceneNode*>( childIter.getNext() );
		Ogre::SceneNode* dstChild = dst->createChildSceneNode();
		
		dstChild->setPosition(srcChild->getPosition());
		dstChild->setOrientation(srcChild->getOrientation());
		dstChild->setScale(srcChild->getScale());
		
		recursiveCreateStencilGlowNode( srcChild, dstChild, material );
	}
}

MGE::OutlineVisualMarker::OutlineVisualMarker(const Ogre::String& material, int mode, float linesThickness, Ogre::SceneNode* node) : 
	MGE::VisualMarker(mode)
{
	stencilGlowNode = node->getCreator()->createSceneNode();
	recursiveCreateStencilGlowNode(node, stencilGlowNode, material);
	stencilGlowNode->setScale(Ogre::Vector3(1.0+linesThickness));
	node->addChild(stencilGlowNode);
}

void MGE::OutlineVisualMarker::recursiveSetMaterial(Ogre::SceneNode* node, const Ogre::String& material) {
	auto objIter   = node->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			static_cast<Ogre::v1::Entity*>(m)->setDatablock(material);
		} else if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			static_cast<Ogre::Item*>(m)->setDatablock(material);
		}
	}
	
	while(childIter.hasMoreElements()) {
		recursiveSetMaterial( static_cast<Ogre::SceneNode*>( childIter.getNext() ), material );
	}
}

void MGE::OutlineVisualMarker::update(int, const Ogre::String& markerMaterial, float linesThickness) {
	stencilGlowNode->setScale(Ogre::Vector3(1.0+linesThickness));
	recursiveSetMaterial(stencilGlowNode, markerMaterial);
}

void MGE::OutlineVisualMarker::recursiveCleanStencilGlow(Ogre::SceneNode* node) {
	auto objIter   = node->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			if (m->getRenderQueueGroup() == MGE::RenderQueueGroups::STENCIL_GLOW_OBJECT_V1)
				m->setRenderQueueGroup(MGE::RenderQueueGroups::DEFAULT_OBJECTS_V1);
		} else if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			if (m->getRenderQueueGroup() == MGE::RenderQueueGroups::STENCIL_GLOW_OBJECT_V2)
				m->setRenderQueueGroup(MGE::RenderQueueGroups::DEFAULT_OBJECTS_V2);
		}
	}
	
	while(childIter.hasMoreElements()) {
		recursiveCleanStencilGlow( static_cast<Ogre::SceneNode*>( childIter.getNext() ) );
	}
}

MGE::OutlineVisualMarker::~OutlineVisualMarker() {
	Ogre::SceneNode* node = stencilGlowNode->getParentSceneNode();
	MGE::OgreUtils::recursiveDeleteSceneNode(stencilGlowNode);
	recursiveCleanStencilGlow(node);
}
