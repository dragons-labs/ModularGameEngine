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

#include "input/CameraControl.h"

#include "rendering/CameraSystem.h"

#include "LogSystem.h"
#include "Engine.h"

MGE::CameraControl::CameraControl() :
	 x(0.0),  y(0.0),  z(0.0),  zoom(0.0),
	kx(0.0), ky(0.0), kz(0.0), kzoom(0.0),
	 yaw(0.0),  pitch(0.0),  fov(0.0),
	kyaw(0.0), kpitch(0.0), kfov(0.0),
	mouseMarginX(0), mouseMarginY(0)
{
	LOG_HEADER("Create CameraControl" );
	
	// register main loop / update listener
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, INPUT_ACTIONS+3); // must be post call Input Listeners, but before CameraSystem
	MGE::InputSystem::getPtr()->registerListener(
		this,
		-1, MGE::InputSystem::Listener::CAMERA_CONTROL, -1,
		MGE::InputSystem::Listener::CAMERA_CONTROL, MGE::InputSystem::Listener::CAMERA_CONTROL, MGE::InputSystem::Listener::CAMERA_CONTROL
	);
}

MGE::CameraControl::~CameraControl() {
	LOG_INFO("Destroy CameraControl");
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_CameraControl \<CameraControl\>

@c \<CameraControl\> is used for setup <b>Camera Control</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(CameraControl) {
	return new MGE::CameraControl();
}


bool MGE::CameraControl::lostInput(bool /*full*/) {
	mouseMarginX = 0;
	mouseMarginY = 0;
	return false;
}

bool MGE::CameraControl::mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* /*_activeTextureObject*/ ) {
	auto currentCamera = MGE::CameraSystem::getPtr()->getCurrentCamera();
	
	if(!currentCamera)
		return false;
	
	// Mouse moving to screen margins
	if (mousePos.x < currentCamera->mouseMaginSize) {
		mouseMarginX = -currentCamera->mouseMaginStep;
	} else if (1.0 - mousePos.x < currentCamera->mouseMaginSize) {
		mouseMarginX = currentCamera->mouseMaginStep;
	} else {
		mouseMarginX = 0;
	}
	
	if (mousePos.y < currentCamera->mouseMaginSize) {
		mouseMarginY = -currentCamera->mouseMaginStep;
	} else if (1.0 - mousePos.y < currentCamera->mouseMaginSize) {
		mouseMarginY = currentCamera->mouseMaginStep;
	} else {
		mouseMarginY = 0;
	}
	
	// Mouse Scroll => zoom
	if (arg.state.Z.rel < 0) {
		if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RCONTROL) || MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LCONTROL))
			fov  += currentCamera->mouseFOVStep;
		else
			zoom += currentCamera->mouseZoomStep;
	} else if (arg.state.Z.rel > 0) {
		if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RCONTROL) || MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LCONTROL))
			fov  -= currentCamera->mouseFOVStep;
		else
			zoom -= currentCamera->mouseZoomStep;
	}
	
	// Mouse Middle Button
	if (arg.state.buttons & (1 << OIS::MB_Middle)) {
		if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RSHIFT) || MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LSHIFT)) {
			// with shift => moving camera
			if (arg.state.X.rel < 0)
				x -= currentCamera->mouseMoveStep;
			else if (arg.state.X.rel > 0)
				x += currentCamera->mouseMoveStep;
			if (arg.state.Y.rel < 0)
				z -= currentCamera->mouseMoveStep;
			else if (arg.state.Y.rel > 0)
				z += currentCamera->mouseMoveStep;
		} else {
			// without shift => rotating camera
			if (arg.state.X.rel < 0)
				yaw -= currentCamera->mouseRotateStep;
			else if (arg.state.X.rel > 0)
				yaw += currentCamera->mouseRotateStep;
			if (arg.state.Y.rel < 0)
				pitch -= currentCamera->mouseRotateStep;
			else if (arg.state.Y.rel > 0)
				pitch += currentCamera->mouseRotateStep;
		}
	}
	
	return false;
}

