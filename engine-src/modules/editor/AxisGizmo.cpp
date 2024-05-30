/*
Copyright (c) 2008-2013 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team
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

Based on:
	→ MIT licensed code from OGITOR
		- ex http://www.ogitor.org
		- now https://github.com/OGRECave/ogitor
*/

#include "modules/editor/AxisGizmo.h"

#include "LogSystem.h"
#include "Engine.h"

#include "rendering/CameraSystem.h"
#include "rendering/utils/RenderQueueGroups.h"
#include "rendering/markers/Shapes.h"
#include "physics/utils/OgreMeshRaycast.h"
#include "data/QueryFlags.h"
#include "data/utils/OgreUtils.h"

#include <OgreManualObject2.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager2.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreViewport.h>

#define EditorResourcesGroup "General"

constexpr char MGE::AxisGizmo::AxisArray[3];

Ogre::Vector3 MGE::AxisGizmo::Callback::getMove(
	Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Ray& ray, Ogre::Vector3& offset, bool& offsetIsValid
) {
	Ogre::Plane gizmoPlane;
	switch (axis) {
		case AXIS_XZ:
		case AXIS_X:
		case AXIS_Z:
			gizmoPlane = Ogre::Plane( Ogre::Vector3(0,1,0), Ogre::Vector3::ZERO );
			break;
		case AXIS_XY:
		case AXIS_Y:
			gizmoPlane = Ogre::Plane( Ogre::Vector3(0,0,1), Ogre::Vector3::ZERO );
			break;
		case AXIS_YZ:
			gizmoPlane = Ogre::Plane( Ogre::Vector3(1,0,0), Ogre::Vector3::ZERO );
			break;
	}
	
	// convert ray from TS_WORLD to gizmo space (transformSpace)
	// get node position in gizmo space (transformSpace)
	Ogre::Vector3 nodePosition;
	Ogre::Ray     convertedRay;
	switch (transformSpace) {
		case Ogre::Node::TS_LOCAL:
			convertedRay.setOrigin( node->_getFullTransform().inverse() * ray.getOrigin() );
			convertedRay.setDirection( node->_getDerivedOrientation().Inverse() * ray.getDirection() );
			nodePosition = Ogre::Vector3::ZERO;
			break;
		case Ogre::Node::TS_PARENT:
			convertedRay.setOrigin( node->getParent()->_getFullTransform().inverse() * ray.getOrigin() );
			convertedRay.setDirection( node->getParent()->_getDerivedOrientation().Inverse() * ray.getDirection() );
			nodePosition = node->getPosition();
			break;
		case Ogre::Node::TS_WORLD:
			convertedRay = ray;
			nodePosition = node->_getDerivedPosition();
			break;
	}
	
	std::pair<bool, Ogre::Real> res = convertedRay.intersects(gizmoPlane);
	if (res.first) {
		Ogre::Vector3  moveVector = (convertedRay.getPoint(res.second) - nodePosition);
		if (!offsetIsValid) {
			offset = moveVector;
			offsetIsValid = true;
			return Ogre::Vector3::ZERO;
		}
		
		Ogre::Vector3 axisVector(
			(axis & AXIS_X) ? 1.0 : 0.0,
			(axis & AXIS_Y) ? 1.0 : 0.0,
			(axis & AXIS_Z) ? 1.0 : 0.0
		);
		
		moveVector = (moveVector - offset) * axisVector;
		
		// convert moveVector from transformSpace to TS_PARENT
		switch(transformSpace) {
			case Ogre::Node::TS_LOCAL:
				moveVector = node->getOrientation() * moveVector * node->getScale();
				break;
			case Ogre::Node::TS_PARENT:
				break;
			case Ogre::Node::TS_WORLD:
				moveVector = node->getParent()->_getDerivedOrientation().Inverse() * moveVector /  node->getParent()->_getDerivedScale();
				break;
		}
		
		return moveVector;
	}
	
	return Ogre::Vector3::ZERO;
}

