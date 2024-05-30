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

#include "NamedSceneNodes.h"

#include "pragma.h"
#include "LogSystem.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>


MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors")

namespace MGE { namespace NamedSceneNodes {
	std::unordered_map<std::string, Ogre::SceneNode*, MGE::string_hash, std::equal_to<>> namedSceneNodesMap;
} }

MGE_CLANG_WARNING_POP


Ogre::SceneNode* MGE::NamedSceneNodes::getSceneNode( const std::string_view& name ) {
	auto iter = namedSceneNodesMap.find( name );
	if (iter != namedSceneNodesMap.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

Ogre::MovableObject* MGE::NamedSceneNodes::getMovable( const std::string_view& nodeName, const std::string_view& movableName, const std::string_view& movableType ) {
	Ogre::SceneNode* node = getSceneNode(nodeName);
	if (!node) {
		LOG_ERROR("Can't find node for name: " << nodeName);
		return NULL;
	}
	
	auto iter1 = node->getAttachedObjectIterator();
	while(iter1.hasMoreElements()) {
		Ogre::MovableObject* m = iter1.getNext();
		if (
			m->getName() == movableName && ( m->getMovableType() == movableType || movableType == Ogre::BLANKSTRING )
		) {
			return m;
		}
	}
	
	LOG_ERROR("Can't find movable object for name: " << movableName << " in node: " << nodeName);
	return NULL;
}

Ogre::SceneNode* MGE::NamedSceneNodes::createSceneNode(
	const std::string_view&   name,
	Ogre::SceneNode*          parent,
	Ogre::SceneMemoryMgrTypes type,
	const Ogre::Vector3&      position,
	const Ogre::Quaternion&   rotation,
	const Ogre::Vector3&      scale
) {
	if (!name.empty() && getSceneNode(name)) {
		LOG_ERROR("SceneNode with name " << name << " already exists");
		return NULL;
	}
	
	Ogre::SceneNode* node = createSceneNode(parent, type, position, rotation, scale);
	
	if (!name.empty()) {
		auto named_node = std::make_pair(static_cast<std::string>(name), node);
		namedSceneNodesMap.insert(named_node);
		node->setName(named_node.first);
		LOG_DEBUG("name: " << name << " set for scene node at: " << position);
	}
	
	return node;
}

Ogre::SceneNode* MGE::NamedSceneNodes::createSceneNode(
	Ogre::SceneNode*          parent,
	Ogre::SceneMemoryMgrTypes type,
	const Ogre::Vector3&      position,
	const Ogre::Quaternion&   rotation,
	const Ogre::Vector3&      scale
) {
	Ogre::SceneNode* node = parent->createChildSceneNode(type, position, rotation);
	
	node->setScale(scale);
	
	LOG_DEBUG("scene node at: " << position << " created");
	return node;
}
