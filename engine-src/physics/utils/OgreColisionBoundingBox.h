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
#include "config.h"

#include <OgreSceneNode.h>
#include <OgreAxisAlignedBox.h>
#include <OgreSceneManager.h>

#include <Math/Simple/OgreAabb.h>

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief class with functions for OGRE BoundingBox based collision checking
 */
class OgreColisionBoundingBox {
public:
	/**
	 * @brief get full Axis Aligned Bounding Box in @a node local coordinate
	 * 
	 * @param[in]  node  node for getting full AABB
	 * @param[out] aabb  pointer to put results
	 * 
	 * @note to get in world coordinate use: 
	 *   \code{.cpp}  getLocalAABB(sn, bb); bb.transformAffine(sn->_getFullTransform());  \endcode
	 */
	static void getLocalAABB(const Ogre::SceneNode* node, Ogre::AxisAlignedBox* aabb, bool getScaled = false);
	
	/// @copydoc getLocalAABB(Ogre::Node*, Ogre::AxisAlignedBox*);
	static void getLocalAABB(const Ogre::SceneNode* node, Ogre::Aabb* aabb, bool getScaled = false);
	
	/**
	 * @brief return true when two Bounding Box intersects
	 * 
	 * @param[in] aabb1                     first Axis Aligned Bounding Box (in LOCAL space of @a node1
	 * @param[in] node1                     node used only for DEBUG visualisation (can be NULL)
	 * @param[in] node1DerivedPosition      position for transform of @a aabb1
	 * @param[in] node1DerivedOrientation   orientation for transform of @a aabb1
	 * @param[in] node1DerivedScale         scale for transform of @a aabb1
	 * @param[in] aabb2                     second Axis Aligned Bounding Box (in LOCAL space of @a node2
	 * @param[in] node2                     node used to get transform for @a aabb2
	 * 
	 * @note function convert Axis Aligned Bounding Box @a aabb2 from LOCAL space of @a node2
	 *       to Oriented Bounding Box in LOCAL space of @a node1 and check collision
	 * 
	 * @note @a aabb1 is scaled by derived @a node1 scale
	 *       for converting @a aabb2 to @a node1 LOCAL space, derived scale of @a node1 is UNITY
	 */
	static bool intersects(
		const Ogre::AxisAlignedBox& aabb1, const Ogre::SceneNode* node1,
		const Ogre::Vector3& node1DerivedPosition, const Ogre::Quaternion& node1DerivedOrientation, const Ogre::Vector3& node1DerivedScale,
		const Ogre::AxisAlignedBox& aabb2, const Ogre::SceneNode* node2
	);
	
	/**
	 * @brief return true when two Bounding Box intersects
	 * 
	 * @param[in] aabb1   first Axis Aligned Bounding Box (in LOCAL space of @a node1
	 * @param[in] node1   node used to get transform for @a aabb1
	 * @param[in] aabb2   second Axis Aligned Bounding Box (in LOCAL space of @a node2
	 * @param[in] node2   node used to get transform for @a aabb2
	 */
	inline static bool intersects(
		const Ogre::AxisAlignedBox& aabb1, const Ogre::SceneNode* node1,
		const Ogre::AxisAlignedBox& aabb2, const Ogre::SceneNode* node2
	) {
		return intersects(
			aabb1, node1,
			node1->_getDerivedPosition(), node1->_getDerivedOrientation(), node1->_getDerivedScale(),
			aabb2, node2
		);
	}
	
	/**
	 * @brief return true when ray intersects Axis Aligned Bounding Box
	 */
	static bool intersects(const Ogre::Ray& ray, const Ogre::AxisAlignedBox& box, Ogre::Real rayLen = Ogre::Math::POS_INFINITY);
	
	/**
	 * @brief check if position is free
	 * 
	 * @param[in]     scnMgr             pointer to SceneManager on which we do search
	 * @param[in]     position           point to check (centre of sphere)
	 * @param[in]     radius             request space size (radius of sphere)
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided
	 * 
	 * @return true if position is free, false otherwise
	 */
	static bool isFreeSphere(
		Ogre::SceneManager* scnMgr,
		const Ogre::Vector3& position,
		Ogre::Real radius,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = nullptr
	);
	
	
	/**
	 * @brief check if position of @a node is free (@a node do not collide with other scene elements)
	 * 
	 * @param[in]     node               pointer to SceneNode to check (for disable collision with own child nodes)
	 * @param[in]     aabb               Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     newPosition        proposed position for @a node to check (in WORLD space)
	 * @param[in]     newOrientation     proposed orientation for @a node to check (in WORLD space)
	 * @param[in]     newScale           proposed scale for @a node to check (in LOCAL space)
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 * 
	 * @return true if position is free, false otherwise
	 */
	static bool isFreePosition(
		const Ogre::SceneNode* node,
		const Ogre::AxisAlignedBox& aabb,
		const Ogre::Vector3& newPosition,
		const Ogre::Quaternion& newOrientation,
		const Ogre::Vector3& newScale,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	);
	
