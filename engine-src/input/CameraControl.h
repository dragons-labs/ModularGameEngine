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

#pragma   once

#include "BaseClasses.h"
#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "rendering/CameraNode.h"
#include "input/InputSystem.h"

#include <OISKeyboard.h>
#include <OISMouse.h>

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief %Camera manager and RTS-style controls
 */
class CameraControl:
	public MGE::MainLoopListener,
	public MGE::InputSystem::Listener,
	public MGE::Module,
	public MGE::Singleton<CameraControl>
{
public:
	/**
	 * @brief Update camera controls: Check keyboard and mouse state, update camera parameters (position, orientation, ...) and update camera
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	virtual bool update(float gameTimeStep, float realTimeStep) override;
	
	/** @name Input interface
	 * 
	 * @{
	 */
		/// On mouse move event
		virtual bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
		/// On lost input
		virtual bool lostInput(bool full = false) override;
		/// On key press event
		virtual bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
		/// On key released event
		virtual bool keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
	/**
	 * @}
	 */
	
	/// constructor
	CameraControl();
	
	/// destructor
	~CameraControl();
	
private:
	/// step values for X camera movements for mouse input
	float x;
	/// step values for Y camera movements for mouse input
	float y;
	/// step values for Z camera movements for mouse input
	float z;
	/// step values for camera zoom for mouse input
	float zoom;
	
	/// step values for X camera movements for keyboard input
	float kx;
	/// step values for Y camera movements for keyboard input
	float ky;
	/// step values for Z camera movements for keyboard input
	float kz;
	/// step values for camera zoom for keyboard input
	float kzoom;
	
	/// step values for YAW camera rotations for mouse input
	Ogre::Radian yaw;
	/// step values for PITCH camera rotations for mouse input
	Ogre::Radian pitch;
	/// step values for camera FOV for mouse input
	Ogre::Radian fov;
	
	/// step values for YAW camera rotations for keyboard input
	Ogre::Radian kyaw;
	/// step values for PITCH camera rotations for keyboard input
	Ogre::Radian kpitch;
	/// step values for camera FOV for keyboard input
	Ogre::Radian kfov;
	
	/// size of scrren margin used to mouse movements in X axis
	float mouseMarginX;
	/// size of scrren margin used to mouse movements in Y axis
	float mouseMarginY;
};

/// @}

}
