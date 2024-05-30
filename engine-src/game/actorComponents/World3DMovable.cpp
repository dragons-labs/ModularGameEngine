/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "game/actorComponents/World3DMovable.h"

#include "Engine.h"

#include "ScriptsSystem.h"
#include "physics/TimeSystem.h"
#include "physics/Raycast.h"
#include "physics/PathFinder.h"
#include "data/utils/OgreUtils.h"
#include "physics/utils/OgreColisionBoundingBox.h"

#include "data/structs/BaseActor.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "game/actions/ActionQueue.h"
#include "game/actions/Action.h"
#include "game/actions/ActionPrototype.h"
#include "game/actorComponents/Trigger.h"

#include "data/property/XmlUtils_Ogre.h"

#ifdef PATHFINDER_SUBTHREAD
#include <pthread.h>
#endif

#ifdef MGE_DEBUG_MOVE
#define DEBUG_MOVE_LOG_STREAM(a)  LOG_VERBOSE(a);
#else
#define DEBUG_MOVE_LOG_STREAM(a)
#endif

/*--------------------- constructor, destructor, create(), setup() ---------------------*/

MGE::World3DMovable::World3DMovable(MGE::NamedObject* parent) :
	MGE::World3DObjectImpl(parent),
	maxSlopeSin2(0.16),
	moveInfo(NULL)
{
	owner = static_cast<MGE::BaseActor*>(parent);
}

MGE::World3DMovable::~World3DMovable() {
	delete moveInfo;
}

MGE_ACTOR_COMPONENT_CREATOR(MGE::World3DMovable, World3DMovable) {
	typeIDs->insert(MGE::World3DObject::classID);
	typeIDs->insert(MGE::World3DMovable::classID);
	return new MGE::World3DMovable(parent);
}


/*--------------------- store / restore ---------------------*/

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_World3DMovable World3DMovable

Store / restore from its @c \<Component\> node subnodes:
  - subnodes described in @ref ActorComponent_World3DObject
  - @c SubType determines movable actor subtype (see @ref MGE::World3DMovable::SubTypes, string or numeric value converted via @ref MGE::World3DMovable::stringToSubType)
  - @c \<MaxSlopeSin2\> maximum sin^2 of slope angle for movement
  - @c \<MoveInfo\> described current status of movement – serialisation of @ref MGE::World3DMovable::MoveInfo

See @ref MGE::World3DMovable for details.
*/
bool MGE::World3DMovable::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	auto moveInfoXML = xmlNode.child("MoveInfo");
	if (moveInfoXML) {
		delete moveInfo;
		moveInfo = new MoveInfo();
		moveInfo->restoreFromXML(moveInfoXML);
	}
	
	auto subTypeXML = xmlNode.child("SubType");
	if (subTypeXML) { // setup can be omitted in save xml format
		subType = stringToSubType( subTypeXML.text().as_string() );
		maxSlopeSin2  = MGE::XMLUtils::getValue<float>(xmlNode.child("MaxSlopeSin2"), 0.16);
	}
	MGE::World3DObjectImpl::restoreFromXML(xmlNode, parent, sceneNode);
	return true;
}

bool MGE::World3DMovable::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	MGE::World3DObjectImpl::storeToXML(xmlNode, onlyRef);
	
	if (moveInfo) {
		moveInfo->storeToXML(xmlNode.append_child("MoveInfo"));
	}
	return true;
}

bool MGE::World3DMovable::MoveInfo::restoreFromXML(const pugi::xml_node& xmlNode){
	points.clear();
	for (auto xmlSubNode : xmlNode.child("points")) {
		points.push_back( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlSubNode) );
	}
	
	moveStart = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("moveStart"));
	turnEnd = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("turnEnd"));
	moveEnd = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("moveEnd"));
	moveDst = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("moveDst"));
	
	direction = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("direction"));
	moveLen = xmlNode.child("moveLen").text().as_float();
	
	target = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlNode.child("target"));
	
	turnStartRadius = MGE::XMLUtils::getValue<Ogre::Vector2>(xmlNode.child("turnStartRadius"));
	turnEndRadius = MGE::XMLUtils::getValue<Ogre::Vector2>(xmlNode.child("turnEndRadius"));
	turnAngle = MGE::XMLUtils::getValue<Ogre::Radian>(xmlNode.child("turnAngle"));
	turnLen = xmlNode.child("turnLen").text().as_float();
	
	turning = xmlNode.child("turning").text().as_int();
	moving = xmlNode.child("moving").text().as_bool();
	finish = xmlNode.child("finish").text().as_bool();
	ready = xmlNode.child("ready").text().as_bool();
	traveledDistance = xmlNode.child("traveledDistance").text().as_float();
	
	return true;
}

