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

Based on:
	→ public domain code from Ogre Wiki (http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Intermediate+Tutorials)
*/

#include "physics/utils/OgreRayCast.h"
#include "physics/utils/OgreColisionBoundingBox.h"

#ifdef USE_BULLET
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#endif

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

void MGE::OgreRayCast::ResultsBase::addResult(const btCollisionObject* object, const Ogre::Vector3& pointOgre, const Ogre::Ray& ray) {
	#ifdef USE_BULLET
	addResult( static_cast<Ogre::MovableObject*>(object->getUserPointer()), pointOgre, ray.getOrigin().squaredDistance(pointOgre) );
	#else
	LOG_WARNING("addResult with btCollisionObject when no USE_BULLET");
	#endif
}

void MGE::OgreRayCast::searchOnRay(MGE::OgreRayCast::ResultsBase* results, Ogre::SceneManager* scnMgr, const Ogre::Ray& ray, const Ogre::Vector3& rayTo, uint32_t searchMask, bool onlyFirst, bool vertical) {
	DEBUG2_LOG("search object at ray from " << ray.getOrigin() << " to " << rayTo << " with mask=" << searchMask);
	
	// search Ogre objects
	DEBUG2_LOG(" - search Ogre objects");
	Ogre::RaySceneQuery* rayScnQuery = scnMgr->createRayQuery(ray, searchMask);
	rayScnQuery->setSortByDistance(true);
	Ogre::RaySceneQueryResult& result = rayScnQuery->execute();
	
	for(auto& iter : result) {
		if (iter.movable) {
			results->addResult( iter.movable, ray.getPoint(iter.distance), iter.distance );
			DEBUG2_LOG("   - found " << iter.movable->getName() << " @ " << iter.distance);
			if (onlyFirst)
				return;
		}
	}
	scnMgr->destroyQuery(rayScnQuery);
	
	// search Orge terrain
	#if 0  /// @todo TODO.9: [Terrain] no terrain support in Ogre >= 2.1
	if (!results->hasGround && (searchMask & MGE::QueryFlags::GROUND) && MGE::Physics::Physics::getPtr()->getTerrain()) {
		DEBUG2_LOG(" - search Orge terrain");
		if (vertical) {
			Ogre::Terrain* tmp       = NULL;
			results->groundPoint     = ray.getOrigin();
			results->groundPoint.y   = MGE::Physics::Physics::getPtr()->getTerrain()->getHeightAtWorldPosition(ray.getOrigin(), &tmp) + MGE::Physics::Physics::getPtr()->getTerrain()->getOrigin().y;
			if (tmp != NULL) {
				results->hasGround   = true;
			}
		} else {
			Ogre::TerrainGroup::RayResult result_trn = MGE::Physics::Physics::getPtr()->getTerrain()->rayIntersects(ray);
			if (result_trn.hit && result_trn.position.y >= rayTo.y) {
				results->groundPoint = result_trn.position;
				results->hasGround   = true;
			}
		}
	}
	#endif
}

void MGE::OgreRayCast::searchOnArea(MGE::OgreRayCast::ResultsBase* results, Ogre::SceneManager* scnMgr, const std::vector<Ogre::Ray>& rays, unsigned int typeMask) {
	DEBUG2_LOG("search ogre object on area");
	
	Ogre::PlaneBoundedVolume vol;
	vol.planes.push_back(Ogre::Plane(rays[0].getPoint(3), rays[1].getPoint(3),   rays[2].getPoint(3)));
	vol.planes.push_back(Ogre::Plane(rays[0].getOrigin(), rays[0].getPoint(100), rays[1].getPoint(100)));
	vol.planes.push_back(Ogre::Plane(rays[0].getOrigin(), rays[3].getPoint(100), rays[0].getPoint(100)));
	vol.planes.push_back(Ogre::Plane(rays[3].getOrigin(), rays[2].getPoint(100), rays[3].getPoint(100)));
	vol.planes.push_back(Ogre::Plane(rays[1].getOrigin(), rays[1].getPoint(100), rays[2].getPoint(100)));
	Ogre::PlaneBoundedVolumeList volList;
	volList.push_back(vol);
	
	Ogre::PlaneBoundedVolumeListSceneQuery* volScnQuery = scnMgr->createPlaneBoundedVolumeQuery(volList, typeMask);
	results->addResult( volScnQuery->execute() );
	scnMgr->destroyQuery(volScnQuery);
}

void MGE::OgreRayCast::searchOnRadius(MGE::OgreRayCast::ResultsBase* results, Ogre::SceneManager* scnMgr, float radius, const Ogre::Vector3& point, unsigned int typeMask) {
	DEBUG2_LOG("search ogre object on radius " << radius << " from " << point);
	
	Ogre::Sphere sphere(point, radius);
	
	Ogre::SphereSceneQuery* volScnQuery = scnMgr->createSphereQuery(sphere, typeMask);
	results->addResult( volScnQuery->execute() );
	scnMgr->destroyQuery(volScnQuery);
}

std::pair<bool, Ogre::Vector3> MGE::OgreRayCast::findFreePosition(const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, int queryMask, Ogre::Real step, int count) {
	int i, x, z;
	Ogre::Vector3 basePosition( node->_getDerivedPosition() );
	
	if ( MGE::OgreColisionBoundingBox::isFreePosition( node, aabb, basePosition, queryMask ) )
		return std::make_pair(true, basePosition);
	
	for (i = 1; i < count; i++) {
		for (x = -i; x <= i; x++) {
			// z = i;
			Ogre::Vector3 position = basePosition + Ogre::Vector3(x * step, 0, i * step);
			if ( MGE::OgreColisionBoundingBox::isFreePosition( node, aabb, position, queryMask ) )
				return std::make_pair(true, position);
			
			// z = -i;
			position = basePosition + Ogre::Vector3(x * step, 0, -i * step);
			if ( MGE::OgreColisionBoundingBox::isFreePosition( node, aabb, position, queryMask ) )
				return std::make_pair(true, position);
		}
		for (z = -i + 1; z < i; z++) {
			// x = i;
			Ogre::Vector3 position = basePosition + Ogre::Vector3(i * step, 0, z * step);
			if ( MGE::OgreColisionBoundingBox::isFreePosition( node, aabb, position, queryMask ) )
				return std::make_pair(true, position);
			
			// x = -i;
			position = basePosition + Ogre::Vector3(-i * step, 0, z * step);
			if ( MGE::OgreColisionBoundingBox::isFreePosition( node, aabb, position, queryMask ) )
				return std::make_pair(true, position);
		}
	}
	
	return std::make_pair(false, basePosition);
}

