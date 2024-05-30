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

Inspired by:
	→ OGRE (MIT licensed)
*/

#include "physics/utils/OgreColisionBoundingBox.h"

#include "LogSystem.h"
#include "data/utils/OgreUtils.h"

#include <OgreMovableObject.h>
#include <OgreItem.h>
#include <OgreEntity.h>
#include <OgreRay.h>

#ifdef MGE_DEBUG_FREEPATH_VISUAL
	#include "rendering/markers/VisualMarkers.h"
#endif

void MGE::OgreColisionBoundingBox::getLocalAABB(const Ogre::SceneNode* node, Ogre::AxisAlignedBox* aabb, bool getScaled) {
	Ogre::Aabb new_aabb(Ogre::Aabb::BOX_ZERO);
	_getLocalAABB(node, &new_aabb, getScaled);
	aabb->setExtents(new_aabb.getMinimum(), new_aabb.getMaximum());
}

void MGE::OgreColisionBoundingBox::getLocalAABB(const Ogre::SceneNode* node, Ogre::Aabb* aabb, bool getScaled) {
	*aabb = Ogre::Aabb::BOX_ZERO;
	_getLocalAABB(node, aabb, getScaled);
}

void MGE::OgreColisionBoundingBox::_getLocalAABB( const Ogre::SceneNode* node, Ogre::Aabb* aabb, bool getScaled ) {
	auto objIter   = node->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		if ((m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) || (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME)) {
			aabb->merge(m->getLocalAabb());
		}
	}
	
	while(childIter.hasMoreElements()) {
		Ogre::SceneNode* child = static_cast<Ogre::SceneNode*>(childIter.getNext());
		
		Ogre::Aabb new_aabb;
		_getLocalAABB( child, &new_aabb );
		
		Ogre::Matrix4 xform;
		xform.makeTransform(child->getPosition(), child->getScale(), child->getOrientation());
		new_aabb.transformAffine(xform);
		aabb->merge(new_aabb);
	}
	
	if (getScaled) {
		Ogre::Matrix4 xform;
		xform.makeTransform(Ogre::Vector3::ZERO, node->getScale(), Ogre::Quaternion::IDENTITY);
		aabb->transformAffine(xform);
	}
}

bool MGE::OgreColisionBoundingBox::isFreeSphere(Ogre::SceneManager* scnMgr, const Ogre::Vector3& position, Ogre::Real radius, int queryMask, std::list<Ogre::MovableObject*>* collisionObjects) {
	Ogre::SphereSceneQuery* volScnQuery = scnMgr->createSphereQuery( Ogre::Sphere(position, radius), queryMask );
	Ogre::SceneQueryResult& result = volScnQuery->execute();
	
	bool ret = result.movables.empty();
	if (!ret) {
		#ifdef MGE_DEBUG_IS_FREE_POSITION
		LOG_VERBOSE( "searching for free position with radius=" << radius << " @ " << position <<
			" found: " << result.movables.front()->getName() << " / " << result.movables.front()->getParentSceneNode()->getName()
		);
		#endif
		if (collisionObjects) {
			for (auto& iter : result.movables)
				collisionObjects->push_back(iter);
		}
	}
	
	scnMgr->destroyQuery(volScnQuery);
	return ret;
}

