/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2000-2014 Torus Knot Software Ltd

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

Based on:
	→ OGRE (MIT licensed)
*/

#include "data/utils/OgreUtils.h"

#include "LogSystem.h"

#include <OgreMaterialManager.h>
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include <OgreManualObject2.h>
#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreRoot.h>
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>
#include <OgreItem.h>

Ogre::Vector2 MGE::OgreUtils::rotateVector2(const Ogre::Vector2& vector, const Ogre::Radian& angle) {
	Ogre::Real cos = Ogre::Math::Cos(angle);
	Ogre::Real sin = Ogre::Math::Sin(angle);
	
	return Ogre::Vector2(vector.x * cos - vector.y * sin, vector.x * sin + vector.y * cos);
}

Ogre::Quaternion MGE::OgreUtils::directionToOrientation(const Ogre::Vector3& direction, const Ogre::Quaternion& currentOrientation) {
	Ogre::Vector3 targetDir = direction.normalisedCopy();
	Ogre::Vector3 yawAxis = currentOrientation * Ogre::Vector3::UNIT_Y;
	
	Ogre::Vector3 xVec = yawAxis.crossProduct(targetDir);
	xVec.normalise();
	Ogre::Vector3 yVec = targetDir.crossProduct(xVec);
	yVec.normalise();
	Ogre::Quaternion unitZToTarget = Ogre::Quaternion(xVec, yVec, targetDir);
	
	// front is negative Z, so do 180 degree turn
	return Ogre::Quaternion(-unitZToTarget.y, -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x);
}

bool MGE::OgreUtils::isChildOfNode(const Ogre::SceneNode* node1, const Ogre::SceneNode* node2) {
	const Ogre::SceneNode* parent = node1;
	do {
		if (parent == node2)
			return true;
		parent = parent->getParentSceneNode();
	} while(parent);
	return false;
}

Ogre::Node* MGE::OgreUtils::getNamedChildOfNode(Ogre::SceneNode* parent, const Ogre::String& cihldName) {
	auto childIter = parent->getChildIterator();
	while(childIter.hasMoreElements()) {
		auto child = childIter.getNext();
		if (child->getName() == cihldName)
			return child;
	}
	return NULL;
}

Ogre::String MGE::OgreUtils::getColorMaterial(const Ogre::ColourValue& color) {
	Ogre::String colorMaterialName("SimpleColorMaterial ");
	colorMaterialName.append(Ogre::StringConverter::toString(color));
	if (! Ogre::MaterialManager::getSingleton().getByName(colorMaterialName, "General")) {
		LOG_XDEBUG("create new material for: " << colorMaterialName);
		Ogre::MaterialPtr matptr = Ogre::MaterialManager::getSingleton().create(colorMaterialName, "General");
		matptr->setReceiveShadows(false);
		matptr->setSelfIllumination(color);
	}
	return colorMaterialName;
}

void MGE::OgreUtils::recursiveDeleteSceneNode(Ogre::Node* pNode, bool deleteParent) {
	Ogre::SceneManager* scnMgr = static_cast<Ogre::SceneNode*>(pNode)->getCreator();
	LOG_XDEBUG(" " << pNode);
	auto objIter   = static_cast<Ogre::SceneNode*>(pNode)->getAttachedObjectIterator();
	auto childIter = pNode->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		
		//m->detachFromParent();
		scnMgr->destroyMovableObject(m);
	}
	
	while(childIter.hasMoreElements()) {
		recursiveDeleteSceneNode( childIter.getNext() );
	}
	
	if (deleteParent) {
		pNode->getParent()->removeChild(pNode);
		scnMgr->destroySceneNode(static_cast<Ogre::SceneNode*>(pNode));
	}
}

void MGE::OgreUtils::recursiveCloneSceneNode( Ogre::SceneNode* src, Ogre::SceneNode* dst ) {
	auto objIter   = src->getAttachedObjectIterator();
	auto childIter = src->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			auto mm = static_cast<Ogre::v1::Entity*>(m)->clone();
			dst->attachObject(mm);
		} else if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			auto mm = dst->getCreator()->createItem( static_cast<Ogre::Item*>(m)->getMesh() );
			dst->attachObject(mm);
		}
	}
	
	while(childIter.hasMoreElements()) {
		Ogre::SceneNode* srcChild = static_cast<Ogre::SceneNode*>( childIter.getNext() );
		Ogre::SceneNode* dstChild = dst->createChildSceneNode();
		
		dstChild->setPosition(srcChild->getPosition());
		dstChild->setOrientation(srcChild->getOrientation());
		dstChild->setScale(srcChild->getScale());
		
		recursiveCloneSceneNode( srcChild, dstChild );
	}
}

void MGE::OgreUtils::recursiveUpdateQueryFlags(Ogre::Node* pNode, int andMask, int orMask) {
	auto objIter   = static_cast<Ogre::SceneNode*>(pNode)->getAttachedObjectIterator();
	auto childIter = pNode->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		m->setQueryFlags(
			(m->getQueryFlags() & andMask) | orMask
		);
	}
	
	while(childIter.hasMoreElements()) {
		recursiveUpdateQueryFlags( childIter.getNext(), andMask, orMask );
	}
}

