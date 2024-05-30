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
#include "config.h"

#include "data/property/Any.h"
#include "LogSystem.h"

#include <OgreRay.h>
#include <OgreSceneQuery.h>

class btCollisionObject;

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Finding objects and indicate point, area or polygonal-chain in game world with ray-casting
 * 
 * `#include <ogreRayCast.h>`
 */
namespace OgreRayCast {
	/// base (interface) struct for describing raycasting results
	struct ResultsBase {
		/// add single searching results from bullet object search
		virtual void addResult(const btCollisionObject* object, const Ogre::Vector3& pointOgre, const Ogre::Ray& ray);
		
		/// add single searching results from ogre object search
		virtual void addResult(const Ogre::MovableObject* object, const Ogre::Vector3& pointOgre, Ogre::Real distance) = 0;
		
		/// add multiple searching results from ogre object search
		virtual void addResult(Ogre::SceneQueryResult& queryResult) = 0;
		
		/// destructor
		virtual ~ResultsBase() {}
	};
	
	/**
	 * @name Elementary Ogre Raycast search functions
	 * 
	 * @{
	 */
	
	/**
	 * @brief search bullet and/xor ogre objects on ray and finish point
	 * 
	 * @param[out] results        pointer to ResultsBase struct for returning search results
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  ray            ray (start point and direction vector) for searching
	 * @param[in]  rayTo          end point (must be on @a ray, when searching for ground must be under ground) for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  vertical       do vertical search (@a ray direction must be on Y axis)
	 */
	void searchOnRay(
		ResultsBase* results,
		Ogre::SceneManager* scnMgr,
		const Ogre::Ray& ray,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask,
		bool onlyFirst,
		bool vertical
	);
	
	/**
	 * @brief search ogre object in the spherical area determinated by radius and point
	 * 
	 * We use ogre raycasting based on AABB for area selecting.
	 * 
	 * @param[out] results        pointer to ResultsBase struct for returning search results
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  radius         searching sphere radius
	 * @param[in]  point          searching sphere centre point
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags
	 */
	void searchOnRadius(
		ResultsBase* results,
		Ogre::SceneManager* scnMgr,
		float radius,
		const Ogre::Vector3& point,
		uint32_t searchMask
	);
	
	/**
	 * @brief search ogre object in the rectangular area determinated by specified rays
	 * 
	 * We use ogre raycasting based on AABB for area selecting, because we have multiple time unselect/select functionality it is enough precise.
	 * 
	 * @param[out] results        pointer to ResultsBase struct for returning search results
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  rays           vector of 4 rays determinated rectangular corners: left-top, right-top, left-bottom, right-bottom
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags
	 */
	void searchOnArea(
		ResultsBase* results,
		Ogre::SceneManager* scnMgr,
		const std::vector<Ogre::Ray>& rays,
		uint32_t searchMask
	);
	
	/**
	 * @brief find free position for placing object near the point
	 * 
	 * @param[in]     node        pointer to SceneNode to check
	 * @param[in]     aabb        Axis Aligned Bounding Box in LOCAL @a node space to check collisions
	 * @param[in]     queryMask   mask of QueryFlags for filter colision objects
	 * @param[in]     step        grid size for searching
	 * @param[in]     count       max number of iteration in grid
	 */
	std::pair<bool, Ogre::Vector3> findFreePosition(
		const Ogre::SceneNode* node,
		const Ogre::AxisAlignedBox& aabb,
		int queryMask,
		Ogre::Real step,
		int count
	);
	
	/**
	 * @}
	 */
	
	/// struct for describing filtered search results
	template <typename ListType, typename AnyElementType = ListType> class ResultsWithFilter : public ResultsBase {
	protected:
		friend class System;
		
		/// constructor
		ResultsWithFilter(const std::string& _filterID, std::set<ListType>& _filteredList) :
			filteredList(_filteredList), filterID(_filterID) { }
		
		/// add single searching results from ogre object search
		virtual void addResult(const Ogre::MovableObject* object, const Ogre::Vector3& pointOgre, Ogre::Real distance) {
			addToFilteredList( object );
		}
		
		/// add multiple searching results from ogre object search
		virtual void addResult(Ogre::SceneQueryResult& queryResult) {
			for(auto& iter : queryResult.movables) {
				addToFilteredList( iter );
			}
		}
		
	private:
		/// filter string
		Ogre::String filterID;
		
		/// set of elements matching the filter
		std::set<ListType>& filteredList;
		
		/// add elements extracted from @a object to @ref filteredList only when matching the filter
		inline void addToFilteredList(Ogre::MovableObject* object) {
			const MGE::Any& tmpAny = MGE::Any::getFromBindings(object, filterID);
			if (!tmpAny.isEmpty()) {
				filteredList.insert(MGE::Any::Cast<ListType, AnyElementType>::getValue(tmpAny));
			}
		}
	};
	
