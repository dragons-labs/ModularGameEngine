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

#include "BaseClasses.h"
#include "MainLoopListener.h"

#include "data/structs/components/3DWorld.h"
#include "physics/Physics.h"

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements car vehicle.
 *         Should be used instead of standard World3DObject or World3DMovable component.
 */
class Car :
	public MGE::World3DObjectImpl
{
public:
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 11;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// static function for register in MGE::ComponentFactory
	static MGE::BaseComponent* create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID);
	
	/// static function performing registration in MGE::ComponentFactory and in MGE::LoadingSystem::sceneConfigParseListeners
	static bool setup(MGE::ComponentFactory* factory);
	
	#ifdef USE_BULLET
	/**
	 * @brief initialise vehicle physics
	 * 
	 * @param[in] _carPhysics    - pointer to car physics
	 * @param[in] _engineMax     - engine max force
	 * @param[in] _steerLimit    - steer limit
	 * @param[in] _brakeForce    - breaking force value
	 * @param[in] suspensionStiffness   - suspension stiffness
	 * @param[in] suspensionDamping     - suspension damping
	 * @param[in] suspensionCompression - suspension compression
	 */
	void initVehicle(
		btRigidBody* _carPhysics, btScalar _engineMax, btScalar _steerLimit, btScalar _brakeForce,
		btScalar suspensionStiffness, btScalar suspensionDamping, btScalar suspensionCompression
	);
	
	/**
	 * @brief add wheel to car
	 * 
	 * @param[in] node                  - wheel Ogre scene node
	 * @param[in] suspensionRestLength  - suspension rest length
	 * @param[in] wheelRadius           - wheel radius
	 * @param[in] isFrontWheel          - true for front wheel
	 * 
	 * @param[in] wheelFriction         - wheel friction
	 * @param[in] rollInfluence         - roll influence
	 * @param[in] suspensionStiffness   - suspension stiffness
	 * @param[in] suspensionDamping     - suspension damping
	 * @param[in] suspensionCompression - suspension compression
	 */
	void addWheel(
		Ogre::SceneNode* node,
		btScalar suspensionRestLength, btScalar wheelRadius, bool isFrontWheel,
		btScalar wheelFriction, btScalar rollInfluence,
		btScalar suspensionStiffness, btScalar suspensionDamping, btScalar suspensionCompression
	);
	
	/**
	 * Update movement parametrs
	 */
	void go(float accel, float turn, float brk, float dt);
	#endif // USE_BULLET
	
protected:
	friend class CarControler;
	
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	/// constructor
	Car(MGE::NamedObject* parent);
	
	/// destructor
	virtual ~Car();
	
private:
	#ifdef USE_BULLET
	btRigidBody*                             carPhysics;
	btRaycastVehicle::btVehicleTuning        carTuning;
	btVehicleRaycaster*                      carVehicleRayCaster;
	btRaycastVehicle*                        carVehicle;
	std::vector<Ogre::SceneNode*>            carWheels;
	
	float throttle, steering, brakes;
	float engineMax, steerLimit, brakeForce;
	#endif
};

/**
 * @brief Simple keyboard controller for car and triggers checking
 */
class CarControler : public MGE::MainLoopListener, public MGE::TrivialSingleton<MGE::CarControler> {
public:
	/// pointer to current controlled car
	Car* currentCar;
	
	/// set of all cars
	std::set<Car*> allCars;
	
	/**
	* Simple keyboard control to update movement parametrs
	*/
	virtual bool update(float gameTimeStep, float realTimeStep) override;
	
protected:
	friend class TrivialSingleton;
	CarControler()  = default;
	~CarControler() = default;
};

/// @}

}
