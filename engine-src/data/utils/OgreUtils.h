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

#include <OgreVector2.h>
#include <OgreSceneNode.h>
#include <Math/Simple/OgreAabb.h>

#include <unordered_map>

namespace MGE {

/// @addtogroup OgreWorldUtils
/// @{
/// @file

/**
 * @brief class with ogre utils functions
 */
class OgreUtils {
public:
	/**
	 * @brief return vector (@a vector) rotated by angle (@a angle)
	 * 
	 * @param[in] vector vector to rotating
	 * @param[in] angle rotating angle
	 */
	static Ogre::Vector2 rotateVector2(const Ogre::Vector2& vector, const Ogre::Radian& angle);
	
	/**
	 * @brief convert direction vector (@a direction) and current orientation info (@a currentOrientation) into new orientation
	 * 
	 * @param[in] direction direction vector
	 * @param[in] currentOrientation current orientation
	 * 
	 * @return orientation after make rotation according to direction vector
	 */
	static Ogre::Quaternion directionToOrientation(const Ogre::Vector3& direction, const Ogre::Quaternion& currentOrientation);
	
	/**
	 * @brief return true when @a node1 is child of @a node2 (or is the same node)
	 */
	static bool isChildOfNode(const Ogre::SceneNode* node1, const Ogre::SceneNode* node2);
	
	/**
	 * @brief return child of @a parent with name @a cihldName, NULL when can't find this child
	 */
	static Ogre::Node* getNamedChildOfNode(Ogre::SceneNode* parent, const Ogre::String& cihldName);
	
	/**
	 * @brief recursive clone scene node (with attached MovableObject)
	 * 
	 * @param src           node to clone
	 * @param dst           node to put cloned objects / nodes
	 */
	static void recursiveCloneSceneNode( Ogre::SceneNode* src, Ogre::SceneNode* dst );
	
	/// function to remove object from any
	typedef void(*AnyRemoverFun)(const Ogre::Any&);
	
	/**
	 * @brief recursive delete scene node (with attached MovableObject and physics)
	 * 
	 * @param pNode          node to deleted
	 * @param deleteParent   when true (deafault) delete @a pNode too
	 */
	static void recursiveDeleteSceneNode(
		Ogre::Node* pNode,
		bool deleteParent = true
	);
	
	/**
	 * @brief recursive update bindings (eg. pointer to actor) in scene node
	 * 
	 * @param pNode  node to update
	 * @param name   name of bindings to update
	 * @param any    Ogre::Any object to set
	 */
	static void recursiveUpdateBindings(Ogre::Node* pNode, const char* name, const Ogre::Any& any);
	
	
	/**
	 * @brief recursive update QueryFlags in scene node
	 *
	 * newQueryFlags = (currentQueryFlags & @a andMask) | @a orMask
	 *
	 * @param pNode    node to update
	 * @param andMask  value to make bitwise "and" with current QueryFlags value
	 * @param orMask   value to make bitwise "or"  with results of "and" operation
	 */
	static void recursiveUpdateQueryFlags(Ogre::Node* pNode, int andMask, int orMask);
	
	
	/**
	 * @brief recursive update QueryFlags in scene node
	 *
	 * newQueryFlags = (currentQueryFlags & @a andMask) | @a orMask
	 *
	 * @param pNode    node to update
	 * @param andMask  value to make bitwise "and" with current QueryFlags value
	 * @param orMask   value to make bitwise "or"  with results of "and" operation
	 */
	static void recursiveUpdateVisibilityFlags(Ogre::Node* pNode, int andMask, int orMask);
	
	
	/**
	 * @brief update cached transform of @a node and worldAABB of movables attachet to this Node
	 * 
	 * @param node          node to update
	 * @param updateAABB    when true update worldAABB of node and (if enabled) child nodes
	 * @param recursive     when true update transformations (and AABBs if enabled) of child nodes
	 * @param updateParent  when true update transformations of parent node
	 */
	static void updateCachedTransform(Ogre::Node* node, bool updateAABB = true, bool recursive = true, bool updateParent = false);
	
	/**
	 * @brief return color materal name based on Ogre::ColourValue @a color
	 *        when materal don't exist create it
	 */
	static Ogre::String getColorMaterial(const Ogre::ColourValue& color);
	
	/**
	 * @brief return color datablock name based on Ogre::ColourValue @a color
	 *        when datablock don't exist create it
	 */
	static Ogre::String getColorDatablock(const Ogre::ColourValue& color);
	
	/**
	 * @brief convert v2 ManualObject to v2 mesh
	 * 
	 * @param manual  pointer to manual object to convert
	 * @param name    name for created mesh
	 * @param group   group name for created mesh
	 */
	static Ogre::MeshPtr convertManualToMesh(Ogre::ManualObject* manual, const Ogre::String& name, const Ogre::String& group);
	
	/**
	 * @brief set datablock for all ManualObject sections
	 * 
	 * @param manualObject  to set datablock
	 * @param datablock     datablock to set
	 */
	static void setDatablock(Ogre::ManualObject* manualObject, Ogre::HlmsDatablock* datablock);
	
	/**
	 * @brief get first datablock from item
	 * 
	 * @param item          to get datablock
	 */
	static Ogre::HlmsDatablock* getFirstDatablock(Ogre::Item* item);
};


/// @}

}
