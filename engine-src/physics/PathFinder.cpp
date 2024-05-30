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

#include "physics/PathFinder.h"
#include "data/structs/components/3DWorld.h"

#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
	#include "rendering/markers/VisualMarkers.h"
	#include "data/utils/OgreUtils.h"
#endif

#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	#include <OgreItem.h>
	#include <OgreMeshManager2.h>
	#include "rendering/markers/Shapes.h"
	#include "rendering/utils/RenderQueueGroups.h"
	#include "data/LoadingSystem.h"
	#include "data/utils/NamedSceneNodes.h"
	#include "data/utils/OgreUtils.h"
#endif

#ifdef MGE_DEBUG_PATHFINDER2
	#define MGE_DEBUG_PATHFINDER2_LOG_STREAM(a)  LOG_VERBOSE(a);
#else
	#define MGE_DEBUG_PATHFINDER2_LOG_STREAM(a)
#endif

#ifdef MGE_DEBUG_PATHFINDER3
	#define MGE_DEBUG_PATHFINDER3_LOG_STREAM(a)  LOG_VERBOSE(a);
#else
	#define MGE_DEBUG_PATHFINDER3_LOG_STREAM(a)
#endif

void MGE::PathFinder::onFoundPath(PathNode* currNode, std::list<Ogre::Vector3>& points, MGE::World3DObject* object) {
	std::list<float> costs;
	float currStepCost = 0;
	uint16_t currDir, prevDir;
	Ogre::Vector3 turnStart, turnStop;
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
	clearVisualPath();
	showPath(currNode, object);
	#endif
	
	MGE_DEBUG_PATHFINDER2_LOG_STREAM(" - this is end node! (reverse) path is:");
	MGE_DEBUG_PATHFINDER2_LOG_STREAM("   - " << currNode->point << " - finish point");
	
	// clear points and add finish point
	points.clear();
	points.push_front( currNode->point.toOgre() );
	
	prevDir  = currNode->direction;
	currNode = currNode->parent;
	
	while (currNode->parent != NULL) {
		MGE_DEBUG_PATHFINDER2_LOG_STREAM(
			"   - " << currNode->point << "   dir=0x" << std::hex << currNode->direction
		);
		currStepCost += currNode->costFromParent;
		
		// adding to points nodes where direction is changed
		currDir = currNode->direction;
		if (prevDir != currDir) {
			turnStart   = currNode->point.toOgre();
			turnStart.y = currNode->groundHeight; // point.toOgre() has only x,z ... ground must be get from groundHeight
			
			MGE_DEBUG_PATHFINDER2_LOG_STREAM(
				"     >>> direcion change - prevDir=0x" << std::hex << prevDir << 
				" currDir=0x" << std::hex << currDir << " dst=" << points.front() << " turnStart=" << turnStart
			);
			points.push_front( turnStart );
			costs.push_front( currStepCost );
			currStepCost = 0;
			
			prevDir = currDir;
		}
		
		currNode = currNode->parent;
	}
	costs.push_front( currStepCost );
	
	// add initial (first turn)
	MGE_DEBUG_PATHFINDER2_LOG_STREAM("   - " << currNode->point << " - start point");
	MGE_DEBUG_PATHFINDER2_LOG_STREAM("     >>> initial direcion change - dst=" << points.front());
	points.push_front( currNode->point.toOgre() );
	
	// remove unneeded turns
	std::list<Ogre::Vector3>::iterator src, turn, dst;
	dst  = points.begin();
	src  = dst;
	turn = ++dst;
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
	showPath(points, object, Ogre::ColourValue(0, 0.95, 0), 1);
	#endif
	
	#ifdef MGE_DEBUG_PATHFINDER1
	LOG_VERBOSE(" - not cleaned path is:");
	for (src = points.begin(); src != points.end(); ++src) {
		LOG_VERBOSE("   - " << *src);
	}
	LOG_VERBOSE(" - cleaning path:");
	#endif
	
	auto costToNext = costs.begin();
	float fullCost = *(costToNext++); // cost of src --> turn
	while (++dst != points.end()) {
		float newCost = 1, newLen, tmp1;
		float costFromTurn = *(costToNext++); // cost of turn --> dst
		fullCost += costFromTurn;
		
		// when we can go directly from "src" to "dst", we don't need "turn"
		if ( object->canMove(*src, *dst, newCost, newLen, tmp1) > 0 ) {
			newLen  = Ogre::Math::Sqrt(newLen);
			newCost = newLen / newCost;
			if (newCost *.9 < fullCost) {
				#ifdef MGE_DEBUG_PATHFINDER1
				LOG_VERBOSE("   - remove turn - turn=" << *turn);
				#endif
				points.erase(turn);
				fullCost = newCost;
			} else {
				src = turn;
				fullCost = costFromTurn;
			}
		} else {
			src = turn;
			fullCost = costFromTurn;
		}
		turn = dst;
	}
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
	showPath(points, object, Ogre::ColourValue(0, 0.95, 0.95), 0);
	#endif
	
	#ifdef MGE_DEBUG_PATHFINDER1
	LOG_VERBOSE(" - final path is:");
	for (src = points.begin(); src != points.end(); ++src) {
		LOG_VERBOSE("   - " << *src);
	}
	#endif
}

