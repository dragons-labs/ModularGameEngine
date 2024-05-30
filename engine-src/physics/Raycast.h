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

#include "physics/Raycast_forward.h"

#include "rendering/CameraSystem.h"
#include "physics/utils/OgreRayCast.h"

#include "data/QueryFlags.h"
#include "physics/utils/WorldSizeInfo.h"

namespace MGE { struct BaseActor; }

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Finding objects and indicate point, area or polygonal-chain in game world with ray-casting
 * 
 * `#include <raycast.h>`
 */
namespace RayCast {
	using namespace MGE::OgreRayCast;
	
	
	/// struct for single entry in RayCast::Results lists
	struct ResultsEntry {
		/// constructor
		ResultsEntry(
			MGE::BaseActor* _gameObject, const Ogre::MovableObject* _ogreObject, const Ogre::Vector3& _hitPoint, Ogre::Real _distance2
		) : gameObject(_gameObject), ogreObject(_ogreObject), hitPoint(_hitPoint), distance2(_distance2) { }
		
		/// pointer to hit bullet object
		MGE::BaseActor*             gameObject;
		
		/// pointer to hit ogre object
		const Ogre::MovableObject*  ogreObject;
		
		/// world position of hit point
		Ogre::Vector3               hitPoint;
		
	private:
		friend struct Results;
		Ogre::Real                  distance2;
	};
	
	/// struct for describing raycasting results
	struct Results : public ResultsBase {
	public:
		/**
		 * @brief find objects attached to Ogre::MovableObject by Ogre::UserObjectBindings key (@a filterID) and put them to @a filteredList
		 *
		 * @param[in]  key            string for filter by getUserObjectBindings() key value
		 * @param[out] filteredList   std::set for put finding objects (received from getUserObjectBindings().getUserAny(@a filterID))
		 *
		 * @return true on success (find least one object), false otherwise
		 * 
		 * Example: \code{.cpp}res->findByType<MyObj*, MGE::BaseActor*>("so", listOfMyObj);\endcode
		 */
		template <typename ListType, typename AnyElementType = ListType>
		bool findByType(const std::string_view& key, std::set<ListType>& filteredList) {
			bool ret = false;
			
			std::list<ResultsEntry>::iterator hitObjectsIter;
			for (hitObjectsIter = hitObjects.begin(); hitObjectsIter != hitObjects.end(); ++hitObjectsIter) {
				if (hitObjectsIter->ogreObject) {
					const MGE::Any& tmpAny = MGE::Any::getFromBindings(hitObjectsIter->ogreObject, key);
					if (!tmpAny.isEmpty()) {
						filteredList.insert(MGE::Any::Cast<ListType, AnyElementType>::getValue(tmpAny));
						ret = true;
					}
				}
			}
			return ret;
		}
		
		/// true when we found ground object
		bool           hasGround;
		
		/// ground-contact position
		Ogre::Vector3  groundPoint;
		
		/// list of ogre (or bullet) objets fond via bullet and/or ogre ray as std::list of RayCast::ResultsEntry
		std::list<ResultsEntry> hitObjects;
		
		/// destructor
		virtual ~Results() { }
		
		/// constructor - we need ray used to performing search for sorting adding results (distance calculation, etc)
		Results() : hasGround(false) { }
		
		/// add single searching results from bullet object search
		virtual void addResult(const btCollisionObject* object, const Ogre::Vector3& pointOgre, const Ogre::Ray& ray) override;
		
		/// add single searching results from ogre object search
		virtual void addResult(const Ogre::MovableObject* object, const Ogre::Vector3& pointOgre, Ogre::Real distance) override;
		
		/// add multiple searching results from ogre object search
		virtual void addResult(Ogre::SceneQueryResult& queryResult) override;
		
	private:
		/// add MGE::BaseActor to hitObjects (if @a _gameObject == NULL then extract MGE::BaseActor from Ogre::MovableObject) and check ground
		/// @note for use in addResult() ONLY
		inline void addObject(
			const Ogre::MovableObject* _ogreObject, Ogre::Vector3 _hitPoint, Ogre::Real _distance2
		);
	};
	
	/// default iteration limit for some variants of @ref findFreePosition function
	extern int defaultIterationLimit;
	
	/**
	 * @brief search bullet and/xor ogre objects on ray and finish point - do real search
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  ray            ray (start point and direction vector) for searching
	 * @param[in]  rayTo          end point (must be on @a ray, when searching for ground must be under ground) for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  vertical       do vertical search (@a ray direction must be on Y axis)
	 * 
	 * @return shared_ptr to (new allocated) Results object
	 */
	inline ResultsPtr searchOnRay(
		Ogre::SceneManager* scnMgr,
		const Ogre::Ray& ray,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		bool vertical = false
	) {
		auto results = std::make_shared<Results>();
		MGE::OgreRayCast::searchOnRay(results.get(), scnMgr, ray, rayTo, searchMask, onlyFirst, vertical);
		return results;
	}
	