bool MGE::World3DMovable::MoveInfo::storeToXML(pugi::xml_node&& xmlNode) const {
	auto xmlSubNode = xmlNode.append_child("points");
	for (auto& p : points) {
		xmlSubNode.append_child("point") << p;
	}
	
	xmlNode.append_child("moveStart") << moveStart;
	xmlNode.append_child("turnEnd") << turnEnd;
	xmlNode.append_child("moveEnd") << moveEnd;
	xmlNode.append_child("moveDst") << moveDst;
	
	xmlNode.append_child("direction") << direction;
	xmlNode.append_child("moveLen") << moveLen;
	
	xmlNode.append_child("target") << target;
	
	xmlNode.append_child("turnStartRadius") << turnStartRadius;
	xmlNode.append_child("turnEndRadius") << turnEndRadius;
	xmlNode.append_child("turnAngle") << turnAngle;
	xmlNode.append_child("turnLen") << turnLen;
	
	xmlNode.append_child("turning") << turning;
	xmlNode.append_child("moving") << moving;
	xmlNode.append_child("finish") << finish;
	xmlNode.append_child("ready") << ready;
	xmlNode.append_child("traveledDistance") << traveledDistance;
	
	return true;
}


/*--------------------- check if move is possible (including path finder calls and callbacks) ---------------------*/

int16_t MGE::World3DMovable::canCrossObject(const MGE::BaseActor* object) const {
	const MGE::Trigger* trigger = object->getComponent<MGE::Trigger>();
	if (!trigger) {
		return MGE::PathFinder::ACTOR_COLLISION;
	} else {
		if (trigger->getSpeedModifier(owner) > 0)
			return MGE::PathFinder::NEED_RUN_TRIGGERS;
		else
			return MGE::PathFinder::TRIGGER_NO_ACCESS;
	}
}

int16_t MGE::World3DMovable::canMove(
	const Ogre::Vector3& start, const Ogre::Vector3& end,
	float& speedModifier, float& squaredLength, float& heightDiff,
	std::forward_list<MGE::BaseActor*>* triggers,
	Ogre::MovableObject** collisionWith
) const {
	DEBUG_MOVE_LOG_STREAM("   check move possibility from " << start << " to " << end);
	
	// 1. calculate move vector (vector between start and end point) and slope info
	Ogre::Vector3 moveVector = end - start;
	           squaredLength = moveVector.squaredLength();
	              heightDiff = moveVector.y;
	Ogre::Real   tangentSin2 = heightDiff * heightDiff / squaredLength;
	
	if (tangentSin2 > maxSlopeSin2) {
		DEBUG_MOVE_LOG_STREAM("     - too steeply tangentSin2: " << tangentSin2);
		return MGE::PathFinder::TOO_STEEPLY;
	}
	
	std::list<Ogre::MovableObject*> collisionObject;
	if (! MGE::OgreColisionBoundingBox::isFreePath(
			getOgreSceneNode(), getAABB(), start, end,
			MGE::QueryFlags::COLLISION_OBJECT | MGE::QueryFlags::TRIGGER,
			&collisionObject
		)
	) {
		
		for (auto& iter : collisionObject) {
			int qf = iter->getQueryFlags();
			if (qf & MGE::QueryFlags::COLLISION_OBJECT) {
				DEBUG_MOVE_LOG_STREAM("     - collision with COLLISION_OBJECT: " << iter->getName() << " @ " << iter->getParentSceneNode()->getPosition());
				if (collisionWith)* collisionWith = iter;
				return MGE::PathFinder::OBJECT_COLLISION;
			} else if (qf & MGE::QueryFlags::GAME_OBJECT) {
				auto  actor = MGE::BaseActor::get(iter);
				float triggerSpeedModifier = actor->getComponent<MGE::Trigger>()->getSpeedModifier(owner);
				if (triggerSpeedModifier == 0) {
					DEBUG_MOVE_LOG_STREAM("     - collision with no crossable TRIGGER: " << actor->getName() << " @ " << iter->getParentSceneNode()->getPosition());
					return MGE::PathFinder::TRIGGER_NO_ACCESS;
				} else {
					speedModifier *= triggerSpeedModifier;
				}
				
				if (triggers)
					triggers->push_front( actor );
			}
		}
	}
	
	return MGE::PathFinder::CAN_MOVE;
}