bool MGE::PathFinder::canMove(MGE::World3DObject* object, const Ogre::Vector3& currPoint, Ogre::Vector3& newPoint, float& costFromParent) {
	if (! MGE::RayCast::getGroundHeight( object->getOgreSceneNode()->getCreator(), newPoint )) {
		MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - can't move from " << currPoint << " to " << newPoint << " not found ground - out of map ?");
		#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
		addGridNode(newPoint, CHILD_FORBIDDEN2);
		#endif
		return false;
	}
	
	float tmp1, tmp2;
	Ogre::MovableObject* collison;
	int16_t retCode = object->canMove(currPoint, newPoint, costFromParent, tmp1, tmp2, NULL, &collison);
	if (retCode & NOT_AVAILABLE) {
		MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - can't move from " << currPoint << " to " << newPoint << " retCode=" << std::hex << std::showbase << retCode);
		#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
		addGridNode(newPoint, CHILD_FORBIDDEN, currPoint, collison);
		#endif
		return false;
	}
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	addGridNode(newPoint, CHILD_OK, currPoint);
	#endif
	
	return true;
}

#define OpenNodesInsert(node)         openNodes.insert(std::make_pair(node->estimateCostToEnd + (node->needCheckFromParent ? node->costFromStart : 0), node))
#define AllGridPointNodesInsert(node) allGridPointNodes.insert(std::make_pair(node->point, node))
#define AllNodesInsert(node)          allNodes.insert(std::make_pair(std::make_pair(node->point, node->parent ? node->parent->point : node->point), node))

int MGE::PathFinder::iterationLimit = 1000;