void MGE::OgreUtils::recursiveUpdateVisibilityFlags(Ogre::Node* pNode, int andMask, int orMask) {
	auto objIter   = static_cast<Ogre::SceneNode*>(pNode)->getAttachedObjectIterator();
	auto childIter = pNode->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		m->setVisibilityFlags(
			(m->getVisibilityFlags() & andMask) | orMask
		);
	}
	
	while(childIter.hasMoreElements()) {
		recursiveUpdateQueryFlags( childIter.getNext(), andMask, orMask );
	}
}

void MGE::OgreUtils::recursiveUpdateBindings(Ogre::Node* pNode, const char* name, const Ogre::Any& any) {
	auto objIter   = static_cast<Ogre::SceneNode*>(pNode)->getAttachedObjectIterator();
	auto childIter = pNode->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		
		Ogre::Any tmpAny =  m->getUserObjectBindings().getUserAny(name);
		if (!tmpAny.isEmpty()) {
			m->getUserObjectBindings().setUserAny( name, any );
		}
	}
	
	while(childIter.hasMoreElements()) {
		recursiveUpdateBindings( childIter.getNext(), name, any );
	}
}

void MGE::OgreUtils::updateCachedTransform(Ogre::Node* pNode, bool updateAABB, bool recursive, bool updateParent) {
	if (updateParent && pNode->getParent()) {
		// call update on parent node (if need) and current node
		pNode->getParent()->_getFullTransformUpdated();
	} else {
		// (typical) we don't need call update on parent of pNode
		// so we do not use _getFullTransformUpdated() or similar
		// we use protected updateFromParentImpl() by fake class NodeUpdater;
		struct NodeUpdater : Ogre::Node {
			void doUpdateFromParent() {
				updateFromParentImpl();
				if (mListener)
					mListener->nodeUpdated(this);
			}
		};
		static_cast<NodeUpdater*>(pNode)->doUpdateFromParent();
	}
	
	// call recursive on all child nodes
	if (recursive) {
		auto childIter = pNode->getChildIterator();
		while(childIter.hasMoreElements()) {
			updateCachedTransform( childIter.getNext(), updateAABB, true, false );
		}
	}
	
	// call on all attached MovableObject to update AABB
	if (updateAABB) {
		auto objIter   = static_cast<Ogre::SceneNode*>(pNode)->getAttachedObjectIterator();
		while(objIter.hasMoreElements()) {
			Ogre::MovableObject* m = objIter.getNext();
			m->getWorldAabbUpdated();
		}
	}
}

Ogre::String MGE::OgreUtils::getColorDatablock(const Ogre::ColourValue& color) {
	Ogre::String colorMaterialName("SimpleColorMaterial ");
	colorMaterialName.append(Ogre::StringConverter::toString(color));
	
	if (! Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT)->getDatablock(colorMaterialName)) {
		LOG_VERBOSE("create new datablock for: " + colorMaterialName);
		
		Ogre::HlmsMacroblock macroblock;
		macroblock.mCullMode = Ogre::CULL_NONE;
		
		Ogre::HlmsUnlitDatablock* datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
			static_cast<Ogre::HlmsUnlit*>( Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT) )->createDatablock(
				colorMaterialName, colorMaterialName,
				macroblock, Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()
			)
		);
		datablock->setUseColour(true);
		datablock->setColour(color);
	}
	return colorMaterialName;
}

Ogre::MeshPtr  MGE::OgreUtils::convertManualToMesh(Ogre::ManualObject* manual, const Ogre::String& name, const Ogre::String& group) {
	class ManualObjectSection : public Ogre::ManualObject::ManualObjectSection {
	public:
		Ogre::VertexArrayObject* getVao() {
			return mVao;
		}
	};
	
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(name, group);
	for (size_t i = 0; i < manual->getNumSections(); ++i) {
		ManualObjectSection* sec = static_cast<ManualObjectSection*>(manual->getSection(i));
		Ogre::SubMesh* subMesh = mesh->createSubMesh();
		
		subMesh->mVao[Ogre::VpNormal].push_back(
			sec->getVao()->clone(Ogre::Root::getSingleton().getRenderSystem()->getVaoManager(), NULL)
		);
		//subMesh->mVao[Ogre::VpShadow].push_back(sec->getVao());
	}
	Ogre::Aabb aabb = manual->getLocalAabb();
	mesh->_setBounds(aabb);
	mesh->_setBoundingSphereRadius(aabb.getRadius());
	mesh->load();
	
	manual->clear();
	manual->_getManager()->destroyManualObject(manual);
	
	return mesh;
}

void MGE::OgreUtils::setDatablock(Ogre::ManualObject* manualObject, Ogre::HlmsDatablock* datablock) {
	for (unsigned int i = 0; i < manualObject->getNumSections(); ++i) {
		static_cast<Ogre::ManualObject::ManualObjectSection*>(manualObject->getSection(i))->setDatablock(
			datablock
		);
	}
}

Ogre::HlmsDatablock* MGE::OgreUtils::getFirstDatablock(Ogre::Item* item) {
	for (size_t k = 0; k < item->getNumSubItems(); ++k) {
		return item->getSubItem(k)->getDatablock();
	}
	return NULL;
}