	/**
	 * @brief check if position of @a node is free (@a node do not collide with other scene elements)
	 * 
	 * @param[in]     node               pointer to SceneNode to check
	 * @param[in]     aabb               Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     newPosition        proposed position for @a node to check (in WORLD space)
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 * 
	 * @return true if position is free, false otherwise
	 */
	inline static bool isFreePosition(
		const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb,
		const Ogre::Vector3& newPosition, int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	) {
		return isFreePosition(
			node, aabb,
			newPosition, node->_getDerivedOrientation(), node->_getDerivedScale(),
			queryMask, collisionObjects
		);
	}
	
	/**
	 * @brief check if position of @a node is free (@a node do not collide with other scene elements)
	 * 
	 * @param[in]     node               pointer to SceneNode to check
	 * @param[in]     aabb               Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 * 
	 * @return true if position is free, false otherwise
	 */
	static bool isFreePosition(
		const Ogre::SceneNode* node,
		const Ogre::AxisAlignedBox& aabb,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	) {
		return isFreePosition(
			node, aabb,
			node->_getDerivedPosition(), node->_getDerivedOrientation(), node->_getDerivedScale(),
			queryMask, collisionObjects
		);
	}
	
	/**
	 * @brief check if position of @a node is free (@a node do not collide with other scene elements)
	 * 
	 * @param[in]     node               pointer to SceneNode to check
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 * 
	 * @return true if position is free, false otherwise
	 */
	inline static bool isFreePosition(
		const Ogre::SceneNode* node,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	) {
		Ogre::AxisAlignedBox aabb;
		getLocalAABB(node, &aabb);
		return isFreePosition(node, aabb, queryMask, collisionObjects);
	}
	
	/**
	 * @brief check path between start and end
	 * 
	 * @param[in]     node               pointer to SceneNode to check (for disable collision with own child nodes)
	 * @param[in]     aabb               Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     start              path start point
	 * @param[in]     end                path end point
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 */
	static bool isFreePath(
		const Ogre::SceneNode* node,
		const Ogre::AxisAlignedBox& aabb,
		const Ogre::Vector3& start,
		const Ogre::Vector3& end,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	);
	
	/**
	 * @brief check path between start and end (using AABB from @a node)
	 * 
	 * @param[in]     node               pointer to SceneNode to check (for disable collision with own child nodes)
	 * @param[in]     start              path start point
	 * @param[in]     end                path end point
	 * @param[in]     queryMask          mask of QueryFlags for filter collision objects
	 * @param[out]    collisionObjects   when not NULL used to return all objects with which we collided (function do not return after find first collision)
	 */
	inline static bool isFreePath(
		const Ogre::SceneNode* node,
		const Ogre::Vector3& start,
		const Ogre::Vector3& end,
		int queryMask,
		std::list<Ogre::MovableObject*>* collisionObjects = NULL
	) {
		Ogre::AxisAlignedBox aabb;
		getLocalAABB(node, &aabb);
		return isFreePath(node, aabb, start, end, queryMask, collisionObjects);
	}
	
private:
	static void _getLocalAABB(const Ogre::SceneNode* node, Ogre::Aabb* aabb, bool getScaled = false);
	static bool _intersects(
		const Ogre::AxisAlignedBox& aabb1,  const Ogre::AxisAlignedBox& aabb2,
		const Ogre::Matrix4&        from1,  const Ogre::Matrix4&        from2,
		const Ogre::Vector3&        scale1, const Ogre::Vector3&        scale2
		#ifdef MGE_DEBUG_INTERSECTS_VISUAL
		, const Ogre::SceneNode* node, const Ogre::Vector3& node1DerivedPosition, const Ogre::Quaternion& nodeDerivedOrientation, int callID
		#endif
	);
};

/// @}

}