int16_t MGE::PathFinder::findPath(
	MGE::World3DObject* object,
	Ogre::Vector3 start, Ogre::Vector3 finish,
	std::list<Ogre::Vector3>& points
) {
	/** @todo TODO.8: 3D world path finding can be slow ... maybe we should use two pathfinders:
	 *                  1) based on 2D image (like minimap, but numeric map of area type, e.g. 0 = forbidden, 1 = deep water, 2 = ..., 99 = road, ... )
	 *                     and (sub)type of moving object (ground vehicle, boat, person, ...) / mapping area type => speed from this object prototype
	 *                  2) this 3D world pathfinder
	 *                in initMove() we can:
	 *                  1. use 2D pathfinder for fast search primary path (if current map has defined 2D image for pathfinder)
	 *                  2. start moving
	 *                  3. rechecking this path (during moving) by 3D world pathfinder
	 *                  4. when it's not crossable – stop move, back to point 1 for search other path
	 */
	
	int16_t retCode = NOT_AVAILABLE;
	int loopCounter = iterationLimit;
	PathNode* newNode, *currNode;
	Ogre::Vector3 newPoint, currPoint, parentPoint;
	float turnCost = 2 * MGE::HexagonalGridPoint::distanceY;
	std::map<std::pair<MGE::HexagonalGridPoint, MGE::HexagonalGridPoint>, PathNode*> allNodes;
	std::map<MGE::HexagonalGridPoint, PathNode*> allGridPointNodes;
	std::multimap<float, PathNode*> openNodes;
	
	LOG_INFO("findPath from " << start << " to " << finish << " with gridSize=" << MGE::HexagonalGridPoint::distanceY << " and iterationLimit=" << loopCounter);
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	reinitVisualGrid();
	#endif
	
	/// @todo TODO.8: check availability of target for fast response (with NO_FREE_SPACE_ON_TARGET) when target is invalid
	
	// create finish grid point
	MGE::HexagonalGridPoint endGridPoint(finish);

	// create start node
	newNode = new PathNode();
	newNode->point.fromOgre(start);
	newNode->groundHeight = start.y;
	newNode->direction = newNode->point.getDirection(object->getWorldDirection());
	newNode->estimateCostToEnd = costEstimate(newNode->point, endGridPoint);
	AllNodesInsert(newNode);
	AllGridPointNodesInsert(newNode);
	OpenNodesInsert(newNode);
	
	// end when start and finish point is in this same grig point
	if (newNode->point == endGridPoint) {
		points.clear();
		points.push_back( start );
		points.push_back( finish );
		return PATH_OK;
	}
	
	// find path
	for (auto openNodesIter = openNodes.begin(); openNodesIter != openNodes.end(); openNodesIter = openNodes.begin()) {
		currNode = openNodesIter->second;
		openNodes.erase(openNodesIter);
		
		if (!currNode->isOpen) { // we can have duplicates in openNodes - on upgrade we don't remove old entry
			continue;
		}
		
		currPoint = currNode->point.toOgre();
		currPoint.y = currNode->groundHeight;
		
		if (currNode->needCheckFromParent) { // this is not executed for root node, so currNode->parent is not NULL here
			parentPoint   = currNode->parent->point.toOgre();
			parentPoint.y = currNode->parent->groundHeight;
			if (! canMove(object, parentPoint, currPoint, currNode->costFromParent) ) {
				if (currNode->parent == NULL/* || currNode->parent->parent == NULL*/) {
					// allow forbidden move from start point but with higher cost
					currNode->costFromParent *= 10;
				} else {
					currNode->parent->childs.erase(currNode);
					continue;
				}
			}
			// update costFromStart due to canMove() can update costFromParent
			currNode->costFromStart = currNode->costFromStart + currNode->costFromParent;
			// update ground height
			currNode->groundHeight  = currPoint.y;
		}
		
		currNode->isOpen = false;
		
		if (--loopCounter < 0) {
			retCode = TOO_MANY_STEPS;
			LOG_INFO("Too many iteration in pathfinder, break");
			break;
		}
		
		#ifdef MGE_DEBUG_PATHFINDER3
		if (currNode->parent)
			LOG_VERBOSE("Analyze node " << currNode->point <<
				" with " << currNode->childs.size() << " childs"
				" from parent " << currNode->parent->point <<
				" costFromStart=" << currNode->costFromStart <<
				" estimateCostToEnd=" << currNode->estimateCostToEnd);
		else
			LOG_VERBOSE("Analyze start node " << currNode->point <<
				" estimateCostToEnd=" << currNode->estimateCostToEnd);
		#endif
		
		// if reached the finish node
		if (currNode->point == endGridPoint) {
			onFoundPath(currNode, points, object);
			retCode = PATH_OK;
			break;
		}
		
		#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
		addGridNode(currPoint, PARENT);
		#endif
		
		// process neighbors of current node
		int bIndex = currNode->point.getBIndex();
		for (int i=0; i<currNode->point.getNeighborCount(); ++i) { // we don't run multi-thread here, because whole pathfinding is running in separate thread
			MGE::HexagonalGridPoint nextGridPoint = currNode->point.getNeighbor(i, bIndex);
			
			if (currNode->parent && currNode->parent->point == nextGridPoint) {
				continue; // don't back to grand parent
			}
			
			// calculate direction betwen currNode->point and nextGridPoint
			uint16_t newDir = currNode->point.getDirection(nextGridPoint);
			
			// calculate cost from parrent with turn cost
			float costFromParent = currNode->point.getNeighborCost( currNode->point.getNeighborMode(i) );
			if (newDir != currNode->direction)
				costFromParent += turnCost;
			
			// calculate cost from start (using updated costFromParent)
			float costFromStart  = currNode->costFromStart + costFromParent;
			
			// calculate estimate cost to end
			float estimateCostToEnd = costEstimate(nextGridPoint, endGridPoint);
			
			// find on allNodes
			auto allNodesIter = allNodes.find(std::make_pair(nextGridPoint, currNode->point));
			if (allNodesIter != allNodes.end()) {
				newNode = allNodesIter->second;
				MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - found node " << nextGridPoint << "/parent=" << currNode->point <<
					" on allNodes as " << newNode->point << "/parent=" << (newNode->parent ? newNode->parent->point : newNode->point)
				);
			} else {
				newNode = NULL;
			}
			
			bool needCheckFromParent = true;
			if (estimateCostToEnd < currNode->estimateCostToEnd) {
				newPoint = nextGridPoint.toOgre();
				if (! canMove(object, currPoint, newPoint, costFromParent) ) {
					if (currNode->parent == NULL/* || currNode->parent->parent == NULL*/) {
						// allow forbidden move from start point but with higher cost
						currNode->costFromParent *= 10;
					} else {
						continue;
					}
				}
				needCheckFromParent = false;
				
				// try find node in allGridPointNodes
				auto iter = allGridPointNodes.find(nextGridPoint);
				if (iter != allGridPointNodes.end()) {
					MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - found node " << nextGridPoint <<
						" on allGridPointNodes as " << iter->second->point <<
						" the same as on allNodes: " << bool(newNode == 0 || iter->second == newNode)
					);
					newNode = iter->second;
					
					// update nodes with lower path cost
					if (costFromStart < newNode->costFromStart) {
						MGE_DEBUG_PATHFINDER3_LOG_STREAM("   - update " << newNode->point <<
							" costFromStart.OLD=" << newNode->costFromStart << " costFromStart.NEW=" << costFromStart <<
							" parent.OLD=" << newNode->parent->point
						);
						
						newNode->parent         = currNode;
						newNode->direction      = newDir;
						newNode->costFromParent = costFromParent;
						newNode->costFromStart  = costFromStart;
						if (!newNode->isOpen) {
							newNode->updateChilds(openNodes, currNode);
						}
						currNode->childs.insert(newNode);
					}
				} else if (!newNode) {
					MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - create new (open) node " << nextGridPoint << " after check from parent");
					newNode = new PathNode(currNode, newDir, costFromParent, costFromStart, estimateCostToEnd, needCheckFromParent);
					newNode->point = nextGridPoint;
					newNode->groundHeight = newPoint.y;
					
					// add to all nodes map ... this must be after set newNode->point value
					AllNodesInsert(newNode);
					
					// add to parent
					currNode->childs.insert(newNode);
					
					// set as GridPointNode
					AllGridPointNodesInsert(newNode);
				
					// add to open nodes
					OpenNodesInsert(newNode);
				}
			} else if (!newNode) {
				MGE_DEBUG_PATHFINDER3_LOG_STREAM(" - create new (open) node " << nextGridPoint << " without check from parent");
				newNode = new PathNode(currNode, newDir, costFromParent, costFromStart, estimateCostToEnd, needCheckFromParent);
				newNode->point = nextGridPoint;
				
				// add to all nodes map ... this must be after set newNode->point value
				AllNodesInsert(newNode);
				
				// add to parent
				currNode->childs.insert(newNode);
				
				// add to open nodes
				OpenNodesInsert(newNode);
			}
		}
	}
	
	for (auto& iter : allNodes) {
		delete iter.second;
	}
	
	LOG_INFO("findPath end with code: " << std::hex << std::showbase << retCode << " after "<< std::dec << std::noshowbase << (iterationLimit - loopCounter - 1) << " iterations");
	return retCode;
}

