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

#pragma   once

#include <stdint.h>

namespace MGE {

/// @addtogroup LoadingSystem
/// @{
/// @file

/**
 * @brief Bullet collision flag / Ogre::MovableObject QueryFlags used in engine.
 *
 * Flags from (1<<0) to (1<<5) is used internal by bullet (see btBroadphaseProxy::CollisionFilterGroups).
 *
 * Colision flag is using to:
 *   @li determinate colision type of object (as collisionFlag),
 *   @li determinate types of objects, which collide (as collisionMask),
 *
 * @note
 *   Do NOT use value from this enum in btCollisionObject::setCollisionFlags().
 *   For this function use only values from btCollisionObject::CollisionFlags.
 *
 * `#include <queryFlags.h>`
 */
namespace QueryFlags { enum QueryFlags : uint16_t {
	/// flag used for Targets
	/// should be used simultaneous with GAME_OBJECT, can be used with or without COLLISION_OBJECT (depending on the expected behavior of target)
	TARGET                = (1<<8),
	
	/// flag used for Triggers
	/// should be used simultaneous with GAME_OBJECT and NOT with COLLISION_OBJECT
	TRIGGER               = (1<<9),
	
	/// flag used for interactive UI object (eg. gizmo axis)
	INTERACTIVE_WIMGET    = (1<<10),
	
	/// flag used for object with interactive texture
	INTERACTIVE_TEXTURE   = (1<<11),
	
	/// flag used for ground collision object (terrain or ground meshes)
	/// this is exclusive case: GROUND flag should NOT be used simultaneous with COLLISION_OBJECT, OGRE_OBJECT, GAME_OBJECT, etc
	GROUND                = (1<<12),
	
	/// flag used for standard (not ground) collision object (not triggers, not crossable objects, ...)
	/// should be used simultaneous with OGRE_OBJECT or GAME_OBJECT
	COLLISION_OBJECT      = (1<<13),
	
	/// flag used for Ogre (not ground) object
	OGRE_OBJECT           = (1<<14),
	
	/// flag used for GameObject
	GAME_OBJECT           = (1<<15)
}; }


/// @}

}