Ogre::Vector3 MGE::AxisGizmo::Callback::getScale(
	Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Vector3& oldScale, const Ogre::Real& scaleFactor
) {
	Ogre::Vector3 axisVector(
		(axis & AXIS_X) ? 1.0 : 0.0,
		(axis & AXIS_Y) ? 1.0 : 0.0,
		(axis & AXIS_Z) ? 1.0 : 0.0
	);
	
	// convert axisVector from gizmo space (transformSpace) to TS_LOCAL
	switch (transformSpace) {
		case Ogre::Node::TS_LOCAL:
			break;
		case Ogre::Node::TS_PARENT:
			axisVector = node->getOrientation() * axisVector;
			break;
		case Ogre::Node::TS_WORLD:
			axisVector = node->_getDerivedOrientation() * axisVector;
			break;
	}
	
	// set scale vector base on axisVector and scale factor (mouse distance)
	Ogre::Vector3 scaleVector(
		Ogre::Math::Abs(axisVector.x), Ogre::Math::Abs(axisVector.y), Ogre::Math::Abs(axisVector.z)
	); 
	
	return (scaleVector * oldScale * scaleFactor);
}

Ogre::Vector3 MGE::AxisGizmo::Callback::getScale(
	Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint,
	const Ogre::Vector3& oldScale, float negScaleFactor, float posScaleFactor
) {
	Ogre::Real scaleFactor = mouseClickPoint.y - mouseCurrentPoint.y;
	scaleFactor *= ((scaleFactor > 0) ? posScaleFactor : negScaleFactor);
	
	return oldScale + getScale(transformSpace, axis, node, oldScale, scaleFactor);
}

Ogre::Quaternion MGE::AxisGizmo::Callback::getOrientation(
	Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Quaternion& oldOrientation, Ogre::Radian rotateAngle
) {
	Ogre::Vector3 axisVector(
		(axis & AXIS_X) ? 1.0 : 0.0,
		(axis & AXIS_Y) ? 1.0 : 0.0,
		(axis & AXIS_Z) ? 1.0 : 0.0
	);
	
	Ogre::Quaternion rotate;
	rotate.FromAngleAxis(rotateAngle, axisVector);
	
	switch(transformSpace) {
		case Ogre::Node::TS_LOCAL:
			return oldOrientation * rotate;
		case Ogre::Node::TS_PARENT:
			return rotate * oldOrientation;
		case Ogre::Node::TS_WORLD: {
			Ogre::Quaternion transform = node->getParent()->_getDerivedOrientation() * oldOrientation;
			return oldOrientation * transform.Inverse() * rotate * transform;
		}
	}
}

Ogre::Quaternion MGE::AxisGizmo::Callback::getOrientation(
	Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint,
	const Ogre::Quaternion& oldOrientation, float rotateFactor
) {
	Ogre::Radian rotateAngle(
		(mouseClickPoint.y - mouseCurrentPoint.y) * rotateFactor
	);
	
	return getOrientation(transformSpace, axis, node, oldOrientation, rotateAngle);
}

Ogre::Vector3 MGE::AxisGizmo::Callback::calculateRotatedPosition(
	const Ogre::SceneNode* target_node, Ogre::Vector3 pivot_position,
	const Ogre::Vector3& init_position, const Ogre::Quaternion& init_orientation, const Ogre::Quaternion& new_orientation
) {
	// convert pivot_node position to target_node PARENT space
	pivot_position = target_node->getParent()->_getDerivedOrientation().Inverse() * (pivot_position - target_node->getParent()->_getDerivedPosition());
	pivot_position = pivot_position / target_node->getParent()->_getDerivedScale();
	
	// calculate offset from Pivot
	Ogre::Vector3 offset = init_position - pivot_position;
	
	// rotate offset
	Ogre::Quaternion rotation = new_orientation * init_orientation.Inverse();
	offset = rotation * offset;
	
	// we return new position for target_node
	return pivot_position + offset;
}

void MGE::AxisGizmo::VisualCallback::gizmoCallback(
	int gizmoMode, Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint, const OIS::MouseEvent& mouseArg,
	bool  endOfOperation
) {
	if (lastNode != node) {
		lastNode        = node;
		initScale       = lastNode->getScale();
		initOrientation = lastNode->getOrientation();
		inMoveMode      = false;
	}
	
	switch(gizmoMode) {
		case MGE::AxisGizmo::MOVE: {
			Ogre::Vector3 move = getMove(
				transformSpace, axis, lastNode,
				MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(mouseCurrentPoint.x, mouseCurrentPoint.y),
				zeroOffset, inMoveMode
			);
			lastNode->setPosition(lastNode->getPosition() + move);
			break;
		}
		case MGE::AxisGizmo::SCALE:
			lastNode->setScale(
				getScale(transformSpace, axis, lastNode, mouseClickPoint, mouseCurrentPoint, initScale)
			);
			break;
		case MGE::AxisGizmo::ROTATE: {
			lastNode->setOrientation(
				getOrientation(transformSpace, axis, lastNode, mouseClickPoint, mouseCurrentPoint, initOrientation)
			);
			break;
		}
	}
	
	if (endOfOperation) {
		lastNode = NULL;
	}
}