/*--------------------- prepare move plan ---------------------*/

void MGE::World3DMovable::cancelMove() {
	delete moveInfo;
	moveInfo = nullptr;
}

MGE::World3DMovable::MoveInfo::~MoveInfo() {
	LOG_DEBUG("MoveInfo destructor");
	deletePathfinder(true);
}

void MGE::World3DMovable::MoveInfo::deletePathfinder(bool killThread) {
	#ifdef PATHFINDER_SUBTHREAD
	if (pathFinderThread) {
		LOG_DEBUG("detach pathfinder thread in " << this);
		
		auto nativeThread = pathFinderThread->native_handle();
		pathFinderThread->detach();
		delete pathFinderThread;
		pathFinderThread = nullptr;
		
		if (killThread) {
			LOG_DEBUG("cancel pathfinder thread in " << this);
			pthread_cancel(nativeThread);
			pthread_join(nativeThread, NULL);
		}
	} else {
		LOG_DEBUG("call deletePathfinder without pathFinderThread in " << this);
	}
	#endif
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
	if (pathFinder && MGE::ScriptsSystem::getPtr()->getGlobalsDict()["visualPathFinder"].cast<MGE::PathFinder*>() == pathFinder) {
		pathFinder->readyToRemove = true;
		LOG_DEBUG("this is visualPathFinder ... skip deletion " << pathFinder << " in " << this);
	}
	else
	#endif
	{
		LOG_DEBUG("delete pathfinder " << pathFinder << " in " << this);
		delete pathFinder;
	}
	pathFinder = nullptr;
}

void MGE::World3DMovable::initMove(const Ogre::Vector3& target) {
	delete moveInfo;
	moveInfo = new MoveInfo();
	
	moveInfo->moveStart = getWorldPosition();
	moveInfo->direction = getWorldDirection();
	moveInfo->moveEnd   = moveInfo->moveStart;
	moveInfo->moveDst   = moveInfo->moveStart + 0.7 * moveInfo->direction;
	moveInfo->moveLen   = 0;
	moveInfo->target    = target;
	moveInfo->ready     = false;
	
	moveInfo->pathFinder = new MGE::PathFinder();
	LOG_DEBUG("pathfinder " << moveInfo->pathFinder << " created in " << moveInfo << " for " << this);
	
	#ifdef MGE_DEBUG_PATHFINDER_VISUAL_GRID
	{
		MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
		MGE::PathFinder* oldVisualPathFinder = nullptr;
		try {
			oldVisualPathFinder = MGE::ScriptsSystem::getPtr()->getGlobalsDict()["visualPathFinder"].cast<MGE::PathFinder*>();
		} catch (...) {}
		if (!oldVisualPathFinder || oldVisualPathFinder->readyToRemove) {
			delete oldVisualPathFinder;
			MGE::ScriptsSystem::getPtr()->getGlobalsDict()["visualPathFinder"] = pybind11::cast(moveInfo->pathFinder);
		}
	}
	#endif
	
	#ifdef PATHFINDER_SUBTHREAD
	moveInfo->pathFinderThread = new std::thread(&MGE::World3DMovable::doPathFinding, this);
	#else
	doPathFinding();
	#endif
}