void MGE::PathFinder::PathNode::updateChilds(std::multimap<float, PathNode*>& openNodes, PathNode* changedParent) {
	if (parent != changedParent) // information in childs can be outdated, so we check if change relates to the actual parent
		return;
	
	for (auto& iter : childs) {
		iter->costFromStart = iter->parent->costFromStart + iter->costFromParent;
		
		if (iter->isOpen)
			// update open node position in openNodes
			OpenNodesInsert(iter);
		else
			iter->updateChilds(openNodes, this);
	}
}

#ifdef MGE_DEBUG_PATHFINDER_VISUAL_PATH
void MGE::PathFinder::clearVisualPath() {
	for (auto& node : visualPath) {
		MGE::VisualMarkersManager::getPtr()->hideMarker(node);
		MGE::OgreUtils::recursiveDeleteSceneNode(node);
	}
	visualPath.clear();
}

void MGE::PathFinder::showPath(std::list<Ogre::Vector3> points, MGE::World3DObject* object, const Ogre::ColourValue& colour, int mode) {
	const Ogre::AxisAlignedBox& aabb = object->getAABB();
	
	Ogre::Vector3 point, nextPoint;
	while (points.size() >= 2) {
		point = points.front();
		points.pop_front();
		nextPoint = points.front();
		
		Ogre::SceneNode* node = MGE::NamedSceneNodes::createSceneNode();
		node->setPosition(point);
		node->lookAt(nextPoint, Ogre::Node::TS_PARENT);
		MGE::VisualMarkersManager::getPtr()->showMarker(node, &aabb, mode, MGE::OgreUtils::getColorDatablock(colour), 0);
		visualPath.push_back(node);
	}
	
	Ogre::SceneNode* node = MGE::NamedSceneNodes::createSceneNode();
	node->setPosition(nextPoint);
	node->lookAt(point, Ogre::Node::TS_PARENT);
	MGE::VisualMarkersManager::getPtr()->showMarker(node, &aabb, mode, MGE::OgreUtils::getColorDatablock(colour), 0);
	visualPath.push_back(node);
}