MGE::AxisGizmo::AxisGizmo(Ogre::SceneManager* scnMgr, float sizeFactor) :
	mOwnerNode(NULL), mGizmoSizeFactor(sizeFactor)
{
	CreateGizmo(scnMgr);
}

MGE::AxisGizmo::~AxisGizmo() {
	LOG_INFO("destroy AxisGizmo");
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	MGE::InputSystem::getPtr()->unregisterListener(this);
	
	DestroyGizmo();
}

void MGE::AxisGizmo::setMode(int mode, Ogre::Node::TransformSpace transformSpace, Callback* callback) {
	mGizmoMode = mode;
	mTransformSpace = transformSpace;
	mGizmoCallback = callback;
	
	if (mOwnerNode) {
		Ogre::SceneNode* node = mOwnerNode;      // use this because hide() set mOwnerNode to NULL
		mLastGizmoCamDist = Ogre::Vector3::ZERO; // force update
		hide();
		show(node);
	}
}

void MGE::AxisGizmo::show(Ogre::SceneNode* node) {
	mOwnerNode = node;
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER+3);
	MGE::InputSystem::getPtr()->registerListener(this, 100, 100, 100, -1, -1, -1);
	
	for (int i=0; i<13; ++i) {
		mGizmoEntities[i]->setVisible(false);
	}
	
	switch (mGizmoMode) {
		case MOVE:
			// arrows
			mGizmoEntities[0]->setVisible(true);
			mGizmoEntities[1]->setVisible(true);
			mGizmoEntities[2]->setVisible(true);
			// planes
			mGizmoEntities[9]->setVisible(true);
			mGizmoEntities[10]->setVisible(true);
			mGizmoEntities[11]->setVisible(true);
			break;
		case ROTATE:
			// arrows
			mGizmoEntities[6]->setVisible(true);
			mGizmoEntities[7]->setVisible(true);
			mGizmoEntities[8]->setVisible(true);
			break;
		case SCALE:
			// arrows
			mGizmoEntities[3]->setVisible(true);
			mGizmoEntities[4]->setVisible(true);
			mGizmoEntities[5]->setVisible(true);
			// planes
			mGizmoEntities[9]->setVisible(true);
			mGizmoEntities[10]->setVisible(true);
			mGizmoEntities[11]->setVisible(true);
			// ball
			mGizmoEntities[12]->setVisible(true);
			break;
	}
	
	update(0, 0);
}

void MGE::AxisGizmo::show(bool visible, Ogre::SceneNode* node) {
	if (visible)
		show(node);
	else
		hide();
}

void MGE::AxisGizmo::hide() {
	mOwnerNode = NULL;
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	MGE::InputSystem::getPtr()->unregisterListener(this);
	
	for (int i=0; i<13; ++i) {
		mGizmoEntities[i]->setVisible(false);
	}
}

bool MGE::AxisGizmo::mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (arg.state.buttons == 0) {
		int newAxis = 0;
		bool ret = PickGizmos(MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(mousePos.x, mousePos.y), newAxis);
		if (newAxis != mCurrentAxis) {
			mCurrentAxis = newAxis;
			HighlightGizmo(mCurrentAxis);
		}
		return ret;
	} else if (mCurrentAxis && arg.state.buttons & (1 << OIS::MB_Left)) {
		mGizmoCallback->gizmoCallback(mGizmoMode, mTransformSpace, mCurrentAxis, mOwnerNode, mMouseClickPoint, mousePos, arg, false);
		return true;
	}
	return false;
}

bool MGE::AxisGizmo::mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* /*fromWindow*/ ) {
	if (mCurrentAxis && buttonID == OIS::MB_Left) {
		mMouseClickPoint = mousePos;
		return true;
	}
	return false;
}
bool MGE::AxisGizmo::mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (mCurrentAxis) {
		if (buttonID == OIS::MB_Left) {
			mGizmoCallback->gizmoCallback(mGizmoMode, mTransformSpace, mCurrentAxis, mOwnerNode, mMouseClickPoint, mousePos, arg, true);
		}
		mCurrentAxis = 0;
		HighlightGizmo(mCurrentAxis);
		return true;
	}
	return false;
}

