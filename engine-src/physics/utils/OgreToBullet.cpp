/*
Copyright (c) 2019-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "physics/utils/OgreToBullet.h"

#include <BtOgreExtras.h>
#include <BtOgrePG.h>

#include "LogSystem.h"

void MGE::OgreToBullet::updateAll() {
	for (auto& iter : nodes) {
		auto& phyObj = iter.first;
		auto& info   = iter.second;
		
		// check change on Ogre-side transform matrix
		const Ogre::Matrix4& newFullTransform = info.node->_getFullTransform();
		if (newFullTransform == info.transform)
			continue;
		info.transform = newFullTransform;
		
		
		// is scale change ?
		bool  force = false;
		const Ogre::Vector3& scale = info.node->_getDerivedScale();
		{
			btVector3 newScale  = BtOgre::Convert::toBullet(scale);
			btVector3 diffScale = phyObj->getCollisionShape()->getLocalScaling() - newScale;
			if ( diffScale.x() > EPSION1 || diffScale.x() < -EPSION1 || diffScale.y() > EPSION1 || diffScale.y() < -EPSION1 || diffScale.z() > EPSION1 || diffScale.z() < -EPSION1 ) {
				phyObj->getCollisionShape()->setLocalScaling(newScale);
				force = true;
			}
		}
		
		// get scaled offset as Bullet transform
		btTransform  transformOffset(
			btQuaternion::getIdentity(),
			BtOgre::Convert::toBullet( info.offset * scale )
		);
		
		// calculate full transform from Ogre state
		btTransform newTransform(
			BtOgre::Convert::toBullet( info.node->_getDerivedOrientation() ),
			BtOgre::Convert::toBullet( info.node->_getDerivedPosition() )
		);
		newTransform *= transformOffset;
		
		// for RigidBody use RigidBodyState
		BtOgre::RigidBodyState* state = NULL;
		if (phyObj->getInternalType() == btCollisionObject::CO_RIGID_BODY) {
			state = static_cast<BtOgre::RigidBodyState*>(static_cast<btRigidBody*>(phyObj)->getMotionState());
		}
		
		// get current state of physics
		btTransform currentTransform;
		if (state)
			state->getWorldTransform(currentTransform);
		else
			currentTransform = phyObj->getWorldTransform();
		
		// compare with calculated newTransform to cancel small changes
		btVector3 posDiff = currentTransform.getOrigin() - newTransform.getOrigin();
		btScalar  rotDiff = currentTransform.getRotation().dot(newTransform.getRotation());
		
		if (
			(!force) &&
			posDiff.x() < EPSION1 && posDiff.x() > -EPSION1 && posDiff.y() < EPSION1 && posDiff.y() > -EPSION1 && posDiff.z() < EPSION1 && posDiff.z() > -EPSION1 &&
			(rotDiff > EPSION2 || rotDiff < -EPSION2)
		) {
			return;
		}
		
		//LOG_DEBUG("nodeUpdated: " << info.node->getName() << "  " << posDiff.x() << ", " << posDiff.y() << ", " << posDiff.z() << "   " << rotDiff);
		
		// update only when need
		if (state) {
			state->setWorldTransformNoUpdate(newTransform);
		}
		phyObj->setWorldTransform(newTransform);
		//phyObj->activate();
	}
}
