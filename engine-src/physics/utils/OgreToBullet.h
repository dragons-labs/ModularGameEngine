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

#pragma   once

#include <OgreNode.h>
#include <OgreMatrix4.h>
#include <OgreVector3.h>

#include <unordered_map>

class btCollisionObject;

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Propagation transforms (position, rotations, scale) from Ogre to Bullet
 * 
 * This class is for make setPosition(), setOrientation and similar on Ogre::Node working with Bullet physic object
 * 
 * @note We don't use Ogre::Node listener interface because Ogre 2.1 call Ogre::Node::Listener::nodeUpdated() every frame,
 *       regardless of transforms changes or not. So do this in this way.
 */
class OgreToBullet {
public:
	inline void addObj(btCollisionObject* obj, Ogre::Node* node, const Ogre::Vector3& offset) {
		nodes.insert( {obj, {node, node->_getFullTransform(), offset}} );
	}
	
	inline void remObj(btCollisionObject* obj) {
		nodes.erase(obj);
	}
	
	inline void clearAll() {
		nodes.clear();
	}
	
	void updateAll();
	
protected:
	struct PhyInfo {
		Ogre::Node*        node;
		Ogre::Matrix4      transform;
		Ogre::Vector3      offset;
	};
	std::unordered_map<btCollisionObject*, PhyInfo> nodes;
	
	static constexpr float EPSION1 = 0.001;
	static constexpr float EPSION2 = 0.99999f;
};

/// @}

}