bool MGE::AxisGizmo::update(float gameTimeStep, float realTimeStep) {
	if (!mOwnerNode)
		return false;
	
	MGE::OgreUtils::updateCachedTransform(mOwnerNode, false, false, true);

	Ogre::Vector3 position  = mOwnerNode->_getDerivedPosition();
	Ogre::Vector3 cameraPos = MGE::CameraSystem::getPtr()->getCurrentCamera()->getCamera()->getDerivedPosition();
	Ogre::Vector3 dist      = position - cameraPos;
	
	if (dist == mLastGizmoCamDist)
		return true;
	
	mLastGizmoCamDist = dist;
	
	// set position, orientation and scale
	mGizmoNode->setPosition(position);
	
	switch(mTransformSpace) {
		case Ogre::Node::TS_LOCAL:
			mGizmoNode->setOrientation( mOwnerNode->_getDerivedOrientation() );
			break;
		case Ogre::Node::TS_PARENT:
			mGizmoNode->setOrientation( mOwnerNode->getParent()->_getDerivedOrientation() );
			break;
		case Ogre::Node::TS_WORLD:
			mGizmoNode->setOrientation( Ogre::Quaternion::IDENTITY);
	}
	
	float minsize = std::min(
		MGE::CameraSystem::getPtr()->getCurrentCamera()->getRenderTarget()->getHeight(),
		MGE::CameraSystem::getPtr()->getCurrentCamera()->getRenderTarget()->getWidth()
	);
	float distance = dist.length() / minsize * mGizmoSizeFactor;
	mGizmoNode->setScale(distance, distance, distance);
	
	// set RenderQueueGroup
	float dist0 = (cameraPos - mGizmoEntities[9]->getWorldAabbUpdated().mCenter).length();
	float dist1 = (cameraPos - mGizmoEntities[10]->getWorldAabbUpdated().mCenter).length();
	float dist2 = (cameraPos - mGizmoEntities[11]->getWorldAabbUpdated().mCenter).length();
	int orders[3];
	int increment;
	
	if (mGizmoMode == ROTATE) {
		increment = 1;
		orders[0] = orders[1] = orders[2] = 0;
	} else {
		increment = -1;
		orders[0] = orders[1] = orders[2] = 3;
	}
	
	if (dist0 > dist1) 
		orders[0] += increment;
	else
		orders[1] += increment;
	
	if (dist0 > dist2) 
		orders[0] += increment;
	else
		orders[2] += increment;
	
	if (dist1 > dist2) 
		orders[1] += increment;
	else
		orders[2] += increment;
	
	// LOG_DEBUG("AAA d0=" << dist0 << " d1=" << dist1 << " d2=" << dist2 << "  =>  o0=" << orders[0] << " o1=" << orders[1] << " o2=" << orders[2] << "  @ cam=" << cameraPos << " giz=" << position);
	
	for (int i=0; i<9; i=i+3) {
		for (int j=0; j<3; ++j) {
			for (size_t k = 0; k < mGizmoEntities[i+j]->getNumSubItems(); ++k)
				mGizmoEntities[i+j]->getSubItem(k)->setRenderQueueSubGroup(orders[j]);
		}
	}
	
	// Rescale the Axis Gizmo so that it is always facing the ogitor camera regardless of view direction.
	if (mGizmoMode == ROTATE) {
		if (dist.x > 0 && dist.z > 0) {
			if (dist.y < 0) {
				mGizmoX->setScale(-1, 1,-1);
				mGizmoZ->setScale( 1,-1, 1);
				mGizmoY->setScale( 1,-1,-1);
			} else {
				mGizmoX->setScale(-1,-1,-1);
				mGizmoZ->setScale( 1,-1,-1);
				mGizmoY->setScale( 1,-1,-1);
			}
		} else if (dist.x > 0 && dist.z < 0) {
			if (dist.y < 0) {
				mGizmoX->setScale(-1, 1, 1);
				mGizmoZ->setScale( 1,-1, 1);
				mGizmoY->setScale( 1, 1,-1);
			} else {
				mGizmoX->setScale( 1,-1, 1);
				mGizmoZ->setScale( 1,-1,-1);
				mGizmoY->setScale( 1, 1,-1);
			}
		} else if (dist.x < 0 && dist.z > 0) {
			if (dist.y < 0) {
				mGizmoX->setScale(-1, 1,-1);
				mGizmoZ->setScale(-1, 1, 1);
				mGizmoY->setScale( 1,-1, 1);
			} else {
				mGizmoX->setScale(-1,-1,-1);
				mGizmoZ->setScale(-1, 1,-1);
				mGizmoY->setScale( 1,-1, 1);
			}
		} else {
			if (dist.y < 0) {
				mGizmoX->setScale( 1, 1, 1);
				mGizmoZ->setScale( 1, 1, 1);
				mGizmoY->setScale( 1, 1, 1);
			} else {
				mGizmoX->setScale(-1,-1, 1);
				mGizmoZ->setScale( 1, 1,-1);
				mGizmoY->setScale( 1, 1, 1);
			}
		}
	} else {
		mGizmoX->setScale( 1, 1, 1);
		mGizmoZ->setScale( 1, 1, 1);
		mGizmoY->setScale( 1, 1, 1);
	} /* {
		if (dist.x > 0 && dist.z > 0) {
			if (dist.y < 0) {
				mGizmoX->setScale(-1, 1,-1);
				mGizmoZ->setScale(-1,-1,-1);
				mGizmoY->setScale( 1,-1,-1);
			} else {
				mGizmoX->setScale(-1,-1,-1);
				mGizmoZ->setScale(-1,-1,-1);
				mGizmoY->setScale(-1,-1,-1);
			}
		} else if (dist.x > 0 && dist.z < 0) {
			if (dist.y < 0) {
				mGizmoX->setScale(-1, 1,-1);
				mGizmoZ->setScale( 1,-1, 1);
				mGizmoY->setScale( 1, 1, 1);
			} else {
				mGizmoX->setScale(-1,-1,-1);
				mGizmoZ->setScale( 1,-1, 1);
				mGizmoY->setScale(-1, 1, 1);
			}
		} else if (dist.x < 0 && dist.z > 0) {
			if (dist.y < 0) {
				mGizmoX->setScale( 1, 1, 1);
				mGizmoZ->setScale(-1, 1, 1);
				mGizmoY->setScale( 1,-1, 1);
			} else {
				mGizmoX->setScale( 1,-1, 1);
				mGizmoZ->setScale(-1, 1, 1);
				mGizmoY->setScale(-1,-1,-1);
			}
		} else {
			if (dist.y < 0) {
				mGizmoX->setScale( 1, 1, 1);
				mGizmoZ->setScale( 1, 1, 1);
				mGizmoY->setScale( 1, 1, 1);
			} else {
				mGizmoX->setScale( 1,-1, 1);
				mGizmoZ->setScale( 1, 1, 1);
				mGizmoY->setScale(-1, 1,-1);
			}
		}
	}*/
	return true;
}