void MGE::PathFinder::showPath(PathNode* pathNode, MGE::World3DObject* object, const Ogre::ColourValue& colour, int mode) {
	const Ogre::AxisAlignedBox& aabb = object->getAABB();
	
	Ogre::Vector3 pos = pathNode->point.toOgre();
	while (pathNode != NULL) {
		Ogre::SceneNode* node = MGE::NamedSceneNodes::createSceneNode();
		node->setPosition(pos);
		if (pathNode->parent) {
			Ogre::Vector3 newPos = pathNode->parent->point.toOgre();
			node->lookAt(newPos, Ogre::Node::TS_PARENT, Ogre::Vector3::UNIT_Z);
			LOG_VERBOSE(" showPath: step from: " << newPos << " to: " << pos << " => dir: " << -1 * node->getOrientation().zAxis() << " / 0x" << std::hex << pathNode->direction);
			pos = newPos;
		} else {
			node->setOrientation(object->getWorldOrientation());
			LOG_VERBOSE(" showPath: init from: " << pos << " => dir: " << -1 * node->getOrientation().zAxis() <<  " / 0x" << std::hex << pathNode->direction);
		}
		pathNode = pathNode->parent;
		
		MGE::VisualMarkersManager::getPtr()->showMarker(node, &aabb, mode, MGE::OgreUtils::getColorDatablock(colour), 0);
		visualPath.push_back(node);
	}
}
#endif

MGE::PathFinder::PathFinder() {
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	readyToRemove = false;
	#endif
	LOG_DEBUG("PathFinder constructor " << this);
}

MGE::PathFinder::~PathFinder() {
	LOG_DEBUG("PathFinder destructor " << this);
}

#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
MGE::PathFinder::MarkedPoint::MarkedPoint(const Ogre::Vector3& p, const Ogre::Vector3& fp, int t, Ogre::MovableObject* c) :
	point (p), fromPoint (fp), type (t), collison (c), node (NULL)
{ }

MGE::PathFinder::MarkedPoint::~MarkedPoint() {
	if (node)
		MGE::OgreUtils::recursiveDeleteSceneNode(node);
}

