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

#include <OgreRay.h>
#include <OgreEntity.h>
#include <OgreItem.h>
#include <OgreSceneNode.h>

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Raycasting to the polygon level
 */
class OgreMeshRaycast {
public:
	/**
	 * @brief struct used to return hit test results
	 */
	struct Results {
		/// index in indices vector for first vertex of hit triangle
		int            index;
		/// ray distance to hit point
		Ogre::Real     distance;
		/// hit point in vertices transform space (node LOCAL or WORLD)
		Ogre::Vector3  hitPoint;
		
		/// constructor
		Results() : index(-1), distance(0) {}
	};
	
	
	/**
	 * @brief get information from mesh
	 * 
	 * @param mo              ogre movable object to get mesh
	 * @param vertices        pointer to vector to put vertices from mesh (no null)
	 * @param indices         pointer to vector to put indices from mesh (no null)
	 * @param UVs             pointer to vector to put UVs from mesh (can be null)
	 * @param applyTransform  when true apply transform from parent node of @a entity
	 */
	static inline void getMeshInformation(
		Ogre::MovableObject* mo,
		std::vector<Ogre::Vector3>* vertices,
		std::vector<int>* indices,
		std::vector<Ogre::Vector2>* UVs = NULL,
		bool applyTransform = false
	) {
		if (mo->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			getMeshInformationV2(static_cast<Ogre::Item*>(mo), vertices, indices, UVs, applyTransform);
		} else if (mo->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			getMeshInformationV1(static_cast<Ogre::v1::Entity*>(mo), vertices, indices, UVs, applyTransform);
		}
	}
	
	/**
	 * @brief get information from v1 mesh
	 * 
	 * @param entity          entity to get mesh
	 * @param vertices        pointer to vector to put vertices from mesh (no null)
	 * @param indices         pointer to vector to put indices from mesh (no null)
	 * @param UVs             pointer to vector to put UVs from mesh (can be null)
	 * @param applyTransform  when true apply transform from parent node of @a entity
	 */
	static void getMeshInformationV1(
		const Ogre::v1::Entity* entity,
		std::vector<Ogre::Vector3>* vertices,
		std::vector<int>* indices,
		std::vector<Ogre::Vector2>* UVs = NULL,
		bool applyTransform = false
	);
	
	/**
	 * @brief get information from v2 mesh
	 * 
	 * @param item            item to get mesh
	 * @param vertices        pointer to vector to put vertices from mesh (no null)
	 * @param indices         pointer to vector to put indices from mesh (no null)
	 * @param UVs             pointer to vector to put UVs from mesh (can be null)
	 * @param applyTransform  when true apply transform from parent node of @a entity
	 */
	static void getMeshInformationV2(
		const Ogre::Item* item,
		std::vector<Ogre::Vector3>* vertices,
		std::vector<int>* indices,
		std::vector<Ogre::Vector2>* UVs = NULL,
		bool applyTransform = false
	);
	
	/**
	 * @brief get information from v2 mesh
	 * 
	 * @param billboardSet    billboard set to get vertices, indices, and UVs
	 * @param vertices        pointer to vector to put vertices from mesh (can be null)
	 * @param indices         pointer to vector to put indices from mesh (can be null)
	 * @param UVs             pointer to vector to put UVs from mesh (can be null)
	 * @param offset          billboard position relative to parent scene node (offset from scene node)
	 */
	static void getBillboardInformation(
		const Ogre::v1::BillboardSet* billboardSet,
		std::vector<Ogre::Vector3>* vertices,
		std::vector<int>* indices = NULL,
		std::vector<Ogre::Vector2>* UVs = NULL,
		const Ogre::Vector3& offset = Ogre::Vector3::ZERO
	);
	
	/**
	 * @brief do polygon level raycast test, version for mesh info with apply node transform
	 * 
	 * @param mouseRay        ray used to doing test
	 * @param vertices        vector of vertices from mesh
	 * @param indices         vector of indices from mesh
	 * @param positiveSide    when true accept hit to positive (front) side of triangle
	 * @param negativeSide    when true accept hit to negative (back) side of triangle
	 * 
	 * @note hitPoint in return @ref Results will be in WORLD transform space
	 */
	static Results entityHitTest(
		Ogre::Ray mouseRay,
		const std::vector<Ogre::Vector3>& vertices,
		const std::vector<int>& indices,
		bool positiveSide,
		bool negativeSide
	);
	
	/**
	 * @brief do polygon level raycast test, version for mesh info with not apply node transform
	 * 
	 * @note after modify MovableObject transform relative to parent SceneNode must (re)call getMeshInformation()
	 * 
	 * @param mouseRay        ray used to doing test
	 * @param toWorld         transform matrix from parent node of testing mesh to WORLD space
	 * @param mo              ogre movable object owned tested mesh
	 * @param vertices        vector of vertices from mesh
	 * @param indices         vector of indices from mesh
	 * @param positiveSide    when true accept hit to positive (front) side of triangle
	 * @param negativeSide    when true accept hit to negative (back) side of triangle
	 * 
	 * @note hitPoint in return @ref Results will be in LOCAL transform space
	 */
	static Results entityHitTest(
		Ogre::Ray mouseRay,
		const Ogre::Matrix4& toWorld,
		Ogre::MovableObject* mo,
		const std::vector<Ogre::Vector3>& vertices,
		const std::vector<int>& indices,
		bool positiveSide,
		bool negativeSide
	);
	
	/**
	 * @brief do polygon level raycast test
	 * 
	 * @param mouseRay        ray used to doing test
	 * @param mo              ogre movable object owned tested mesh
	 * @param vertices        vector of vertices from mesh
	 * @param indices         vector of indices from mesh
	 * @param positiveSide    when true accept hit to positive (front) side of triangle
	 * @param negativeSide    when true accept hit to negative (back) side of triangle
	 * @param verticesInLocal when true vertices is not transform to WORLD space
	 *                        (and hitPoint in return @ref Results will be in LOCAL transform space)
	 *                        otherwise is in WORLD space (and hitPoint will be in WORLD transform space too)
	 */
	static inline Results entityHitTest(
		Ogre::Ray mouseRay,
		Ogre::MovableObject* mo,
		const std::vector<Ogre::Vector3>& vertices,
		const std::vector<int>& indices,
		bool positiveSide,
		bool negativeSide,
		bool verticesInLocal
	) {
		if (verticesInLocal) {
			Ogre::Matrix4 toWorld = mo->getParentSceneNode()->_getFullTransform();
			return MGE::OgreMeshRaycast::entityHitTest(
				mouseRay, toWorld, mo, vertices, indices, positiveSide, negativeSide
			);
		} else {
			return MGE::OgreMeshRaycast::entityHitTest(
				mouseRay, vertices, indices, positiveSide, negativeSide
			);
		}
	}
	
	/**
	 * @brief get texture point based on results of entityHitTest()
	 * 
	 * @param hitTest         results of @ref entityHitTest
	 * @param vertices        vector of vertices from mesh
	 * @param indices         vector of indices from mesh
	 * @param UVs             vector of UVs from mesh
	 */
	static Ogre::Vector2 getTexturePoint(
		const Results& hitTest,
		const std::vector<Ogre::Vector3>& vertices,
		const std::vector<int>& indices,
		const std::vector<Ogre::Vector2>& UVs
	);
};

/// @}

}
