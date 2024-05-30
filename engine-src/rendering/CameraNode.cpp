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

#include "rendering/CameraNode.h"
#include "rendering/CameraSystem.h"
#include "rendering/RenderingSystem.h"

#include "FormatTime.h"
#include "LogSystem.h"
#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"
#include "data/utils/NamedSceneNodes.h"
#include "data/utils/OgreUtils.h"

#include <OgreSceneNode.h>
#include <OgreItem.h>

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorNode.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassScene.h>

#undef  LOG_MODULE_NAME
#define LOG_MODULE_NAME "Camera: " + getName()


MGE::CameraNode::CameraNode(const std::string& name, Ogre::SceneManager* scnMgr) {
	LOG_INFO("Camera: " + name, "create");
	
	node   = scnMgr->getRootSceneNode()->createChildSceneNode();
	camera = scnMgr->createCamera(name);
	camera->detachFromParent();
	camera->setQueryFlags(0);
	node->attachObject( camera );
	camera->setFixedYawAxis(true, Ogre::Vector3::UNIT_Y);
	
	#ifdef MGE_DEBUG_CAMERA_MARKER
	try { if (name != "LoadingScreen") node->attachObject(scnMgr->createItem("Axis.mesh")); } catch (...) {}
	#endif
	
	renderTarget = NULL;
	setMode();
	setOwner();
	
	// register write save (store) listener in CameraSystem
	if (MGE::CameraSystem::getPtr())
		MGE::CameraSystem::getPtr()->allCameraNodes[name] = this;
}

MGE::CameraNode::~CameraNode() {
	LOG_INFO("destroy");
	
	if (MGE::CameraSystem::getPtr())
		MGE::CameraSystem::getPtr()->allCameraNodes.erase( getName() );
	
	Ogre::SceneManager* scnMgr = getSceneManager();
	scnMgr->destroyCamera(camera);
	try {
		MGE::OgreUtils::recursiveDeleteSceneNode(node, true);
	} catch(Ogre::Exception& e) {
		LOG_ERROR("CameraNode destructor - error in recursiveDeleteSceneNode: " << e.getFullDescription());
	}
	
	if (workspace) {
		Ogre::Root::getSingletonPtr()->getCompositorManager2()->removeWorkspace(workspace);
	}
}

void MGE::CameraNode::setRenderTarget(
	Ogre::TextureGpu* _renderTarget, uint32_t visibilityMask, int ZOrder
) {
	LOG_INFO("set render target");
	
	if (renderTarget) {
		LOG_WARNING("this camera has render target :-o");
	} else {
		renderTarget = _renderTarget;
		
		Ogre::CompositorManager2* compositorManager = Ogre::Root::getSingletonPtr()->getCompositorManager2();
		const Ogre::String workspaceName("Workspace" + camera->getName());
		if ( !compositorManager->hasWorkspaceDefinition(workspaceName) ) {
			compositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue( 0.4f, 0.4f, 0.4f ));
		}
		workspace = compositorManager->addWorkspace( getSceneManager(), renderTarget, camera, workspaceName, true, -ZOrder );
		
		for (auto nodeIter = workspace->getNodeSequence().begin(); nodeIter != workspace->getNodeSequence().end(); ++nodeIter) {
			Ogre::CompositorNode* compNode = (*nodeIter);
			LOG_DEBUG("WNP: compNode=" << compNode << " (id=" << compNode->getId() << ")");
			for (auto passIter = compNode->_getPasses().begin(); passIter != compNode->_getPasses().end(); ++passIter) {
				if( (*passIter)->getType() == Ogre::PASS_SCENE ) {
					Ogre::CompositorPassScene* compPass = static_cast<Ogre::CompositorPassScene*>( *passIter );
					LOG_DEBUG("WNP: compPass=" << compPass);
					LOG_DEBUG("WNP: compPassDef=" << compPass->getDefinition());
					LOG_DEBUG("WNP: mFirstRQ=" << static_cast<int>(compPass->getDefinition()->mFirstRQ) << "  mLastRQ=" << static_cast<int>(compPass->getDefinition()->mLastRQ));
					scenePassDefs.insert(
						const_cast<Ogre::CompositorPassSceneDef*>(compPass->getDefinition())
					);
				}
			}
		}
		for (auto& d : scenePassDefs) {
			d->setVisibilityMask(visibilityMask);
		}
		camera->setAutoAspectRatio( true );
	}
}