void MGE::World3DMovable::doPathFinding() {
	#ifdef PATHFINDER_SUBTHREAD
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	#endif
	
	LOG_DEBUG("initMove: from=" << moveInfo->moveStart << " with init direcion=" << moveInfo->direction << " to dst=" << moveInfo->target);
	
	moveInfo->pathStatus = moveInfo->pathFinder->findPath(this, moveInfo->moveStart, moveInfo->target, moveInfo->points );
	
	moveInfo->deletePathfinder(false);
	
	if (moveInfo->pathStatus >= 0) {
		// remove start point (current position) from moveInfo->points
		moveInfo->points.pop_front();
		
		#ifdef MGE_DEBUG_MOVE_GNUPLOT
		LOG_VERBOSE( "GNUPLOT: #USAGE: ./Make.sh run |& grep '^GNUPLOT' | cut -f2- -d' ' > /tmp/plot  # and use gnuplot 'load \"/tmp/plot\"' command" );
		LOG_VERBOSE( "GNUPLOT: set parametric; set trange [0:2*pi]; set size ratio -1" );
		LOG_VERBOSE( "GNUPLOT: plot '<echo " << moveInfo->moveStart.x << " " << moveInfo->moveStart.z << " " << moveInfo->direction.x << " " << moveInfo->direction.z <<
			"' title 'start' with vectors filled head size character 2.7,30,60 lw 2 lc rgb '#ff0000'" );
		LOG_VERBOSE( "GNUPLOT: replot '<echo " << moveInfo->points.front().x << " " << moveInfo->points.front().z << "' title 'stepDst' with points lc rgb '#0000ff' pt 5" );
		LOG_VERBOSE( "GNUPLOT: replot '<echo " << moveInfo->points.back().x  << " " << moveInfo->points.back().z  << "' title 'finish' with points lc rgb '#ff00ff' pt 5" );
		LOG_VERBOSE( "GNUPLOT: pause 3" );
		#endif
		
		moveInfo->reinitMove();
	}
	
	// path finding is finished
	moveInfo->ready = true;
	
	auto action = owner->getComponent<MGE::ActionQueue>()->getFirstAction();
	if (action && action->getType() & MGE::ActionPrototype::WAIT_FOR_READY_FLAG)
		action->ready = true;
}

void MGE::World3DMovable::initMove(const std::list<Ogre::Vector3>& points) {
	delete moveInfo;
	moveInfo = new MoveInfo();
	
	moveInfo->moveEnd   = getWorldPosition();
	moveInfo->direction = getWorldDirection();
	moveInfo->moveDst   = moveInfo->moveStart + 0.7 * moveInfo->direction;
	
	for (auto& iter : points) {
		moveInfo->points.push_back(iter);
	}
	
	moveInfo->reinitMove();
}