bool MGE::OgreColisionBoundingBox::isFreePosition(
	const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb,
	const Ogre::Vector3& newPosition, const Ogre::Quaternion& newOrientation, const Ogre::Vector3& newScale,
	int queryMask, std::list<Ogre::MovableObject*>* collisionObjects
) {
	#ifdef MGE_DEBUG_IS_FREE_POSITION
	LOG_VERBOSE( "isFreePosition: aabb=" << aabb << " newPosition=" << newPosition <<
		" newOrientation=" << newOrientation << " newScale=" << newScale << " queryMask=" << queryMask
	);
	#endif
	bool ret = true;
	
	// 1. create world AABB
	Ogre::AxisAlignedBox world_aabb(aabb);
	Ogre::Matrix4 xform;
	xform.makeTransform(newPosition, newScale, newOrientation);
	world_aabb.transformAffine(xform);
	
	// 2. do scene query with world AABB
	Ogre::AxisAlignedBoxSceneQuery* volScnQuery = node->getCreator()->createAABBQuery(world_aabb, queryMask);
	Ogre::SceneQueryResult& queryResult = volScnQuery->execute();
	for(auto iter: queryResult.movables) {
		// 3a. check each results of scene query - base tests
		Ogre::SceneNode* iterNode = iter->getParentSceneNode();
		
		if(!iterNode)
			continue;
		
		if (MGE::OgreUtils::isChildOfNode(iterNode, node))
			continue;
		
		// 3b. check each results of scene query - oriented bounding box check
		Ogre::Aabb aabb2 = iter->getLocalAabb();
		if (intersects( aabb, node, newPosition, newOrientation, newScale, Ogre::AxisAlignedBox(aabb2.getMinimum(), aabb2.getMaximum()), iterNode )) {
			#ifdef MGE_DEBUG_IS_FREE_POSITION
			LOG_VERBOSE( "collision with: " << iterNode->getName() );
			#endif
			ret = false;
			if (collisionObjects) {
				collisionObjects->push_back(iter);
			} else {
				break;
			}
		}
		#ifdef MGE_DEBUG_IS_FREE_POSITION
		LOG_VERBOSE( "non-real collision with: " << iterNode->getName() );
		#endif
	}
	node->getCreator()->destroyQuery(volScnQuery);
	
	return ret;
}

bool MGE::OgreColisionBoundingBox::isFreePath(
	const Ogre::SceneNode* node,
	const Ogre::AxisAlignedBox& aabb,
	const Ogre::Vector3& start,
	const Ogre::Vector3& end,
	int queryMask,
	std::list<Ogre::MovableObject*>* collisionObjects
) {
	Ogre::AxisAlignedBox newAABB(aabb);
	newAABB.getMaximum().z /= 2.0; // back of moving object -> reduce AABB length of back part (related to rotation point) of object
	newAABB.getMinimum().z -= start.distance(end); // front of moving object (should be negative value!) -> increase by distance to destination
	Ogre::Quaternion newOrientation = Ogre::Vector3::NEGATIVE_UNIT_Z.getRotationTo(end - start, Ogre::Vector3::UNIT_Y);
	
	#ifdef MGE_DEBUG_FREEPATH_VISUAL
		static Ogre::SceneNode* newNode = 0;
		if (newNode) {
			MGE::VisualMarkersManager::getPtr()->hideMarker(newNode);
		} else {
			newNode = node->getCreator()->getRootSceneNode()->createChildSceneNode();
		}
		newNode->setPosition(end);
		newNode->setOrientation(newOrientation);
		MGE::VisualMarkersManager::getPtr()->showMarker(newNode, &newAABB, 0, MGE::OgreUtils::getColorDatablock(Ogre::ColourValue(0, 0.95, 0)), 0);
	#endif
	
	return isFreePosition(node, newAABB, end, newOrientation, node->_getDerivedScale(), queryMask, collisionObjects);
}

#ifdef  MGE_DEBUG_INTERSECTS_VISUAL
#ifndef MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE
#define MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE 1
#endif
#endif