void MGE::CameraNode::setMode(bool _rotationAllowed, bool _moveAllowed, bool _lookOutside) {
	LOG_INFO("set mode");
	
	if (_lookOutside)
		lookDirection  = LOOK_FROM;
	else
		lookDirection  = LOOK_AT;
		
	rotationAllowed    = _rotationAllowed;
	moveAllowed        = _moveAllowed;
	moveAllowedNoOwner = _moveAllowed;
}

void MGE::CameraNode::setOwner(const std::string_view& _owner, bool _autoGetOwnerPosition, bool _autoGetOwnerRotation) {
	setOwner(
		MGE::NamedSceneNodes::getSceneNode(_owner),
		_autoGetOwnerPosition,
		_autoGetOwnerRotation
	);
}

void MGE::CameraNode::setOwner(const Ogre::SceneNode* _owner, bool _autoGetOwnerPosition, bool _autoGetOwnerRotation) {
	if (_owner) {
		LOG_INFO("set owner");
		owner = _owner;
		autoGetOwnerPosition = _autoGetOwnerPosition;
		autoGetOwnerRotation = _autoGetOwnerRotation;
		if (autoGetOwnerPosition) {
			moveAllowed = false;
		}
	} else {
		LOG_INFO("unset owner");
		owner = NULL;
		autoGetOwnerPosition = false;
		autoGetOwnerRotation = false;
		moveAllowed = moveAllowedNoOwner;
	}
}

void MGE::CameraNode::update() {
	// recalculate and set (x,y,z) camera position (relative to node) and rotate camera to look at/from node
	if (needInternalPositionUpdate && rotationAllowed) {
		// calculate (convert from spherical coordinate system) and set position for camera
		Ogre::Vector3 position(
			camDistance * camPitchCos * camYawSin,
			camDistance * camPitchSin,
			camDistance * camPitchCos * camYawCos
		);
		camera->setPosition(position);
		
		// rotate camera to look at/from parent (node), this do the same as:
		//   setDirection(lookDirection * position , Ogre::Node::TS_PARENT, Ogre::Vector3::NEGATIVE_UNIT_Z)
		// or (for lookDirection == -1) as:
		//   lookAt(Vector3::ZERO, Ogre::Node::TS_PARENT, Ogre::Vector3::NEGATIVE_UNIT_Z)
		// for Ogre::SceneNode
		node->setOrientation(Ogre::Quaternion::IDENTITY);
		camera->setDirection(lookDirection * position);
	}
	needInternalPositionUpdate = false;
	
	if (autoGetOwnerRotation)
		node->setOrientation( owner->getOrientation() );
	
	if (autoGetOwnerPosition)
		node->setPosition( owner->getPosition() );
}

