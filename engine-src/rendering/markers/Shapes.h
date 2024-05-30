/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2008-2013 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team

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

#include <OgreMesh.h>

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief Simple manual meshes
 */
namespace Shapes {
	/// create vertical plane
	Ogre::MeshPtr createPlaneMesh(Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material);
	
	/// create sphere
	Ogre::MeshPtr createSphereMesh(Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material, const float r = 1.0, const int nRings = 8, const int nSegments = 8);
	
	/// create cone with apex show UNIT_Z
	Ogre::MeshPtr createConeMesh(Ogre::SceneManager* manager, const Ogre::String& name, const Ogre::String& group, const Ogre::String& material, const float r = 0.3, const float h = 1.0, const int nSegments = 8);
}

/// @}

}