bool MGE::CameraControl::keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* /*_activeTextureObject*/ ) {
	auto currentCamera = MGE::CameraSystem::getPtr()->getCurrentCamera();
	
	if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::NumLock) || !currentCamera)
		return false;
	
	switch(static_cast<int>(arg.key)) {
		case OIS::KC_SYSRQ:    // PrintScreen         ==> make screenshot
			currentCamera->writeScreenshot("screenshot", "png");
			return true;
		
		case OIS::KC_NUMPAD4:
			kx = -currentCamera->kbdMoveStep;
			return true;
		case OIS::KC_NUMPAD6:
			kx = currentCamera->kbdMoveStep;
			return true;
		case OIS::KC_NUMPAD8:
			kz = -currentCamera->kbdMoveStep;
			return true;
		case OIS::KC_NUMPAD2:
			kz = currentCamera->kbdMoveStep;
			return true;
		case OIS::KC_NUMPAD3:
			ky = -currentCamera->kbdMoveStep;
			return true;
		case OIS::KC_NUMPAD9:
			ky = currentCamera->kbdMoveStep;
			return true;
		
		case OIS::KC_NUMPAD0:
			kyaw = -currentCamera->kbdRotateStep;
			return true;
		case OIS::KC_DECIMAL:
			kyaw = currentCamera->kbdRotateStep;
			return true;
		case OIS::KC_NUMPAD1:
			kpitch = -currentCamera->kbdRotateStep;
			return true;
		case OIS::KC_NUMPAD7:
			kpitch = currentCamera->kbdRotateStep;
			return true;
		
		case OIS::KC_ADD:
			kzoom = -currentCamera->kbdZoomStep;
			return true;
		case OIS::KC_SUBTRACT:
			kzoom = currentCamera->kbdZoomStep;
			return true;
		case OIS::KC_MULTIPLY:
			kfov = -currentCamera->kbdFOVStep;
			return true;
		case OIS::KC_DIVIDE:
			kfov = currentCamera->kbdFOVStep;
			return true;
		
		default:
			return false;
	}
}

bool MGE::CameraControl::keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* /*_activeTextureObject*/ ) {
	switch(static_cast<int>(arg.key)) {
		case OIS::KC_NUMPAD4:
		case OIS::KC_NUMPAD6:
			kx = 0;
			return true;
		case OIS::KC_NUMPAD8:
		case OIS::KC_NUMPAD2:
			kz = 0;
			return true;
		case OIS::KC_NUMPAD3:
		case OIS::KC_NUMPAD9:
			ky = 0;
			return true;
		
		case OIS::KC_DECIMAL:
		case OIS::KC_NUMPAD0:
			kyaw = 0;
			return true;
		case OIS::KC_NUMPAD1:
		case OIS::KC_NUMPAD7:
			kpitch = 0;
			return true;
		
		case OIS::KC_ADD:
		case OIS::KC_SUBTRACT:
			kzoom = 0;
			return true;
		case OIS::KC_MULTIPLY:
		case OIS::KC_DIVIDE:
		case 0x62:
			kfov = 0;
			return true;
		default:
			return false;
	}
}

bool MGE::CameraControl::update(float gameTimeStep, float realTimeStep) {
	auto currentCamera = MGE::CameraSystem::getPtr()->getCurrentCamera();
	
	if(!currentCamera)
		return false;
	
	x += kx;  y += ky;  z += kz;
	yaw += kyaw;  pitch += kpitch;
	zoom += kzoom;  fov += kfov;
	
	x += mouseMarginX;
	z += mouseMarginY;
	
	// camera move
	if ((x !=0 || y!=0 || z!=0)) {
		float zoomScaler = currentCamera->getZoom() * currentCamera->zoomMultiplier;
		if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift)) {
			zoomScaler *= currentCamera->shiftMultiplier;
		}
		x *= zoomScaler;
		y *= zoomScaler;
		z *= zoomScaler;
		currentCamera->move(x, y, z);
		x=0; y=0; z=0;
	}
	
	// camera zoom
	if (zoom != 0) {
		float zoomScaler = currentCamera->getZoom() * currentCamera->zoomMultiplier;
		if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift)) {
			zoom *= currentCamera->shiftMultiplier;
		}
		currentCamera->incDistance(zoom * zoomScaler);
		zoom = 0;
	}
	
	// camera height and angle to XY surface
	if (yaw.valueRadians() != 0) {
		currentCamera->incYaw(yaw);
		yaw = 0;
	}
	
	// camera rotation at XY surface
	if (pitch.valueRadians() != 0) {
		currentCamera->incPitch(pitch);
		pitch = 0;
	}
	
	// camera Field of View
	if (fov.valueRadians() != 0) {
		currentCamera->incFOV(fov);
		fov = 0.0;
	}
	
	return true;
}