/**
@page XMLSyntax_Misc

@subsection XMLNode_Camera Camera save or config
Camera save or config, contains next subnodes:
	- @c Limits for set camera movment and adjust limits, with subnodes:
		- @c PositionMax and @c PositionMin
		- @c ZoomMax and @c ZoomMin
		- @c PitchMax and @c PitchMin
		- @c FarClipDistance and @c NearClipDistance
		- @c FOVMax and @c FOVMin
	- @c Limits for set camera movment and adjust speeds, with subnodes:
		- for keyboard control:
			- @c MoveStep size (length) of single move step
			- @c ZoomStep size of single zoom step
			- @c FOVStep size of single zoom via FOV step
			- @c RotateStep size of single rotate step
			- @c ShiftMultiplier multipliter for move when use shift key
			- @c ZoomMultiplier multipliter for zoom when use shift key
		- for mouse control:
			- @c MouseMoveStep, @c MouseZoomStep, @c MouseFOVStep and @c MouseRotateStep as previous, but for mouse input
			- @c MouseMarginSize size of screen margins (edges) for move when cursor is on screen edge
			- @c MouseMarginStep size of move for move when cursor is on screen edge
	- @c Mode for set camera mode, with subnodes:
		- @c RotationAllowed camera can be rotated via keyboard or mouse controls
		- @c MoveAllowedcamera can be moved via keyboard or mouse controls
		- @c LookOutside @ref XML_Bool when true camera looks in back direction (not into camera main node, but like from camera main node)
	- @c Place for set camera parameters, with subnodes:
		- for camera main node (foolowing scene node) settings:
			- @c Position @ref XML_Vector3 with 3D world position of camera main node
			- @c Orientation @ref XML_Quaternion with camera main node orientation
		- for camera object relative to camera main node position
			- @c Zoom with camera distance from camera main node
			- @c Yaw and @c Pitch for set camera rotation relative to camera main
	- @c Owner for set camera owner
		- @c Name  name of owner scene node
		- @c GetPosition @ref XML_Bool when true camera use (copy) position of owner for camera main node
		- @c GetRotation @ref XML_Bool when true camera use (copy) orientation of owner for camera main node
		.
		when both @c GetPosition and @c GetRotation is false settings of owner has no effect to camera.
*/

bool MGE::CameraNode::storeToXML(pugi::xml_node&& xmlNode, bool onlyRef) const {
	LOG_INFO("store");
	
	{
		auto xmlSubNode = xmlNode.append_child("Limits");
		xmlSubNode.append_child("PositionMax") <<       limitPositionMax;
		xmlSubNode.append_child("PositionMin") <<       limitPositionMin;
		xmlSubNode.append_child("PitchMax") <<          limitPitchMax.valueRadians();
		xmlSubNode.append_child("PitchMin") <<          limitPitchMin.valueRadians();
		xmlSubNode.append_child("ZoomMax") <<           limitZoomMax;
		xmlSubNode.append_child("ZoomMin") <<           limitZoomMin;
		xmlSubNode.append_child("FOVMax") <<            limitFOVMax.valueRadians();
		xmlSubNode.append_child("FOVMin") <<            limitFOVMin.valueRadians();
		xmlSubNode.append_child("FarClipDistance") <<   camera->getFarClipDistance();
		xmlSubNode.append_child("NearClipDistance") <<  camera->getNearClipDistance();
	}
	
	{
		auto xmlSubNode = xmlNode.append_child("Controls");
		xmlSubNode.append_child("MoveStep") <<          kbdMoveStep;
		xmlSubNode.append_child("ZoomStep") <<          kbdZoomStep;
		xmlSubNode.append_child("FOVStep") <<           kbdFOVStep.valueRadians();
		xmlSubNode.append_child("RotateStep") <<        kbdRotateStep.valueRadians();
		xmlSubNode.append_child("ShiftMultiplier") <<   shiftMultiplier;
		xmlSubNode.append_child("ZoomMultiplier") <<    zoomMultiplier;
		xmlSubNode.append_child("MouseMoveStep") <<     mouseMoveStep;
		xmlSubNode.append_child("MouseZoomStep") <<     mouseZoomStep;
		xmlSubNode.append_child("MouseFOVStep") <<      mouseFOVStep.valueRadians();
		xmlSubNode.append_child("MouseRotateStep") <<   mouseRotateStep.valueRadians();
		xmlSubNode.append_child("MouseMarginSize") <<   mouseMaginSize;
		xmlSubNode.append_child("MouseMarginStep") <<   mouseMaginStep;
	}
	
	{
		auto xmlSubNode = xmlNode.append_child("Mode");
		xmlSubNode.append_child("RotationAllowed") <<   rotationAllowed;
		xmlSubNode.append_child("MoveAllowed") <<       moveAllowed;
		xmlSubNode.append_child("LookOutside") <<       (lookDirection == LOOK_FROM);
	}
	
	{
		auto xmlSubNode = xmlNode.append_child("Place");
		xmlSubNode.append_child("Position") <<          getPosition();
		xmlSubNode.append_child("Orientation") <<       getOrientation();
		xmlSubNode.append_child("Yaw") <<               camYaw.valueRadians();
		xmlSubNode.append_child("Pitch") <<             camPitch.valueRadians();
		xmlSubNode.append_child("Zoom") <<              camDistance;
		xmlSubNode.append_child("Fov") <<               camera->getFOVy().valueRadians();
	}
	
	if (owner) {
		auto xmlSubNode = xmlNode.append_child("Owner");
		xmlSubNode.append_child("Name") <<              owner->getName();
		xmlSubNode.append_child("GetPosition") <<       autoGetOwnerPosition;
		xmlSubNode.append_child("GetRotation") <<       autoGetOwnerRotation;
	}
	return true;
}