	/**
	 * @name Ogre Raycast search functions used ResultsWithFilter
	 * 
	 * @{
	 */
	
	/**
	 * @brief filtered search bullet and/xor ogre objects on ray and finish point - do real search
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  ray            ray (start point and direction vector) for searching
	 * @param[in]  rayTo          end point (must be on @a ray, when searching for ground must be under ground) for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find first object
	 * @param[in]  vertical       do vertical search (@a ray direction must be on Y axis)
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> void searchOnRay(
		const std::string& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		const Ogre::Ray& ray,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false,
		bool vertical = false
	) {
		auto results = new ResultsWithFilter<ListType,AnyElementType>(filterID, filteredList);
		searchOnRay(results, scnMgr, ray, rayTo, searchMask, onlyFirst, vertical);
		delete results;
	}
	
	/**
	 * @brief filtered search ogre object in the spherical area determinated by radius and point
	 * 
	 * We use ogre raycasting based on AABB for area selecting.
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  radius         searching sphere radius
	 * @param[in]  point          searching sphere centre point
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> void searchOnRadius(
		const std::string& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		Ogre::Real radius,
		const Ogre::Vector3& point,
		uint32_t searchMask
	) {
		auto results = new ResultsWithFilter<ListType,AnyElementType>(filterID, filteredList);
		searchOnRadius(results, scnMgr, radius, point, searchMask);
		delete results;
	}
	
	/**
	 * @brief filtered search ogre object in the rectangular area determinated by specified rays
	 * 
	 * We use ogre raycasting based on AABB for area selecting, because we have multiple time unselect/select functionality it is enough precise.
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  rays           vector of 4 rays determinated rectangular corners: left-top, right-top, left-bottom, right-bottom
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> void searchOnArea(
		const std::string& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		const std::vector<Ogre::Ray>& rays,
		uint32_t searchMask
	) {
		auto results = new ResultsWithFilter<ListType,AnyElementType>(filterID, filteredList);
		searchOnArea(results, scnMgr, rays, searchMask);
		delete results;
	}
	
	
	/**
	 * @brief filtered search on ray betwen two points
	 * 
	 * @param[in]  filterID       string for filter by getUserObjectBindings() key value
	 * @param[out] filteredList   std::set for put results (objects received from getUserObjectBindings().getUserAny(@a filterID) of searched object)
	 * 
	 * @param[in]  scnMgr         pointer to SceneManager on which we do search
	 * @param[in]  rayFrom        start point for searching
	 * @param[in]  rayTo          finish point for searching
	 * @param[in]  searchMask     specify mask for reduce searching to specyfic object types, see @ref MGE::QueryFlags, default all objects
	 * @param[in]  onlyFirst      stop search after find fistr object
	 * 
	 * @tparam ListType           type of @a filteredList std:set elements (by default autodetected)
	 * @tparam AnyElementType     use when type of @a filteredList is diffrent from type of getUserObjectBindings().getUserAny(@a filterID)
	 */
	template <typename ListType, typename AnyElementType = ListType> inline void searchFromPoints(
		const std::string& filterID,
		std::set<ListType>& filteredList,
		Ogre::SceneManager* scnMgr,
		const Ogre::Vector3& rayFrom,
		const Ogre::Vector3& rayTo,
		uint32_t searchMask=0xFFFFFFFF,
		bool onlyFirst = false
	) {
		Ogre::Vector3 direction = rayTo - rayFrom;
		direction.normalise();
		return searchOnRay<ListType,AnyElementType>( filterID, filteredList, scnMgr, Ogre::Ray(rayFrom, direction), rayTo, searchMask, onlyFirst );
	}
	
	/**
	 * @}
	 */
}

/// @}

}
