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

#include "pragma.h"
#include <OgreResourceGroupManager.h>

namespace Ogre { class SceneManager; class SceneNode; class MovableObject; }

namespace MGE {
/// @addtogroup OgreWorldUtils
/// @{
/// @file

	/**
	 * @brief Structure describing scene object.
	 * 
	 * @remark  In most case information in this structure is redundant.
	 * 
	 */
	struct SceneObjectInfo {
		/// Pointer to this scene object ogre scene node.
		Ogre::SceneNode* node;
		
		/// Pointer to this scene object ogre movable object.
		Ogre::MovableObject* movable;
	};
	
	/**
	 * @brief Structure describing restoring / loading context.
	 */
	struct LoadingContext {
		MGE_CLANG_WARNING_IGNORED("-Wshadow-field-in-constructor")
		
		/// Constructor
		LoadingContext(
			Ogre::SceneManager* scnMgr = nullptr,
			bool preLoad = false,
			bool linkToXML = false,
			std::string_view defaultResourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
		) :
			scnMgr(scnMgr),
			preLoad(preLoad),
			linkToXML(linkToXML),
			defaultResourceGroup(defaultResourceGroup)
		{}
		
		MGE_CLANG_WARNING_POP
		
		/// Pointer to scene manager used for creating this scene object.
		Ogre::SceneManager* scnMgr;
		
		/// If true, in next step (after pre-loading) will be loaded save, so can skip some part of loading.
		/// Can be ignored by some loading functions.
		bool preLoad;
		
		/// If true store info about source XML in created scene objects.
		/// For editor support, so typically used only for editor selectable objects.
		bool linkToXML;
		
		// Name of default resource group for use when not provided for element by its xml config.
		std::string defaultResourceGroup;
	};

/// @}

}