bool MGE::CameraNode::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	LOG_INFO("configure");
	
	{
		LOG_INFO("configure - set camera limits");
		auto xmlSubNode = xmlNode.child("Limits");
		limitPositionMax  =   MGE::XMLUtils::getValue(xmlSubNode.child("PositionMax"),       Ogre::Vector3( 1000,  1000,  1000));
		limitPositionMin  =   MGE::XMLUtils::getValue(xmlSubNode.child("PositionMin"),       Ogre::Vector3(-1000, -1000, -1000));
		limitZoomMax      =   MGE::XMLUtils::getValue(xmlSubNode.child("ZoomMax"),           300.0);
		limitZoomMin      =   MGE::XMLUtils::getValue(xmlSubNode.child("ZoomMin"),           0.2);
		limitPitchMax     =   MGE::XMLUtils::getValue(xmlSubNode.child("PitchMax"),          1.4);
		limitPitchMin     =   MGE::XMLUtils::getValue(xmlSubNode.child("PitchMin"),         -1.4);
		camera->setFarClipDistance(
		                      MGE::XMLUtils::getValue(xmlSubNode.child("FarClipDistance"),   350.0)
		);
		camera->setNearClipDistance(
		                      MGE::XMLUtils::getValue(xmlSubNode.child("NearClipDistance"),  0.5)
		);
		limitFOVMax       =   MGE::XMLUtils::getValue(xmlSubNode.child("FOVMax"),            3.0);
		limitFOVMin       =   MGE::XMLUtils::getValue(xmlSubNode.child("FOVMin"),            0.1);
	}
	
	{
		LOG_INFO("configure - set camera control params");
		auto xmlSubNode = xmlNode.child("Limits");
		kbdMoveStep       =   MGE::XMLUtils::getValue(xmlSubNode.child("MoveStep"),          0.03);
		kbdZoomStep       =   MGE::XMLUtils::getValue(xmlSubNode.child("ZoomStep"),          kbdMoveStep);
		kbdFOVStep        =   MGE::XMLUtils::getValue(xmlSubNode.child("FOVStep"),           0.01);
		kbdRotateStep     =   MGE::XMLUtils::getValue(xmlSubNode.child("RotateStep"),        0.01);
		
		shiftMultiplier   =   MGE::XMLUtils::getValue(xmlSubNode.child("ShiftMultiplier"),   10.0);
		zoomMultiplier    =   MGE::XMLUtils::getValue(xmlSubNode.child("ZoomMultiplier"),    0.1);
		
		mouseMoveStep     =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseMoveStep"),     kbdMoveStep);
		mouseZoomStep     =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseZoomStep"),     kbdZoomStep * 5);
		mouseFOVStep      =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseFOVStep"),      kbdFOVStep.valueRadians() * 5);
		mouseRotateStep   =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseRotateStep"),   kbdRotateStep.valueRadians() * 10);
		
		mouseMaginSize    =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseMarginSize"),   0.01);
		mouseMaginStep    =   MGE::XMLUtils::getValue(xmlSubNode.child("MouseMarginStep"),   kbdMoveStep);
	}
	
	{
		LOG_INFO("configure - set camera mode");
		auto xmlSubNode = xmlNode.child("Mode");
		setMode(
		                      MGE::XMLUtils::getValue(xmlSubNode.child("RotationAllowed"),   true),
		                      MGE::XMLUtils::getValue(xmlSubNode.child("MoveAllowed"),       true),
		                      MGE::XMLUtils::getValue(xmlSubNode.child("LookOutside"),       false)
		);
	}
	
	{
		LOG_INFO("configure - set camera position, orientation, rotations and zoom");
		auto xmlSubNode = xmlNode.child("Place");
		setPosition(          MGE::XMLUtils::getValue(xmlSubNode.child("Position"),          Ogre::Vector3::ZERO) );
		setOrientation(       MGE::XMLUtils::getValue(xmlSubNode.child("Orientation"),       Ogre::Quaternion::IDENTITY) );
		
		setYaw(  Ogre::Radian(MGE::XMLUtils::getValue(xmlSubNode.child("Yaw"),               0.0)) );
		setPitch(Ogre::Radian(MGE::XMLUtils::getValue(xmlSubNode.child("Pitch"),             0.785)) );
		setDistance(          MGE::XMLUtils::getValue(xmlSubNode.child("Zoom"),              15.0) );
		setFOV(  Ogre::Radian(MGE::XMLUtils::getValue(xmlSubNode.child("Fov"),               0.785)) );
	}
	
	{
		LOG_INFO("configure - set camera owner");
		auto xmlSubNode = xmlNode.child("Owner");
		if (xmlSubNode) {
			setOwner(
				MGE::XMLUtils::getValue(xmlSubNode.child("Name"),              ""),
				MGE::XMLUtils::getValue(xmlSubNode.child("GetPosition"),       true),
				MGE::XMLUtils::getValue(xmlSubNode.child("GetRotation"),       true)
			);
		} else {
			setOwner(NULL);
		}
	}
	update();
	return true;
}