bool MGE::World3DMovable::MoveInfo::reinitMove() {
	std::list<Ogre::Vector3>::iterator dstPoint = points.begin();
	if (dstPoint == points.end())
		return false;
	
	Ogre::Vector3 lastDir   = direction;
	Ogre::Vector3 turnPoint = moveDst;
	Ogre::Vector3 newDst    = *dstPoint;
	Ogre::Vector3 newDir    = newDst - turnPoint;
	Ogre::Vector3 nextDst;
	newDir.y = 0;
	newDir.normalise();
	
	moveStart     = moveEnd;        // when call reinitMove() moveEnd is our current position
	turnEnd       = turnPoint + newDir * 0.7;
	if (++dstPoint == points.end()) {
		moveEnd   = newDst;
		nextDst   = newDst;
	} else {
		moveEnd   = newDst - newDir * 0.7;
		nextDst   = *dstPoint;
	}
	moveDst       = newDst;
	
	if ( (nextDst - moveEnd).squaredLength() >= (nextDst - turnEnd).squaredLength() ) { // if turnEnd is closer to nextDst than moveEnd
		moveLen   = 0;
	} else {
		direction = moveEnd - turnEnd;  // offset from turnEnd to (current step) moveEnd
		moveLen   = direction.normalise();
	}
	
	// line orthogonal to vector "lastDir" and passing through point "moveStart"
	float A1 = lastDir.x;
	float B1 = lastDir.z;
	float C1 = -A1 * moveStart.x - B1 * moveStart.z;
	
	// line orthogonal to vector "newDir" and passing through point "turnEnd"
	float A2 = newDir.x;
	float B2 = newDir.z;
	float C2 = -A2 * turnEnd.x - B2 * turnEnd.z;
	
	// lines crossing point
	float W  = A1*B2 - A2*B1;
	float Cx = (C2*B1 - C1*B2)/W;
	float Cz = (C1*A2 - C2*A1)/W;
	
	Ogre::Vector2 r1(moveStart.x - Cx, moveStart.z - Cz);
	Ogre::Vector2 r2(turnEnd.x   - Cx, turnEnd.z  - Cz);
	
	turnStartRadius  = r1;
	turnEndRadius    = r2;
	turnAngle        = r1.angleBetween(r2);
	turnLen          = turnAngle.valueRadians() * r1.length();
	if (r1.crossProduct(r2)<0)
		turnAngle = -turnAngle;
	
	traveledDistance = 0;
	turning          = 3;
	moving           = false;
	finish           = false;
	
	LOG_DEBUG("reinitMove done:"
		<< " moveStart=" << moveStart << " turnPoint=" << turnPoint
		<< " turnEnd="   << turnEnd   << " moveEnd=" << moveEnd
		<< " moveDst="   << moveDst
		<< " lastDir="   << lastDir             << " newDir="  << newDir
		<< " Cx="        << Cx                  << " Cz="  << Cz
		<< " turnStartRadius=" << r1            << " turnEndRadius=" << r2
		<< " turnAngle=" << turnAngle << " turnLen=" << turnLen
	);
	
	#ifdef MGE_DEBUG_MOVE_GNUPLOT
	LOG_VERBOSE( "GNUPLOT: replot " << r1.length() << "*cos(t)+" << Cx << ", " << r1.length() << "*sin(t)+" << Cz << " notitle lc rgb '#00ffff';");
	LOG_VERBOSE( "GNUPLOT: replot '<echo " << moveStart.x << " " << moveStart.z << "'  with points lc rgb '#00ff00' pt 9 title 'turn start'" );
	LOG_VERBOSE( "GNUPLOT: replot '<echo " << turnEnd.x << " " << turnEnd.z << "'  with points lc rgb '#00ffff' pt 9 title 'turn end'" );
	LOG_VERBOSE( "GNUPLOT: replot '<echo " << moveDst.x << " " << moveDst.z << "'  with points lc rgb '#ffff00' pt 9 title 'turn destination'" );
	LOG_VERBOSE( "GNUPLOT: replot '<echo " << turnPoint.x << " " << turnPoint.z << "'  with points lc rgb '#ff0000' pt 9 title 'turn point'" );
	LOG_VERBOSE( "GNUPLOT: pause 1.5" );
	#endif
	
	return true;
}


/*--------------------- realize moving ---------------------*/

int MGE::World3DMovable::doMoveStep(float gameTimeStep) {
	if (moveInfo->finish) {
		// on SUB-TARGET
		moveInfo->points.pop_front();
		
		if ( !moveInfo->reinitMove() ) {
			// on FINAL-TARGET
			delete moveInfo;
			moveInfo = NULL;
			return 1;
		}
	} else if (!moveInfo->moving) {
		moveInfo->moving = true; // skip first frame after calculate path
	} else {
		int moveOK = _doMoveStep(gameTimeStep);
		
		if (moveOK != 0) {
			LOG_WARNING("error in movement from " << getWorldPosition() << " to " << moveInfo->points.back()
				<< " via " << ( moveInfo->turning ? moveInfo->turnEnd : moveInfo->moveEnd )
				<< ", error_code = " << std::hex << std::showbase << moveOK
			);
			
			return 2;
		}
	}
	
	return 0;
}