bool MGE::OgreColisionBoundingBox::_intersects(
	const Ogre::AxisAlignedBox& aabb1,  const Ogre::AxisAlignedBox& aabb2,
	const Ogre::Matrix4&        from1,  const Ogre::Matrix4&        from2,
	const Ogre::Vector3&        scale1, const Ogre::Vector3&        scale2
	#ifdef MGE_DEBUG_INTERSECTS_VISUAL
	, const Ogre::SceneNode* node, const Ogre::Vector3& nodeDerivedPosition, const Ogre::Quaternion& nodeDerivedOrientation, int callID
	#endif
) {
	// 1. get aabb2 as 4 corners (ray origin) each with 3 direction
	const Ogre::Vector3& min = aabb2.getMinimum();
	const Ogre::Vector3& max = aabb2.getMaximum();
	#if 22 == 11
           .-------B
          /|      /|
         / |     / |
        C-------.  |
        |  A----|--.
        | /     | /
        |/      |/
        .-------D
	#endif
	Ogre::Vector3 a(min.x, min.y, min.z);
	Ogre::Vector3 b(max.x, max.y, min.z);
	Ogre::Vector3 c(min.x, max.y, max.z);
	Ogre::Vector3 d(max.x, min.y, max.z);
	
	// 2. convert aabb2 A,B,C,D corners from node2 LOCAL space to node1 LOCAL space
	Ogre::Matrix4 from2To1 = from1.inverse() * from2;
	a = from2To1 * a;
	b = from2To1 * b;
	c = from2To1 * c;
	d = from2To1 * d;
	
	// 3. convert aabb2 directions from node2 LOCAL space to node1 LOCAL space
	from2To1.setTrans(Ogre::Vector3::ZERO);
	Ogre::Vector3 nDirX( from2To1 * Ogre::Vector3::NEGATIVE_UNIT_X );
	Ogre::Vector3 nDirY( from2To1 * Ogre::Vector3::NEGATIVE_UNIT_Y );
	Ogre::Vector3 nDirZ( from2To1 * Ogre::Vector3::NEGATIVE_UNIT_Z );
	Ogre::Vector3 pDirX( -nDirX );
	Ogre::Vector3 pDirY( -nDirY );
	Ogre::Vector3 pDirZ( -nDirZ );
	
	#ifdef MGE_DEBUG_INTERSECTS_VISUAL
	if (node) {
		static Ogre::SceneNode* vn[10]{};
		
		Ogre::SceneNode*& parent = vn[0 + 5*callID];
		if (parent)
			parent->getCreator()->destroySceneNode(parent);
		parent = node->getCreator()->getRootSceneNode()->createChildSceneNode();
		parent->setPosition(nodeDerivedPosition);
		parent->setOrientation(nodeDerivedOrientation);
		
		for (int i=1; i<5; ++i) {
			Ogre::SceneNode*& nn = vn[i + 5*callID];
			if (nn)
				nn->getCreator()->destroySceneNode(nn);
			nn = parent->createChildSceneNode();
			nn->setDirection(nDirZ);
			switch (i) {
				case 1:
					nn->setPosition(a);
					nn->setScale( Ogre::Vector3(1,1,1) * MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE );
					break;
				case 2:
					nn->setPosition(b);
					nn->setScale( Ogre::Vector3(-1,-1,1) * MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE );
					break;
				case 3:
					nn->setPosition(c);
					nn->setScale( Ogre::Vector3(1,-1,-1) * MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE );
					break;
				case 4:
					nn->setPosition(d);
					nn->setScale( Ogre::Vector3(-1,1,-1) * MGE_DEBUG_INTERSECTS_VISUAL_MARKER_SCALE );
					break;
			}
			nn->attachObject( nn->getCreator()->createEntity("Axis.mesh") );
		}
	}
	#endif
	
	// 4. convert aabb2 lengths from node2 LOCAL space to WORLD space
	Ogre::Vector3 len( scale2 * (max - min) );
	
	// 5. scale aabb1 to WORLD space
	Ogre::AxisAlignedBox aabb(aabb1);
	aabb.scale(scale1);
	
	// 6. for each 12 rays from converted aabb2 corners check intersects with aabb1
	if (
		intersects(Ogre::Ray(a, pDirX) , aabb, len.x) || intersects(Ogre::Ray(a, pDirY) , aabb, len.y) || intersects(Ogre::Ray(a, pDirZ) , aabb, len.z) ||
		intersects(Ogre::Ray(b, nDirX) , aabb, len.x) || intersects(Ogre::Ray(b, nDirY) , aabb, len.y) || intersects(Ogre::Ray(b, pDirZ) , aabb, len.z) ||
		intersects(Ogre::Ray(c, pDirX) , aabb, len.x) || intersects(Ogre::Ray(c, nDirY) , aabb, len.y) || intersects(Ogre::Ray(c, nDirZ) , aabb, len.z) ||
		intersects(Ogre::Ray(d, nDirX) , aabb, len.x) || intersects(Ogre::Ray(d, pDirY) , aabb, len.y) || intersects(Ogre::Ray(d, nDirZ) , aabb, len.z)
	) {
		return true;
	}
	
	return false;
}

