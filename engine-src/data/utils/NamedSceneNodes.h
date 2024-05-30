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

#include "StringTypedefs.h"

#include <OgreVector3.h>
#include <OgreQuaternion.h>
namespace Ogre {
	class SceneManager;
	class SceneNode;
	class MovableObject;
}

namespace MGE {

/// @addtogroup OgreWorldUtils
/// @{
/// @file

/**
 * @brief 
 *
 * `#include <namedSceneNodes.h>`
 */
namespace NamedSceneNodes { 
	/**
	 * @brief get named scene node based on name
	 * 
	 * @param name      name of scene node to return
	 */
	Ogre::SceneNode* getSceneNode(
		const std::string_view& name
	);
	
	/**
	 * @brief get movable based on name and type
	 * 
	 * @param nodeName      name of scene node to find movable
	 * @param movableName   name of movable
	 * @param movableType   type of movable – e.g. Ogre::ItemFactory::FACTORY_TYPE_NAME, Ogre::v1::EntityFactory::FACTORY_TYPE_NAME, Ogre::v1::BillboardSetFactory::FACTORY_TYPE_NAME, ...
	 *                                        or empty string (default) for disable type checking
	 */
	Ogre::MovableObject* getMovable(
		const std::string_view& nodeName,
		const std::string_view& movableName,
		const std::string_view& movableType = MGE::EMPTY_STRING_VIEW
	);
	
	/**
	 * @brief create named scene node
	 * 
	 * @param name      name of created scene node (when empty - create un-named/auto-named node; when non empty - must be unique)
	 * @param parent    parent node
	 * @param type      dynamic or static
	 * @param position  position in @a parent space
	 * @param rotation  rotation in @a parent space
	 * @param scale     scale in LOCAL space
	 */
	Ogre::SceneNode* createSceneNode(
		const std::string_view&   name,
		Ogre::SceneNode*          parent,
		Ogre::SceneMemoryMgrTypes type     = Ogre::SCENE_DYNAMIC,
		const Ogre::Vector3&      position = Ogre::Vector3::ZERO,
		const Ogre::Quaternion&   rotation = Ogre::Quaternion::IDENTITY,
		const Ogre::Vector3&      scale    = Ogre::Vector3::UNIT_SCALE
	);
	
	/**
	 * @brief create un-named/auto-named scene node
	 * 
	 * @param parent    parent node (when NULL use RootSceneNode)
	 * @param type      dynamic or static
	 * @param position  position in @a parent space
	 * @param rotation  rotation in @a parent space
	 * @param scale     scale in LOCAL space
	 */
	Ogre::SceneNode* createSceneNode(
		Ogre::SceneNode*          parent,
		Ogre::SceneMemoryMgrTypes type     = Ogre::SCENE_DYNAMIC,
		const Ogre::Vector3&      position = Ogre::Vector3::ZERO,
		const Ogre::Quaternion&   rotation = Ogre::Quaternion::IDENTITY,
		const Ogre::Vector3&      scale    = Ogre::Vector3::UNIT_SCALE
	);
}


/// @}

}