	/**
	 * @brief search on specific ray
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  ray            searching ray (defines the start point and direction of searching)
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  searchDistance length of searching ray (with @a ray defines the end point of searching), when < 0 use default value
	 * 
	 * @return shared pointer to RayCast::Results with search results
	 */
	inline ResultsPtr searchFromRay(
		Ogre::SceneManager* scnMgr,
		const Ogre::Ray& ray,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		Ogre::Real searchDistance = MGE::WorldSizeInfo::getRayLenght()
	) {
		return searchOnRay( scnMgr, ray, ray.getPoint(searchDistance), searchMask, onlyFirst );
	}
	
	/**
	 * @brief filtered search on specific ray
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  ray            searching ray (defines the start point and direction of searching)
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  searchDistance length of searching ray (with @a ray defines the end point of searching), when < 0 use default value
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> inline void searchFromRay(
		const std::string_view& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		const Ogre::Ray& ray,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		Ogre::Real searchDistance = MGE::WorldSizeInfo::getRayLenght()
	) {
		return MGE::OgreRayCast::searchOnRay<ListType,AnyElementType>( filterID, filteredList, scnMgr, ray, ray.getPoint(searchDistance), searchMask, onlyFirst );
	}
	
	/**
	 * @brief search on ray betwen two points
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  rayFrom        start point for searching
	 * @param[in]  rayTo          finish point for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 *
	 * @return shared pointer to RayCast::Results with search results
	 */
	inline ResultsPtr searchFromPoints(
		Ogre::SceneManager* scnMgr,
		const Ogre::Vector3& rayFrom,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false
	) {
		Ogre::Vector3 direction = rayTo - rayFrom;
		direction.normalise();
		return searchOnRay( scnMgr, Ogre::Ray(rayFrom, direction), rayTo, searchMask, onlyFirst );
	}
	
	/**
	 * @brief search on vertical ray to world point
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  x              x coordinate for vertical searching
	 * @param[in]  z              z coordinate for vertical searching
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  maxY           start y coordinate for searching, when is NaN use default value
	 * @param[in]  minY           finish y coordinate for searching, when is NaN use default value
	 *
	 * @return shared pointer to RayCast::Results with search results
	 */
	inline ResultsPtr searchVertical(
		Ogre::SceneManager* scnMgr,
		Ogre::Real x,
		Ogre::Real z,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		Ogre::Real maxY = MGE::WorldSizeInfo::getWorldMax().y,
		Ogre::Real minY = MGE::WorldSizeInfo::getWorldMin().y
	) {
		return searchOnRay(
			scnMgr,
			Ogre::Ray( Ogre::Vector3(x, maxY, z), Ogre::Vector3::NEGATIVE_UNIT_Y ),
			Ogre::Vector3(x, minY, z),
			searchMask,
			onlyFirst,
			true
		);
	}
	
	/**
	 * @brief filtered search on vertical ray to world point
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  x              x coordinate for vertical searching
	 * @param[in]  z              z coordinate for vertical searching
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  maxY           start y coordinate for searching, when is NaN use default value
	 * @param[in]  minY           finish y coordinate for searching, when is NaN use default value
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> inline void searchVertical(
		const std::string_view& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		Ogre::Real x,
		Ogre::Real z,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		Ogre::Real maxY = MGE::WorldSizeInfo::getWorldMax().y,
		Ogre::Real minY = MGE::WorldSizeInfo::getWorldMin().y
	) {
		return MGE::OgreRayCast::searchOnRay<ListType,AnyElementType>(
			filterID, filteredList,
			scnMgr,
			Ogre::Ray( Ogre::Vector3(x, maxY, z), Ogre::Vector3::NEGATIVE_UNIT_Y ),
			Ogre::Vector3(x, minY, z),
			searchMask,
			onlyFirst,
			true
		);
	}
	
	/**
	 * @brief search on ray from camera to screen point
	 * 
	 * find objects with point determinated by a world space ray as cast from the current camera
	 * through a viewport position by specified point
	 * 
	 * @param[in]  screenx        x coordinate of point
	 * @param[in]  screeny        y coordinate of point
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 *
	 * @return shared pointer to RayCast::Results with search results
	 */
	inline ResultsPtr searchFromCamera(
		Ogre::Real screenx,
		Ogre::Real screeny,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false
	) {
		auto cameraSystem = MGE::CameraSystem::getPtr();
		return searchFromRay( cameraSystem->getCurrentSceneManager(), cameraSystem->getCurrentCamera()->getCameraRay(screenx, screeny), searchMask, onlyFirst );
	}
	