bool MGE::OgreColisionBoundingBox::intersects(
	const Ogre::AxisAlignedBox& aabb1, const Ogre::SceneNode* node1,
	const Ogre::Vector3& node1DerivedPosition, const Ogre::Quaternion& node1DerivedOrientation, const Ogre::Vector3& node1DerivedScale,
	const Ogre::AxisAlignedBox& aabb2, const Ogre::SceneNode* node2
) {
	// 1. check trival cases
	if (aabb1.isInfinite() || aabb2.isInfinite())
		return true;
	
	if (aabb1.isNull() && aabb2.isNull())
		return node1DerivedPosition.positionEquals( node2->_getDerivedPosition() );
	
	if (aabb1.isNull())
		return aabb2.intersects( node1DerivedPosition );
	
	if (aabb2.isNull())
		return aabb1.intersects( node2->_getDerivedPosition() );
	
	// 2. get transform info
	Ogre::Matrix4 fromNode1;
	fromNode1.makeTransform(node1DerivedPosition, Ogre::Vector3::UNIT_SCALE, node1DerivedOrientation);
	Ogre::Matrix4 fromNode2 = node2->_getFullTransform();
	
	// 3. check rays from aabb2 corners in node1 space
	if (_intersects(
		aabb1, aabb2, fromNode1, fromNode2, node1DerivedScale, node2->_getDerivedScale()
		#ifdef MGE_DEBUG_INTERSECTS_VISUAL
		, node1, node1DerivedPosition, node1DerivedOrientation, 0
		#endif
	)) {
		return true;
	}
	
	// 4. check rays from aabb1 corners in node2 space
	if (_intersects(
		aabb2, aabb1, fromNode2, fromNode1, node2->_getDerivedScale(), node1DerivedScale
		#ifdef MGE_DEBUG_INTERSECTS_VISUAL
		, node2, node2->_getDerivedPosition(), node2->_getDerivedOrientation(), 1
		#endif
	)) {
		return true;
	}
	
	// 5. check if aabb1 is in aabb2
	return intersects(
		Ogre::Ray(fromNode2.inverse() * fromNode1 * aabb1.getMinimum(), Ogre::Vector3::UNIT_X),
		aabb2
	);
}

bool MGE::OgreColisionBoundingBox::intersects(const Ogre::Ray& ray, const Ogre::AxisAlignedBox& box, Ogre::Real rayLen) {
	Ogre::Real t;
	Ogre::Vector3 hitpoint;
	const Ogre::Vector3& min = box.getMinimum();
	const Ogre::Vector3& max = box.getMaximum();
	const Ogre::Vector3& rayorig = ray.getOrigin();
	const Ogre::Vector3& raydir  = ray.getDirection();
	
	// Check origin inside first
	if ( rayorig > min && rayorig < max ) {
		return true;
	}
	
	// Check each face in turn, only check closest 3
	// Min x
	if (rayorig.x <= min.x && raydir.x > 0) {
		t = (min.x - rayorig.x) / raydir.x;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
				hitpoint.z >= min.z && hitpoint.z <= max.z) {
				return true;
			}
		}
	}
	// Max x
	if (rayorig.x >= max.x && raydir.x < 0) {
		t = (max.x - rayorig.x) / raydir.x;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
				hitpoint.z >= min.z && hitpoint.z <= max.z) {
				return true;
			}
		}
	}
	// Min y
	if (rayorig.y <= min.y && raydir.y > 0) {
		t = (min.y - rayorig.y) / raydir.y;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.z >= min.z && hitpoint.z <= max.z) {
				return true;
			}
		}
	}
	// Max y
	if (rayorig.y >= max.y && raydir.y < 0) {
		t = (max.y - rayorig.y) / raydir.y;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.z >= min.z && hitpoint.z <= max.z) {
				return true;
			}
		}
	}
	// Min z
	if (rayorig.z <= min.z && raydir.z > 0) {
		t = (min.z - rayorig.z) / raydir.z;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.y >= min.y && hitpoint.y <= max.y) {
				return true;
			}
		}
	}
	// Max z
	if (rayorig.z >= max.z && raydir.z < 0) {
		t = (max.z - rayorig.z) / raydir.z;
		if (t >= 0 && t < rayLen) {
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.y >= min.y && hitpoint.y <= max.y) {
				return true;
			}
		}
	}
	
	return false;
}
