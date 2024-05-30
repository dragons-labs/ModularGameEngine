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

#pragma   once

#include "config.h"

#include "StringUtils.h"
#include "MessagesSystem.h"

#include "data/structs/components/3DWorld.h"

namespace MGE { class PathFinder; }

#include <OgreVector2.h>
#include <thread>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements 3d world movable Actor.
 *        Should be used insted of standard World3DObject component.
 */
class World3DMovable :
	public MGE::World3DObjectImpl
{
public:
	/**
	 * @brief flags using to actor selection and filtering
	 */
	enum SubTypes {
		/// is person
		IS_PERSON,
		/// is passanger car
		IS_CAR,
		/// is (semi) truck
		IS_TRUCK,
		/// is offroad truck/car (e.g. jeep)
		IS_OFFROAD_TRUCK,
		/// is heavy truck (e.g. fire engine, fire ladder)
		IS_HEAVY_TRUCK,
		/// is offroad heavy truck
		IS_HEAVY_OFFROAD_TRUCK,
		/// is boat
		IS_BOAT,
		/// is water-ground vehicle (amphibious vehicle)
		IS_AMPHIBIOUS,
		/// is flaying water-ground vehicle (hovercraft)
		IS_HOVERCRAFT
	};
	
	/**
	 * @brief convert string notation of SubTypes to numeric value
	 * 
	 * @param[in] s  string to convert
	 */
	inline static int stringToSubType(const std::string_view& s) {
		if (s == "IS_PERSON")                    return IS_PERSON;
		else if (s == "IS_CAR")                  return IS_CAR;
		else if (s == "IS_TRUCK")                return IS_TRUCK;
		else if (s == "IS_OFFROAD_TRUCK")        return IS_OFFROAD_TRUCK;
		else if (s == "IS_HEAVY_TRUCK")          return IS_HEAVY_TRUCK;
		else if (s == "IS_HEAVY_OFFROAD_TRUCK")  return IS_HEAVY_OFFROAD_TRUCK;
		else if (s == "IS_BOAT")                 return IS_BOAT;
		else if (s == "IS_AMPHIBIOUS")           return IS_AMPHIBIOUS;
		else if (s == "IS_HOVERCRAFT")           return IS_HOVERCRAFT;
		return MGE::StringUtils::toNumeric<int>(s);
	}
	
	int getSubType() const {
		return subType;
	}
	
	/**
	 * @brief check possiblity of crossing object
	 * 
	 * @param[in]  object      object to cross
	 * 
	 * @return if (value \< 0) error; if (value \> 0) success
	 *         full list of values see @ref ReturnCodes
	 */
	int16_t canCrossObject(const MGE::BaseActor* object) const;
	
	/// @copydoc MGE::World3DObject::canMove
	virtual int16_t canMove(
		const Ogre::Vector3& start, const Ogre::Vector3& end,
		float& speedModifier, float& squaredLength, float& heightDiff,
		std::forward_list<MGE::BaseActor*>* triggers = 0,
		Ogre::MovableObject** collisionWith = 0
	) const override;
	
	/**
	 * @brief initialize scene object move (prepare @ref MoveInfo, do pathfinding, init first step of move via @ref MoveInfo::reinitMove)
	 * 
	 * @param[out]    target    3D world final destination point
	 */
	void initMove(const Ogre::Vector3& target);
	
	/**
	 * @brief initialize scene object move by points list (prepare @ref MoveInfo, WITHOUT doing pathfinding, init first step of move via @ref MoveInfo::reinitMove)
	 * 
	 * @param[out]    points    list of points (straight line movement between points)
	 */
	void initMove(const std::list<Ogre::Vector3>& points);
	
	/**
	 * @brief cancel move during initialization or execution
	 */
	void cancelMove();
	
	/**
	 * @brief return move preparation (pathfinding status)
	 *
	 * @return
	 *   *  1 when move is ready and path is OK
	 *   *  0 when path is NOT OK
	 *   * -2 when path is NOT ready or searching is not initialised
	 */
	int moveIsReady() {
		if (! moveInfo || !moveInfo->ready) {
			return -2;
		}
		return moveInfo->pathStatus > 0;
	}
	
	/**
	 * @brief do single moving step
	 * 
	 * @param[in] gameTimeStep  time since the previous move step
	 * 
	 * @return
	 * @li 0 - move step successful, continue moving
	 * @li 1 - move step successful, moving finish
	 * @li 2 - moving errror
	 */
	int doMoveStep(float gameTimeStep);
	
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 8;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID || id == MGE::World3DObject::classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// constructor
	World3DMovable(MGE::NamedObject* parent);
	
protected:
	/// sub-type id value
	int subType;
	
	/// destructor
	virtual ~World3DMovable();
	
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	/// current speed of actors (using for movment with different than maximum speed)
	float currentSpeed;
	
	/// maximum sin^2 of slope angle for movement
	float maxSlopeSin2;
	
	/**
	 * @brief move actor on ground - do move step based on @ref moveInfo and time step @a t
	 * 
	 * @param[in]     t          - time since the previous move step
	 */
	int _doMoveStep(float t);
	
	/**
	 * @brief move actor on ground in single step - do real checks and move
	 * 
	 * @param[in,out] gotoPoint     - move destination, function can update Y coordinate
	 * @param[in]     position      - start position
	 * @param[in]     search4ground - when true (default) serching for ground at end point and update y coordinate in @a end point vector
	 */
	int moveTo_onGround(Ogre::Vector3& gotoPoint, const Ogre::Vector3& position, bool search4ground = true);
	
private:
	struct MoveInfo {
		std::list<Ogre::Vector3> points;
		
		Ogre::Vector3     moveStart;
		Ogre::Vector3     turnEnd;
		Ogre::Vector3     moveEnd; // last point in current move step (turning start point)
		Ogre::Vector3     moveDst; // destination of current move step
		
		Ogre::Vector3     direction;
		Ogre::Real        moveLen; // distance betwen moveStart and moveEnd
		
		Ogre::Vector3     target; // target point of whole move action
		
		Ogre::Vector2     turnStartRadius;
		Ogre::Vector2     turnEndRadius;
		Ogre::Radian      turnAngle;
		Ogre::Real        turnLen;
		
		short             turning;
		bool              moving;
		bool              finish;
		bool              ready;
		Ogre::Real        traveledDistance;
		
		MGE::PathFinder*  pathFinder;
		std::thread*      pathFinderThread;
		int16_t           pathStatus;
		
		/**
		 * @brief (re)initialise actor move (rotate to target, calculate distance, ...)
		 * 
		 * @return
		 * @li true  - next move step is possible
		 * @li false - can't do next move step, we on finish point
		 */
		bool reinitMove();
		
		MoveInfo() {
			ready = 0;
			pathFinder = 0;
			pathFinderThread = nullptr;
		}
		
		~MoveInfo();
		
		void deletePathfinder(bool cancelThread);
		
		bool storeToXML(pugi::xml_node&& xmlNode) const;
		
		bool restoreFromXML(const pugi::xml_node& xmlNode);
	};
	
	void doPathFinding();
	
	MoveInfo* moveInfo;
};

/**
 * @brief struct for actor move state update message (is sending when actor start/stop moving)
 */
struct ActorMovingEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorMovingUpdate"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// actor with updated moving state
	MGE::BaseActor* actor;
	
	/// actor's moving state
	bool isMove;
	
	/// constructor
	ActorMovingEventMsg(MGE::BaseActor* _actor, bool _isMove) :
		actor(_actor), isMove(_isMove)
	{}
};

/// @}

}