void MGE::CameraNode::remFromVisibilityMask(uint32_t val) {
	for (auto& d : scenePassDefs) {
		d->setVisibilityMask( d->mVisibilityMask & (~val) );
	}
}

void MGE::CameraNode::addToVisibilityMask(uint32_t val) {
	for (auto& d : scenePassDefs) {
		d->setVisibilityMask( d->mVisibilityMask | val );
	}
}

void MGE::CameraNode::writeScreenshot(const Ogre::String& name) const {
	renderTarget->writeContentsToFile(name, 1, 1);
}

std::string MGE::CameraNode::writeScreenshot(const std::string_view& prefix, const std::string_view& suffix) const {
	std::string ssfn = prefix + "_" + MGE::FormatTime::getTime(MGE::FormatTime::ISO_DATE_AND_TIME_COMPACT) + "." + suffix;
	writeScreenshot(ssfn);
	return ssfn;
}

void MGE::CameraNode::setPosition(const Ogre::Vector3& vec) {
	Ogre::Vector3 newPos;
	newPos.x = Ogre::Math::Clamp(vec.x, limitPositionMin.x, limitPositionMax.x);
	newPos.y = Ogre::Math::Clamp(vec.y, limitPositionMin.y, limitPositionMax.y);
	newPos.z = Ogre::Math::Clamp(vec.z, limitPositionMin.z, limitPositionMax.z);
	
	node->setPosition(newPos);
}

void MGE::CameraNode::move(const float right, const float up, const float forward)  {
	if (!moveAllowed)
		return;
	
	// calculate move size taking into account camera direction
	Ogre::Vector3 ori = camera->getDerivedOrientation().zAxis();
	ori.y = 0;
	ori.normalise();
	
	Ogre::Vector3 move(0, up, 0);
	move.z = ori.z * forward  - ori.x * right;
	move.x = ori.z * right + ori.x * forward;
	
	// add move size to current position, check limits and set new position
	setPosition(node->getPosition() + move);
}