bool MGE::AxisGizmo::PickGizmos(const Ogre::Ray& ray, int& Axis) {
	Ogre::Real closest_distance = -1.0f;
	
	for(int widx = 0;widx < 13;widx++) {
		// get the entity to check
		Ogre::Item* pentity = mGizmoEntities[widx];
		
		if (!(pentity->isVisible()))
			continue;
		
		Ogre::Aabb aabb = pentity->getWorldAabb();
		std::pair<bool, Ogre::Real> hit = ray.intersects(Ogre::AxisAlignedBox(aabb.getMinimum(), aabb.getMaximum()));
		if (!hit.first)
			continue;
		
		Ogre::Matrix4 toWorldFull = pentity->getParentSceneNode()->_getFullTransform();
		MGE::OgreMeshRaycast::Results res = MGE::OgreMeshRaycast::entityHitTest(
			ray, toWorldFull, pentity, vertices[widx/3], indices[widx/3], true, true
		);
		
		if (res.index >= 0 && (closest_distance > res.distance || closest_distance < 0)) {
			closest_distance = res.distance;
			switch(widx) {
				case 0:
				case 3:
				case 6:
					Axis = AXIS_X;
					break;
				case 1:
				case 4:
				case 7:
					Axis = AXIS_Y;
					break;
				case 2:
				case 5:
				case 8:
					Axis = AXIS_Z;
					break;
				case 9:
					Axis = AXIS_XY;
					break;
				case 10:
					Axis = AXIS_YZ;
					break;
				case 11:
					Axis = AXIS_XZ;
					break;
				case 12:
					Axis = AXIS_ALL;
					break;
			}
		}
	}
	
	return (closest_distance >= 0.0f);
}