void MGE::PathFinder::reinitVisualGrid() {
	LOG_DEBUG("reinitVisualGrid  " << this <<  "  " << &visualGrid << " : " << visualGrid.size());
	visualGrid.clear();
	visualGridIter = visualGrid.begin();
	
	markerSettings.markerType      = MGE::VisualMarker::OBBOX | MGE::VisualMarker::BOX_PROPORTIONAL_THICKNESS | MGE::VisualMarker::FULL_BOX;
	markerSettings.materialName    = MGE::OgreUtils::getColorDatablock(Ogre::ColourValue(.916, 0, 0));
	markerSettings.linesThickness  = 0.06;
}

void MGE::PathFinder::addGridNode(const Ogre::Vector3& point, int type, const Ogre::Vector3& fromPoint, Ogre::MovableObject* collison) {
	visualGrid.emplace_back(point, fromPoint, type, collison);
}

void MGE::PathFinder::showNextGridPoint(const std::list<MarkedPoint>::iterator& iter) {
	static Ogre::MeshPtr gridNodeSphereMarkerMesh = Ogre::MeshManager::getSingleton().getByName("PathFinder_SphereMesh");
	if ( gridNodeSphereMarkerMesh.isNull() )
		gridNodeSphereMarkerMesh = MGE::Shapes::createSphereMesh(MGE::LoadingSystem::getPtr()->getGameSceneManager(), "PathFinder_SphereMesh", "General", "MAT_GIZMO_ALL", 0.2, 16, 16);
	
	static Ogre::MeshPtr gridNodeConeMarkerMesh = Ogre::MeshManager::getSingleton().getByName("PathFinder_ConeMesh");
	if ( gridNodeConeMarkerMesh.isNull() )
		gridNodeConeMarkerMesh = MGE::Shapes::createConeMesh(MGE::LoadingSystem::getPtr()->getGameSceneManager(), "PathFinder_ConeMesh", "General", "MAT_GIZMO_ALL", 0.2, 0.6, 8);
	
	if (!iter->node) {
		iter->node = MGE::NamedSceneNodes::createSceneNode( MGE::LoadingSystem::getPtr()->getGameSceneManager()->getRootSceneNode() );
		Ogre::Item* item;
		
		iter->node->setPosition( iter->point );
		
		switch (iter->type) {
			case PARENT:
			case CHILD_FORBIDDEN2:
				item = iter->node->getCreator()->createItem(gridNodeSphereMarkerMesh);
				break;
			case CHILD_OK:
			case CHILD_FORBIDDEN:
				item = iter->node->getCreator()->createItem(gridNodeConeMarkerMesh);
				iter->node->lookAt(iter->fromPoint, Ogre::Node::TS_PARENT, Ogre::Vector3::NEGATIVE_UNIT_Z);
				break;
		}
		
		switch (iter->type) {
			case PARENT:
				item->setDatablock("MAT_GIZMO_ALL");
				iter->node->setPosition( iter->point + Ogre::Vector3(0,  0.6, 0) );
				break;
			case CHILD_OK:
				item->setDatablock("MAT_GIZMO_Y");
				iter->node->setPosition( iter->point + Ogre::Vector3(0, -0.6, 0) );
				break;
			case CHILD_FORBIDDEN:
				item->setDatablock("MAT_GIZMO_X");
				break;
			case CHILD_FORBIDDEN2:
				item->setDatablock("MAT_GIZMO_Z");
				break;
		}
		
		item->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
		item->setQueryFlags(0);
		iter->node->attachObject(item);
	}
	
	if(iter->collison) {
		MGE::VisualMarkersManager::getPtr()->showMarker( iter->collison->getParentSceneNode(), NULL, markerSettings );
	}
}

void MGE::PathFinder::showNextGridPoints(int count) {
	static int ii = 0;
	
	if (visualGridIter == visualGrid.end()) {
		LOG_DEBUG("visualGridIter was .end() iterator ... set to begin");
		visualGridIter = visualGrid.begin();
	}
	
	for (int i=0; i<count; ++i) {
		LOG_DEBUG("show " << ++ii << " / " << visualGrid.size());
		showNextGridPoint(visualGridIter);
		
		LOG_DEBUG("prepare to " << i+1);
		std::list<MarkedPoint>::iterator prev = visualGridIter;
		++visualGridIter;
		if (visualGridIter == visualGrid.end()) {
			LOG_DEBUG("end data at " << i);
			visualGridIter = prev;
			break;
		}
	}
}
#endif
