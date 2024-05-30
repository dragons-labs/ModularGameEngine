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

Inspired by:
	→ "micropather" by Lee Thomason (http://grinninglizard.com/micropather/) (Zlib licensed)
*/

#pragma   once

#include "config.h"

#include "physics/Raycast.h"
#include "physics/utils/HexagonalGrid.h"

namespace MGE { struct World3DObject; }

#include <forward_list>

#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
#include "rendering/markers/VisualMarkers.h"
#endif

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Find path between two point in game world
 * 
 * See too: @ref PathFinding
 */
class PathFinder {
public:
	enum ReturnCodes : int16_t {
		OK                       = 1, //< generic OK info
		CAN_MOVE                 = OK | (1 << 1), //< can move
		PATH_OK                  = OK | (1 << 2), //< path is available
		NEED_RUN_TRIGGERS        = OK | (1 << 3), //< path have triggers to run (it may not be set)
		
		NOT_AVAILABLE            = static_cast<int16_t>(0x8000), //< path is not available (generic error)
		IS_NOT_MOVABLE           = NOT_AVAILABLE | (1 << 1), //< moving object is not movable
		GROUND_NOT_FOUND         = NOT_AVAILABLE | (1 << 2), //< don't found ground
		NO_FREE_SPACE_ON_TARGET  = NOT_AVAILABLE | (1 << 3), //< no free space on target position (sub-path check fail)
		TOO_MANY_STEPS           = NOT_AVAILABLE | (1 << 4), //< too many steps in path finder algorithm (only in debug mode)
		TOO_STEEPLY              = NOT_AVAILABLE | (1 << 5), //< too steeply
		ACTOR_COLLISION          = NOT_AVAILABLE | (1 << 6), //< collision with actor object
		OGRE_OBJECT_COLLISION    = NOT_AVAILABLE | (1 << 7), //< collision with ogre object (not being actor)
		OBJECT_COLLISION         = NOT_AVAILABLE | (1 << 8), //< collision with MGE::QueryFlags::COLLISION_OBJECT (actor or Ogre object, NOT trigger)
		TRIGGER_NO_ACCESS        = NOT_AVAILABLE | (1 << 9), //< trigger object do not allow crossing
	};
	
	/**
	 * @brief find path between two points
	 * 
	 * @param[in]  object            pointer to "3D World Interface" of the moving object
	 * @param[in]  src               start point
	 * @param[in]  dst               stop point
	 * @param[out] points            reference to std::list of points to write found path
	 * 
	 * @return if (value \< 0) error; if (value \> 0) success
	 *         full list of values see @ref ReturnCodes
	 */
	int16_t findPath(
		MGE::World3DObject* object,
		Ogre::Vector3 src, Ogre::Vector3 dst,
		std::list<Ogre::Vector3>& points
	);
	
	/**
	 * @brief find path between two points
	 * 
	 * @param[in]  object            pointer to "3D World Interface" of the moving object
	 * @param[in]  src               start point
	 * @param[in]  dst               stop point
	 * 
	 * @return pair of values:
	 *         @li first  - if (value \< 0) error; if (value \> 0) success
	 *                      full list of values see @ref ReturnCodes
	 *         @li second - found path points
	 */
	inline std::pair<int16_t, std::list<Ogre::Vector3>> findPath(
		MGE::World3DObject* object,
		Ogre::Vector3 src, Ogre::Vector3 dst
	) {
		std::list<Ogre::Vector3> ret_vec;
		auto ret_code = findPath( object, src, dst, ret_vec );
		return std::make_pair( ret_code, ret_vec );
	}
	
	/// iteration limit for findPath function (number open-nodes to check)
	static int iterationLimit;
	
	/// constructor
	PathFinder();
	
	/// destructor
	~PathFinder();
	
protected:
	/// struct represents node of path for A* solver
	struct PathNode {
		/// hexagonal grid node point representation
		MGE::HexagonalGridPoint point;
		
		/// current optimal path parrent of node
		PathNode* parent;
		
		/// set of child for whom this node was whenever parents
		std::set<PathNode*> childs;
		
		/// direction betwen parent and this node
		uint16_t direction;
		
		/// path cost from current parrent
		float costFromParent;
		
		/// path cost from start node
		float costFromStart;
		
		/// estimate path cost to finish node
		float estimateCostToEnd;
		
		/// 3D world groundHeight at node point
		float groundHeight;
		
		/// true when node is open (was not query about neighbors)
		bool isOpen;
		
		/// true need check accessibility from parrent
		bool needCheckFromParent;
		
		/// update costFromStart in child and re-put open child to openNodes with new estimate total cost
		void updateChilds(std::multimap<float, PathNode*>& openNodes, PathNode* changedParent);
		
		/// constructor
		PathNode(PathNode* p = NULL, uint16_t d = 0, float c1 = 0, float c2 = 0, float c3 = 0, bool needCheck = false) :
			parent (p), direction (d), costFromParent (c1), costFromStart (c2), estimateCostToEnd (c3), isOpen (true), needCheckFromParent (needCheck) { }
	};
	
	/// function to calculate minimal path cost (distance) betwen two nodes
	float costEstimate(
		MGE::HexagonalGridPoint stateStart,
		MGE::HexagonalGridPoint stateEnd
	) {
		Ogre::Vector3 d = stateEnd.toOgre() - stateStart.toOgre();
		return d.length();
	}
	
	/// check possibility to move from @a currPoint to @a newPoint
	bool canMove(MGE::World3DObject* object, const Ogre::Vector3& currPoint, Ogre::Vector3& newPoint, float& costFromParent);
	
	/// used internally in findPath() when path is found
	void onFoundPath(PathNode* currNode, std::list<Ogre::Vector3>& points, MGE::World3DObject* object);
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
	/// list of nodes used to show visual path
	std::list<Ogre::SceneNode*> visualPath;
	
	/// clear visual path (remove OBBoxRenderable and nodes)
	void clearVisualPath();
	
	//@{
	/// show path using OBBoxRenderable
	void showPath(
		std::list<Ogre::Vector3> points,
		MGE::World3DObject* object,
		const Ogre::ColourValue& colour = Ogre::ColourValue(0, 0.95, 0),
		int mode = 0
	);
	void showPath(
		PathNode* pathNode,
		MGE::World3DObject* object,
		const Ogre::ColourValue& colour = Ogre::ColourValue(0.95, 0, 0),
		int mode = 0
	);
	//@}
	#endif
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	enum MarkerTypes {
		PARENT, CHILD_OK, CHILD_FORBIDDEN, CHILD_FORBIDDEN2
	};
	struct MarkedPoint {
		Ogre::Vector3 point;
		Ogre::Vector3 fromPoint;
		int type;
		Ogre::MovableObject* collison;
		Ogre::SceneNode* node;
		
		MarkedPoint(const Ogre::Vector3& p, const Ogre::Vector3& fp, int t, Ogre::MovableObject* c = NULL);
		
		~MarkedPoint();
	};
	
	/// list of points with marker type used to show visual grid
	std::list<MarkedPoint> visualGrid;
	std::list<MarkedPoint>::iterator visualGridIter;
	
	/// collision marker visual settings
	MGE::VisualMarkerSettingsSet markerSettings;
	
	void addGridNode(const Ogre::Vector3& point, int type, const Ogre::Vector3& fromPoint = Ogre::Vector3::ZERO, Ogre::MovableObject* collison = NULL);
	
	/// clear and (re)init visual grid
	void reinitVisualGrid();
	
	/// show single points from @ref visualGrid, by iterator
	void showNextGridPoint(const std::list<MarkedPoint>::iterator& iter);
	
public:
	bool readyToRemove;
	
	/// show points from @ref visualGrid
	void showNextGridPoints(int count);
	#endif
};

/// @}

}