void MGE::AxisGizmo::HighlightGizmo(int ID) {
	LOG_DEBUG("HighlightGizmo " << ID);
	for (int i=0; i<9; i=i+3) {
		if(ID & AXIS_X)
			mGizmoEntities[i]->setDatablock("MAT_GIZMO_X_L");
		else
			mGizmoEntities[i]->setDatablock("MAT_GIZMO_X");
		
		if(ID & AXIS_Y)
			mGizmoEntities[i+1]->setDatablock("MAT_GIZMO_Y_L");
		else
			mGizmoEntities[i+1]->setDatablock("MAT_GIZMO_Y");
		
		if(ID & AXIS_Z)
			mGizmoEntities[i+2]->setDatablock("MAT_GIZMO_Z_L");
		else
			mGizmoEntities[i+2]->setDatablock("MAT_GIZMO_Z");
	}
	
	if((ID & AXIS_XY) == AXIS_XY)
		mGizmoEntities[9]->setDatablock("MAT_GIZMO_XY_L");
	else
		mGizmoEntities[9]->setDatablock("MAT_GIZMO_XY");
	
	if((ID & AXIS_YZ) == AXIS_YZ)
		mGizmoEntities[10]->setDatablock("MAT_GIZMO_YZ_L");
	else
		mGizmoEntities[10]->setDatablock("MAT_GIZMO_YZ");
	
	if((ID & AXIS_XZ) == AXIS_XZ)
		mGizmoEntities[11]->setDatablock("MAT_GIZMO_ZX_L");
	else
		mGizmoEntities[11]->setDatablock("MAT_GIZMO_ZX");
	
	if((ID & AXIS_ALL) == AXIS_ALL)
		mGizmoEntities[12]->setDatablock("MAT_GIZMO_ALL_L");
	else
		mGizmoEntities[12]->setDatablock("MAT_GIZMO_ALL");
}

void MGE::AxisGizmo::CreateGizmo(Ogre::SceneManager* scnMgr) {
	Ogre::MeshPtr moveMesh   = createMoveArrowMesh(scnMgr,   "AxisGizmo_MoveArrowMesh");
	Ogre::MeshPtr scaleMesh  = createScaleArrowMesh(scnMgr,  "AxisGizmo_ScaleArrowMesh");
	Ogre::MeshPtr rotateMesh = createRotateArrowMesh(scnMgr, "AxisGizmo_RotateArrowMesh");
	Ogre::MeshPtr planeMesh  = MGE::Shapes::createPlaneMesh(scnMgr, "AxisGizmo_PlaneMesh", EditorResourcesGroup, "MAT_GIZMO_ALL");
	
	mGizmoNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	mGizmoX = mGizmoNode->createChildSceneNode();
	mGizmoY = mGizmoNode->createChildSceneNode();
	mGizmoZ = mGizmoNode->createChildSceneNode();
	mGizmoA = mGizmoNode->createChildSceneNode();
	
	Ogre::Quaternion q1;
	Ogre::Quaternion q2;
	
	q1.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0,0,1));
	q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(1,0,0));
	mGizmoY->setOrientation(q1 * q2);
	
	q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0,1,0));
	q2.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(1,0,0));
	mGizmoZ->setOrientation(q1 * q2);
	
	//Entities
	mGizmoEntities[0]  = scnMgr->createItem(moveMesh);
	mGizmoEntities[1]  = scnMgr->createItem(moveMesh);
	mGizmoEntities[2]  = scnMgr->createItem(moveMesh);
	
	mGizmoEntities[3]  = scnMgr->createItem(scaleMesh);
	mGizmoEntities[4]  = scnMgr->createItem(scaleMesh);
	mGizmoEntities[5]  = scnMgr->createItem(scaleMesh);
	
	mGizmoEntities[6]  = scnMgr->createItem(rotateMesh);
	mGizmoEntities[7]  = scnMgr->createItem(rotateMesh);
	mGizmoEntities[8]  = scnMgr->createItem(rotateMesh);
	
	mGizmoEntities[9]  = scnMgr->createItem(planeMesh);
	mGizmoEntities[10] = scnMgr->createItem(planeMesh);
	mGizmoEntities[11] = scnMgr->createItem(planeMesh);
	
	mGizmoEntities[12] = scnMgr->createItem(MGE::Shapes::createSphereMesh(scnMgr, "AxisGizmo_SphereMesh", EditorResourcesGroup, "MAT_GIZMO_ALL", 50.0, 2*accuracy, 2*accuracy));
	
	for (int i=0; i<13; ++i) {
		mGizmoEntities[i]->setCastShadows(false);
		mGizmoEntities[i]->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
		mGizmoEntities[i]->setQueryFlags(MGE::QueryFlags::INTERACTIVE_WIMGET);
	}
	
	for (int i=0; i<12; i=i+3) {
		mGizmoX->attachObject(mGizmoEntities[i]);
		mGizmoY->attachObject(mGizmoEntities[i+1]);
		mGizmoZ->attachObject(mGizmoEntities[i+2]);
	}
	mGizmoA->setScale(0.01, 0.01, 0.01);
	mGizmoA->attachObject(mGizmoEntities[12]);
	
	//Entities info for MeshRaycast
	MGE::OgreMeshRaycast::getMeshInformation(mGizmoEntities[0],  vertices,   indices);
	MGE::OgreMeshRaycast::getMeshInformation(mGizmoEntities[3],  vertices+1, indices+1);
	MGE::OgreMeshRaycast::getMeshInformation(mGizmoEntities[6],  vertices+2, indices+2);
	MGE::OgreMeshRaycast::getMeshInformation(mGizmoEntities[9],  vertices+3, indices+3);
	MGE::OgreMeshRaycast::getMeshInformation(mGizmoEntities[12], vertices+4, indices+4);
	
	HighlightGizmo(0);
	
	for (int i=0; i<13; ++i) {
		mGizmoEntities[i]->setVisible(false);
	}
}