	/**
	 * @brief filtered search on specific ray
	 * 
	 * find objects with point determinated by a world space ray as cast from the current camera
	 * through a viewport position by specified point
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  screenx        x coordinate of point
	 * @param[in]  screeny        y coordinate of point
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> inline void searchFromCamera(
		const std::string_view& filterID,
		std::set<ListType>& filteredList,
		Ogre::Real screenx,
		Ogre::Real screeny,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false
	) {
		auto cameraSystem = MGE::CameraSystem::getPtr();
		return searchFromRay<ListType,AnyElementType>(
			filterID, filteredList, cameraSystem->getCurrentSceneManager(), cameraSystem->getCurrentCamera()->getCameraRay(screenx, screeny), searchMask, onlyFirst
		);
	}
	
	/**
	 * @brief search ogre object in the rectangular area determinated by specified rays
	 * 
	 * We use ogre raycasting based on AABB for area selecting, because we have multiple time unselect/select functionality it is enough precise.
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  rays           vector of 4 rays determinated rectangular corners: left-top, right-top, left-bottom, right-bottom
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * 
	 * @return shared_ptr to (new allocated) Results object
	 */
	inline ResultsPtr searchOnArea(
		Ogre::SceneManager* scnMgr,
		const std::vector<Ogre::Ray>& rays,
		uint32_t searchMask=0xFFFFFFFF
	) {
		auto results = std::make_shared<Results>();
		MGE::OgreRayCast::searchOnArea(results.get(), scnMgr, rays, searchMask);
		return results;
	}
	
	/**
	 * @brief search ogre object in the spherical area determinated by radius and point
	 * 
	 * We use ogre raycasting based on AABB for area selecting.
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  radius         searching sphere radius
	 * @param[in]  point          searching sphere centre point
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default all objects
	 * 
	 * @return shared_ptr to (new allocated) Results object
	 */
	inline ResultsPtr searchOnRadius(
		Ogre::SceneManager* scnMgr,
		Ogre::Real radius,
		const Ogre::Vector3& point,
		uint32_t searchMask=0xFFFFFFFF
	) {
		auto results = std::make_shared<Results>();
		MGE::OgreRayCast::searchOnRadius(results.get(), scnMgr, radius, point, searchMask);
		return results;
	}
	
	/**
	 * @brief search for ground at @a point and get ground height (update y component in @a point)
	 * 
	 * @param[in]      scnMgr   pointer to SceneManager on which we do search
	 * @param[in,out]  point    point to get ground height
	 */
	static inline bool getGroundHeight( Ogre::SceneManager* scnMgr, Ogre::Vector3& point ) {
		auto res = MGE::RayCast::searchVertical(
			scnMgr, point.x, point.z, MGE::QueryFlags::GROUND, true
		);
		if (res->hasGround) {
			point.y = res->groundPoint.y;
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * @brief find free position for placing object near the point
	 * 
	 * @param[in]     node        pointer to SceneNode to check
	 * @param[in]     aabb        Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     searchMask  pecify mask for reduce searching to specific object types, see @ref MGE::QueryFlags, default COLLISION_OBJECT
	 */
	inline std::pair<bool, Ogre::Vector3> findFreePosition(
		const Ogre::SceneNode* node,
		const Ogre::AxisAlignedBox& aabb,
		uint32_t searchMask = MGE::QueryFlags::COLLISION_OBJECT
	) {
		Ogre::Vector3 aabbSize(aabb.getMaximum() - aabb.getMinimum());
		Ogre::Real step = ((aabbSize.x > aabbSize.z) ? aabbSize.z : aabbSize.x) / 3.0;
		return MGE::OgreRayCast::findFreePosition( node, aabb, searchMask, step, defaultIterationLimit );
	}
	
	/**
	 * @brief search bullet and/xor ogre objects on ray and finish point
	 * 
	 * @param[out] results        pointer to ResultsBase struct for returning search results
	 * @param[in]  ray            ray (start point and direction vector) for searching
	 * @param[in]  rayTo          end point (must be on @a ray, when searching for ground must be under ground) for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specific object types, see @ref MGE::QueryFlags
	 * @param[in]  onlyFirst      stop search after find first object
	 */
	void searchOnBulletRay(
		MGE::RayCast::ResultsBase* results,
		const Ogre::Ray& ray,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask,
		bool onlyFirst
	);
	
	/**
	 * @brief check if position of @a node is free (@a node do not collide with other scene elements)
	 * 
	 * @param[in]  position      position to check
	 * @param[in]  object        bullet object that need free space
	 * 
	 * @return true if position is free (or no bullet), false otherwise
	 */
	bool isFreeBulletPosition(
		const Ogre::Vector3& position,
		btCollisionObject* object
	);
}

/// @}

}
