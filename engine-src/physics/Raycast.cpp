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

#include "physics/Raycast.h"
#include "physics/Physics.h"

#include "data/structs/BaseActor.h"

#include <OgreItem.h>
#include <OgreEntity.h>

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

int MGE::RayCast::defaultIterationLimit = 10;

void MGE::RayCast::Results::addObject(
	const Ogre::MovableObject* _ogreObject, Ogre::Vector3 _hitPoint, Ogre::Real _distance2
) {
	MGE::BaseActor* _gameObject = MGE::BaseActor::get(_ogreObject);
	
	// only add hits to Items and Entities
	if (_ogreObject->getMovableType() != Ogre::ItemFactory::FACTORY_TYPE_NAME && _ogreObject->getMovableType() != Ogre::v1::EntityFactory::FACTORY_TYPE_NAME)
		return;
	
	// prevent add duplicate object and ensure to hitObjects is sorted by distance from ray start
	std::list<RayCast::ResultsEntry>::iterator hitObjectsIter;
	for (hitObjectsIter = hitObjects.begin(); hitObjectsIter != hitObjects.end(); ++hitObjectsIter) {
		if ((_gameObject != NULL && hitObjectsIter->gameObject == _gameObject) || hitObjectsIter->ogreObject == _ogreObject) {
			return;
		}
		if (hitObjectsIter->distance2 > _distance2)
			break;
	}
	
	// add results to list
	#ifdef MGE_DEBUG2_LOG_STREAM
	if (_gameObject)
		DEBUG2_LOG_STREAM("   - found (game)\"" << _gameObject->getName() << "\" at " << _hitPoint );
	else
		DEBUG2_LOG_STREAM("   - found (ogre)\"" << _ogreObject->getName() << "\" at " << _hitPoint );
	#endif
	hitObjects.insert(hitObjectsIter, RayCast::ResultsEntry( _gameObject, _ogreObject, _hitPoint, _distance2 ) );
	
	// check ground
	if (_ogreObject->getQueryFlags() & MGE::QueryFlags::GROUND) {
		DEBUG2_LOG("     - it's GROUND");
		hasGround   = true;
		groundPoint = _hitPoint;
	}
}

void MGE::RayCast::Results::addResult(const btCollisionObject* object, const Ogre::Vector3& pointOgre, const Ogre::Ray& ray) {
	#ifdef USE_BULLET
	addObject(
		static_cast<Ogre::MovableObject*>(object->getUserPointer()),
		pointOgre, ray.getOrigin().squaredDistance(pointOgre)
	);
	#else
	LOG_WARNING("addResult with btCollisionObject when no USE_BULLET");
	#endif
}

void MGE::RayCast::Results::addResult(const Ogre::MovableObject* object, const Ogre::Vector3& pointOgre, Ogre::Real distance) {
	addObject( object, pointOgre, distance*distance );
}

void MGE::RayCast::Results::addResult(Ogre::SceneQueryResult& queryResult) {
	for(auto& iter : queryResult.movables) {
		hitObjects.push_back( RayCast::ResultsEntry( MGE::BaseActor::get(iter), iter, Ogre::Vector3::ZERO, -1 ) );
	}
}


void MGE::RayCast::searchOnBulletRay(MGE::RayCast::ResultsBase* results, const Ogre::Ray& ray, const Ogre::Vector3& rayTo, uint32_t searchMask, bool onlyFirst) {
#ifdef USE_BULLET
	if (MGE::Physics::Physics::getPtr()->getDynamicsWorld()) {
		DEBUG2_LOG(" - search Bullet objects");
		btVector3 start = BtOgre::Convert::toBullet( ray.getOrigin() );
		btVector3 end   = BtOgre::Convert::toBullet( rayTo );
		if (onlyFirst) {
			btCollisionWorld::ClosestRayResultCallback bulletRay(start, end);
			bulletRay.m_collisionFilterMask = searchMask;
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->rayTest(start, end, bulletRay);
			
			if (bulletRay.hasHit()) {
				Ogre::Vector3 pointOgre = BtOgre::Convert::toOgre(bulletRay.m_hitPointWorld);
				results->addResult(
					bulletRay.m_collisionObject,
					pointOgre,
					ray
				);
				return;
			}
		} else {
			btCollisionWorld::AllHitsRayResultCallback bulletRay(start, end);
			bulletRay.m_collisionFilterMask = searchMask;
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->rayTest(start, end, bulletRay);
			
			if (bulletRay.hasHit()) {
				int i, num = bulletRay.m_collisionObjects.size();
				for (i=0; i<num; ++i) {
					Ogre::Vector3 pointOgre = BtOgre::Convert::toOgre(bulletRay.m_hitPointWorld[i]);
					results->addResult(
						bulletRay.m_collisionObjects[i],
						pointOgre,
						ray
					);
				}
			}
		}
	}
#endif
}

bool MGE::RayCast::isFreeBulletPosition(const Ogre::Vector3& position, btCollisionObject* object) {
	bool ret = true;
	
	#ifdef USE_BULLET
	struct ContactTestResultCallback : public btCollisionWorld::ContactResultCallback {
		bool hasCollision;
		btCollisionObject* testObject;
		
		ContactTestResultCallback(btCollisionObject* obj) {
			hasCollision = false;
			testObject = obj;
		}
		
		virtual btScalar addSingleResult(
			btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0,int partId0,int index0,
			const btCollisionObjectWrapper* colObj1,int partId1,int index1
		) {
			if ((!colObj0->m_collisionObject->getCollisionFlags()) & MGE::QueryFlags::GROUND & (!colObj1->m_collisionObject->getCollisionFlags()) & MGE::QueryFlags::GROUND)
				hasCollision = true;
			return 0;
		}
	};
	
	if (object) {
		ContactTestResultCallback bulletTest(object);
		if (MGE::Physics::Physics::getPtr()->getDynamicsWorld())
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->contactTest(object, bulletTest);
		ret = bulletTest.hasCollision;
	}
	#endif
	
	return ret;
}