void MGE::AxisGizmo::DestroyGizmo() {
	if(!mGizmoNode) 
		return;
	
	Ogre::MeshManager::getSingleton().remove("AxisGizmo_MoveArrowMesh");
	Ogre::MeshManager::getSingleton().remove("AxisGizmo_ScaleArrowMesh");
	Ogre::MeshManager::getSingleton().remove("AxisGizmo_RotateArrowMesh");
	Ogre::MeshManager::getSingleton().remove("AxisGizmo_PlaneMesh");
	Ogre::MeshManager::getSingleton().remove("AxisGizmo_SphereMeshs");
	
	for(int i= 0;i < 6;i++) {
		mGizmoEntities[i]->detachFromParent();
		mGizmoEntities[i]->_getManager()->destroyItem(mGizmoEntities[i]);
	}
	
	mGizmoNode->removeAndDestroyAllChildren();
	mGizmoNode->getParentSceneNode()->removeChild(mGizmoNode);
	mGizmoNode->getCreator()->destroySceneNode(mGizmoNode);
	
	mGizmoNode = 0;
	mGizmoX = mGizmoY = mGizmoZ = 0;
	mGizmoEntities[0] = mGizmoEntities[1] = mGizmoEntities[2] = mGizmoEntities[3] = mGizmoEntities[4] = mGizmoEntities[5] = 0;
}


Ogre::MeshPtr MGE::AxisGizmo::createMoveArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name) {
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_LINE_LIST);
	manualObj->position(0, 0, 0);
	manualObj->position(3, 0, 0);
	
	manualObj->index(0);
	manualObj->index(1);
	manualObj->end();
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_TRIANGLE_LIST);
	
	manualObj->position(2.85f, 0, 0);
	for(float theta = 0; theta < 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy) {
		manualObj->position(2.95f, radius * cos(theta), radius * sin(theta));
	}
	manualObj->position(3.45f, 0, 0);
	
	for(int inside = 1;inside < 16;inside++)
	{
		manualObj->index(0);
		manualObj->index(inside);
		manualObj->index(inside + 1);
	}
	manualObj->index(0);
	manualObj->index(16);
	manualObj->index(1);
	
	for(int outside = 1;outside < 16;outside++) {
		manualObj->index(17);
		manualObj->index(outside);
		manualObj->index(outside + 1);
	}
	manualObj->index(17);
	manualObj->index(16);
	manualObj->index(1);
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, EditorResourcesGroup);
	
	return mesh;
}