int MGE::World3DMovable::_doMoveStep(float t) {
	Ogre::Vector3 move, dstPoint;
	
	currentSpeed = 3.0; /// @todo TODO.6: calculate currentSpeed ...
	
	// calculate move distanse
	Ogre::Real moveDistance  = currentSpeed * t * MGE::TimeSystem::getPtr()->getSpeed();
	moveInfo->traveledDistance += moveDistance;
	
	if (moveInfo->turning == 3) {
		if (moveInfo->traveledDistance < moveInfo->turnLen) {
			// moving on circle
			Ogre::Real percentDistance = moveInfo->traveledDistance / moveInfo->turnLen;
			Ogre::Vector2 newR = MGE::OgreUtils::rotateVector2(
				moveInfo->turnStartRadius,
				moveInfo->turnAngle * percentDistance
			);
			move.x   = newR.x - moveInfo->turnStartRadius.x;
			move.y   = (moveInfo->turnEnd.y - moveInfo->moveStart.y) * percentDistance;
			move.z   = newR.y - moveInfo->turnStartRadius.y;
			dstPoint = moveInfo->moveStart + move;
		} else {
			// moving on circle - last step
			dstPoint = moveInfo->turnEnd;
			moveInfo->traveledDistance = 0;
			moveInfo->turning = 2;
		}
	} else {
		if (moveInfo->moveLen == 0) {
			// don't do last turn (with moveInfo->turning == 1) and last move (to moveInfo->moveEnd), when moveInfo->moveLen is zero
			moveInfo->finish = true;
			
			// (re)set moveInfo direction and position values to real 3D world values
			moveInfo->moveEnd = getWorldPosition();
			moveInfo->direction = getWorldDirection();
			moveInfo->moveDst = moveInfo->moveEnd + 0.7 * moveInfo->direction;
			
			return 0;
		}
		
		if (moveInfo->traveledDistance < moveInfo->moveLen) {
			// moving on line
			dstPoint = moveInfo->turnEnd + moveInfo->direction * moveInfo->traveledDistance;
		} else {
			// moving on line - last step
			dstPoint = moveInfo->moveEnd;
			moveInfo->finish = true;
		}
	}
	
	if (moveInfo->turning) {
		mainSceneNode->lookAt(dstPoint, Ogre::Node::TS_WORLD, Ogre::Vector3::NEGATIVE_UNIT_Z);
		updateCachedTransform();
		
		DEBUG_MOVE_LOG_STREAM("turning in 3d world direction=" << getWorldDirection() << " from=" << getWorldPosition() << " to=" << dstPoint); // this values should be continuous ...
		
		if (moveInfo->turning == 2) {
			// last turn on circle done, but we need turn on line moving start
			moveInfo->turning = 1;
		} else if (moveInfo->turning == 1) {
			// turn on first line point done, we not turn anymore
			moveInfo->turning = 0;
		}
		
	#ifdef MGE_DEBUG_MOVE_GNUPLOT
		LOG_VERBOSE( "GNUPLOT: pause 0.5; replot '<echo "
			<< getWorldPosition().x << " " << getWorldPosition().z << " " << dstPoint.x - getWorldPosition().x << " " << dstPoint.z - getWorldPosition().z <<
			"' notitle with vectors filled head size character 2.7,30,60 lw 2 lc rgb '#ff8000'" );
	} else {
		LOG_VERBOSE( "GNUPLOT: pause 0.01; replot '<echo " << dstPoint.x << " " << dstPoint.z << "' notitle  with points lc rgb '#ff8000' pt 7" );
	#endif
	}
	
	return moveTo_onGround(dstPoint, getWorldPosition(), false);
}

int MGE::World3DMovable::moveTo_onGround(Ogre::Vector3& gotoPoint, const Ogre::Vector3& position, bool search4ground) {
	// check ground
	if (search4ground) {
		if (! MGE::RayCast::getGroundHeight( getOgreSceneNode()->getCreator(), gotoPoint )) {
			LOG_WARNING("moveTo_onGround: can't find ground");
			return MGE::PathFinder::GROUND_NOT_FOUND;
		}
	}
	
	// check moving possible
	float tmp1, tmp2, tmp3;
	std::forward_list<MGE::BaseActor*> triggers;
	int16_t retCode = canMove(position, gotoPoint, tmp1, tmp2, tmp3, &triggers);
	if (retCode < 0) {
		LOG_WARNING("do forbidden move step");
		/*return retCode;*/ /// @todo TODO.6: don't allow forbidden move step
	}
	
	// run triggers
	for (auto& iter : triggers) {
		iter->getComponent<MGE::Trigger>()->runTrigger(owner);
	}
	
	// do move step
	mainSceneNode->translate(gotoPoint - position);
	
	return 0;
}