Ogre::MeshPtr MGE::AxisGizmo::createScaleArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name) {
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_LINE_LIST);
	manualObj->position(0, 0, 0);
	manualObj->position(3, 0, 0);
	
	manualObj->index(0);
	manualObj->index(1);
	manualObj->end();
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_TRIANGLE_LIST);
	
	manualObj->position(2.85f, 0, 0);
	for(float theta = 0; theta < 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy) {
		manualObj->position(2.85f, radius * cos(theta), radius * sin(theta));
	}
	manualObj->position(3.45f,  0,      0);
	
	manualObj->position(3.40f,  0.20f,  0.20f);
	manualObj->position(3.40f,  0.20f, -0.20f);
	manualObj->position(3.40f, -0.20f, -0.20f);
	manualObj->position(3.40f, -0.20f,  0.20f);
	manualObj->position(3.50f,  0.20f,  0.20f);
	manualObj->position(3.50f,  0.20f, -0.20f);
	manualObj->position(3.50f, -0.20f, -0.20f);
	manualObj->position(3.50f, -0.20f,  0.20f);
	
	for(int inside = 1;inside < 16;inside++) {
		manualObj->index(0);
		manualObj->index(inside);
		manualObj->index(inside + 1);
	}
	manualObj->index(0);
	manualObj->index(16);
	manualObj->index(1);
	
	for(int outside = 1;outside < 16;outside++) {
		manualObj->index(17);
		manualObj->index(outside);
		manualObj->index(outside + 1);
	}
	manualObj->index(17);
	manualObj->index(16);
	manualObj->index(1);
	
	manualObj->index(18);
	manualObj->index(19);
	manualObj->index(20);
	manualObj->index(18);
	manualObj->index(20);
	manualObj->index(21);
	
	manualObj->index(22);
	manualObj->index(23);
	manualObj->index(24);
	manualObj->index(22);
	manualObj->index(24);
	manualObj->index(25);
	
	manualObj->index(18);
	manualObj->index(22);
	manualObj->index(25);
	manualObj->index(18);
	manualObj->index(25);
	manualObj->index(21);
	
	manualObj->index(19);
	manualObj->index(23);
	manualObj->index(24);
	manualObj->index(19);
	manualObj->index(24);
	manualObj->index(20);
	
	manualObj->index(18);
	manualObj->index(22);
	manualObj->index(23);
	manualObj->index(18);
	manualObj->index(23);
	manualObj->index(19);
	
	manualObj->index(21);
	manualObj->index(20);
	manualObj->index(24);
	manualObj->index(21);
	manualObj->index(24);
	manualObj->index(25);
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, EditorResourcesGroup);
	
	return mesh;
}

Ogre::MeshPtr MGE::AxisGizmo::createRotateArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name) {
	Ogre::ManualObject* manualObj = manager->createManualObject();
	
	float division = (Ogre::Math::PI / 2.0f) / 16.0f;
	float start = division * 3;
	float end = division * 14;
	
	int index_pos = 0;
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_LINE_STRIP);
	
	for(float theta = start; theta < end; theta += division) {
		manualObj->position(0, 3.0f * cos(theta), 3.0f * sin(theta));
		manualObj->index(index_pos++);
	}
	
	manualObj->end();
	
	manualObj->begin("MAT_GIZMO_ALL", Ogre::OT_TRIANGLE_LIST);
	
	Ogre::Quaternion q1;
	q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0,0,1));
	Ogre::Quaternion q2;
	q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0,1,0));
	
	Ogre::Vector3 translate1(0, 3.0f * cos(end), 3.0f * sin(end));
	Ogre::Vector3 translate2(0, 3.0f * cos(start), 3.0f * sin(start) - 0.25f);
	
	Ogre::Vector3 pos(-0.3f, 0, 0);
	manualObj->position(q1 * pos + translate1);
	
	for(float theta = 0; theta < 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy) 
	{
		pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
		manualObj->position(q1 * pos + translate1);
	}
	pos = Ogre::Vector3(0.3f, 0 , 0);
	manualObj->position(q1 * pos + translate1);

	pos = Ogre::Vector3(-0.3f, 0, 0);
	manualObj->position(q2 * pos + translate2);
	
	for(float theta = 0; theta < 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy) 
	{
		pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
		manualObj->position(q2 * pos + translate2);
	}
	pos = Ogre::Vector3(0.3f, 0 , 0);
	manualObj->position(q2 * pos + translate2);
	
	for(int inside = 1;inside < 16;inside++)
	{
		manualObj->index(0);
		manualObj->index(inside);
		manualObj->index(inside + 1);
	}
	manualObj->index(0);
	manualObj->index(16);
	manualObj->index(1);
	
	for(int outside = 1;outside < 16;outside++) {
		manualObj->index(17);
		manualObj->index(outside);
		manualObj->index(outside + 1);
	}
	manualObj->index(17);
	manualObj->index(16);
	manualObj->index(1);
	
	for(int inside = 19;inside < 34;inside++) {
		manualObj->index(18);
		manualObj->index(inside);
		manualObj->index(inside + 1);
	}
	manualObj->index(18);
	manualObj->index(34);
	manualObj->index(19);
	
	for(int outside = 19;outside < 34;outside++) {
		manualObj->index(35);
		manualObj->index(outside);
		manualObj->index(outside + 1);
	}
	manualObj->index(35);
	manualObj->index(34);
	manualObj->index(19);
	
	manualObj->end();
	
	Ogre::MeshPtr mesh = MGE::OgreUtils::convertManualToMesh(manualObj, name, EditorResourcesGroup);
	
	return mesh;
}
