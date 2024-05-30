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

#include "game/actorComponents/Car.h"

#include "LogSystem.h"

#include "input/InputSystem.h"
#include "data/utils/OgreUtils.h"
#include "physics/utils/OgreColisionBoundingBox.h"
#include "data/QueryFlags.h"

#include "data/structs/BaseActor.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "game/actorComponents/Trigger.h"

bool MGE::CarControler::update(float gameTimeStep, float realTimeStep) {
	// check triggers for all cars
	for (auto& car : allCars) {
		std::list<Ogre::MovableObject*> triggers;
		if ( ! MGE::OgreColisionBoundingBox::isFreePosition(car->mainSceneNode, car->aabb, MGE::QueryFlags::TRIGGER, &triggers) ) {
			for (auto& iter : triggers) {
				auto actor = MGE::BaseActor::get(iter);
				if (actor) {
					actor->getComponent<MGE::Trigger>()->runTrigger(car->owner);
				}
			}
		}
	}
	
	if (!currentCar)
		return false;
	
	float accel = 0, turn = 0, brk = 0;
	if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_UP)) {
		accel = 0.1;
	}
	if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_DOWN)) {
		accel = -0.1;
	}
	if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LEFT)) {
		turn = 0.8;
	}
	if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RIGHT)) {
		turn = -0.8;
	}
	if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_SPACE)) {
		if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift)) {
			currentCar->setWorldOrientation(Ogre::Quaternion::IDENTITY);
			currentCar->setWorldPosition(Ogre::Vector3(0,5,0));
		} else {
			brk = 0.2;
		}
	}
	if (accel != 0 || turn != 0 || brk != 0) {
		LOG_DEBUG("Car: action");
		currentCar->go(accel, turn, brk, gameTimeStep);
	}
	// else carVehicle->updateVehicle(dt);
	return true;
}

MGE::BaseComponent* MGE::Car::create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID) {
	typeIDs->insert(MGE::World3DObject::classID);
	typeIDs->insert(classID);
	return new MGE::Car(parent);
}

bool MGE::Car::setup(MGE::ComponentFactory* factory) {
	factory->registerComponent(
		MGE::Car::classID, "Car", MGE::Car::create
	);
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(MGE::CarControler::getPtr(), MGE::CarControler::INPUT_ACTIONS+1);
	return true;
}

MGE_REGISTER_ACTOR_COMPONENT(Car, MGE::Car::setup)


MGE::Car::Car(MGE::NamedObject* parent) :
	MGE::World3DObjectImpl(parent),
	owner( static_cast<MGE::BaseActor*>(parent) )
{
	LOG_DEBUG("Create \"Car\" actor component for " << parent->getName());
	
	MGE::CarControler::getPtr()->allCars.insert(this);
	#ifdef USE_BULLET
	carVehicleRayCaster = NULL;
	carVehicle = NULL;
	#endif
}

MGE::Car::~Car() {
	MGE::CarControler::getPtr()->allCars.erase(this);
	#ifdef USE_BULLET
	// remove physics
	if (MGE::Physics::getPtr()->getDynamicsWorld()) {
		MGE::Physics::getPtr()->getDynamicsWorld()->removeVehicle(carVehicle);
		delete carVehicle;
		delete carVehicleRayCaster;
	}
	#endif
}

bool MGE::Car::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	return MGE::World3DObjectImpl::storeToXML(xmlNode, onlyRef);
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Car Car

Use subnodes:
  - @c \<vehicle\> for Bullet vehicle configuration, can have attributes:
    - @c engineMax   max engine force
    - @c brakeForce  max break force
    - @c steerLimit  limit wheel steering
    - @c suspensionStiffness, @c suspensionDamping and @c suspensionCompression default suspension settings
  - @c \<wheel\> for each vehicle wheel with it config by attributes:
    - @c suspensionRestLength - length of suspension
    - @c radius   - wheel radius
    - @c isFront  - true for front (steering) wheels
    - @c friction - friction value
    - @c rollInfluence
    - @c suspensionStiffness
    - @c suspensionDamping
    - @c suspensionCompression
  - and subnodes described in @ref ActorComponent_World3DObject.

@note
  - Actor @ref XMLNode_Item or @ref XMLNode_Entity node for car body must have @ref XMLNode_Physics with `physicsMode == "full"` and `mass != 0`.
  - Each wheel must have own scene (sub)node without physics
  - Ground object for vehicle must have @ref XMLNode_Physics with `physicsMode == "full"`.

\image html cars_wheels.png

@subsubsection ActorComponent_Car_Example Example
@code{.xml}
<ActorPrototype name="MyCar">
	<ActorComponents>
		<Component classID="Car">
			<vehicle />
			<wheel onNode="car_w1" suspensionRestLength="0.55" radius="0.5" isFront="true" />
			<wheel onNode="car_w2" suspensionRestLength="0.55" radius="0.5" isFront="false" />
			<!-- more wheels here ... -->
		</Component>
	</ActorComponents>
	
	<entity meshFile="MyCar.mesh">
		<physics physicsMode="full" shapeMode="box" mass="2500" />
	</entity>
	
	<node name="car_w1">
		<position x="-0.88" y="0.3" z="-1.8" />
		<entity meshFile="wheel.mesh" />
	</node>
	<node name="car_w2">
		<position x="0.88" y="0.3" z="1.15" />
		<entity meshFile="wheel.mesh" />
	</node>
	<!-- more wheels here ... -->
</ActorPrototype>
@endcode
*/

bool MGE::Car::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	#ifdef USE_BULLET
	MGE::World3DObjectImpl::restoreFromXML(xmlNode, parent, sceneNode);
	
	auto xmlVehicleSubNode = xmlNode.child("vehicle");
	if (!xmlVehicleSubNode)
		return true; // setup can be omitted in save xml format
	
	// remove old physics
	if (carVehicle) {
		MGE::Physics::getPtr()->getDynamicsWorld()->removeVehicle(carVehicle);
		delete carVehicle;
		delete carVehicleRayCaster;
	}
	
	auto phy = MGE::Any::getFromBindings(mainSceneNode, "phy").getValuePtr<std::shared_ptr<Physics::AnyHolder>>();
	if (!phy) {
		LOG_WARNING("Can't find physics for ");
		return false;
	}
	
	// create new physics
	initVehicle(
		static_cast<btRigidBody*>( phy->get()->physicsBody ),
		xmlVehicleSubNode.attribute("engineMax").as_float(4000),
		xmlVehicleSubNode.attribute("steerLimit").as_float(0.78),
		xmlVehicleSubNode.attribute("brakeForce").as_float(2000),
		xmlVehicleSubNode.attribute("suspensionStiffness").as_float(20.0),
		xmlVehicleSubNode.attribute("suspensionDamping").as_float(2.3),
		xmlVehicleSubNode.attribute("suspensionCompression").as_float(4.4)
	);
	
	for (auto xmlSubNode : xmlNode.children("wheel")) {
		Ogre::SceneNode* wheelNode = static_cast<Ogre::SceneNode*>(MGE::OgreUtils::getNamedChildOfNode(
			mainSceneNode,
			xmlSubNode.attribute("onNode").as_string()
		));
		
		addWheel(
			wheelNode,
			xmlSubNode.attribute("suspensionRestLength").as_float(0.7),
			xmlSubNode.attribute("radius").as_float(0.5),
			xmlSubNode.attribute("isFront").as_bool(false),
			xmlSubNode.attribute("friction").as_float(1000),
			xmlSubNode.attribute("rollInfluence").as_float(0.1),
			xmlSubNode.attribute("suspensionStiffness").as_float(std::nan("1")),
			xmlSubNode.attribute("suspensionDamping").as_float(std::nan("1")),
			xmlSubNode.attribute("suspensionCompression").as_float(std::nan("1"))
		);
	}
	
	return true;
	#else
	return false;
	#endif
}

#ifdef USE_BULLET
void MGE::Car::initVehicle(
	btRigidBody* _carPhysics, btScalar _engineMax, btScalar _steerLimit, btScalar _brakeForce,
	btScalar suspensionStiffness, btScalar suspensionDamping, btScalar suspensionCompression
) {
	carPhysics = _carPhysics;
	engineMax  = _engineMax;
	steerLimit = _steerLimit;
	brakeForce = _brakeForce;
	
	throttle = 0;
	steering = 0;
	brakes   = 0;
	
	if (carPhysics) {
		LOG_INFO("init VEHICLE with:" <<
			" suspensionStiffness="   << suspensionStiffness <<
			" suspensionDamping="     << suspensionDamping <<
			" suspensionCompression=" << suspensionCompression <<
			" carPhysics="            << carPhysics
		);
		
		/// create car VEHICLE physics 
		carTuning.m_suspensionStiffness = suspensionStiffness;
		carTuning.m_suspensionDamping = suspensionDamping;
		carTuning.m_suspensionCompression = suspensionCompression;
		carVehicleRayCaster = new btDefaultVehicleRaycaster( MGE::Physics::getPtr()->getDynamicsWorld() );
		carVehicle = new btRaycastVehicle(carTuning, carPhysics, carVehicleRayCaster);
		carVehicle->setCoordinateSystem(0,1,2);
		carPhysics->setActivationState(DISABLE_DEACTIVATION);
		
		/// add car VEHICLE physics 
		MGE::Physics::getPtr()->getDynamicsWorld()->addVehicle(carVehicle);
	} else {
		LOG_WARNING("call initVehicle() without btRigidBody");
	}
}

void MGE::Car::addWheel(
	Ogre::SceneNode* node,
	btScalar suspensionRestLength, btScalar wheelRadius, bool isFrontWheel,
	btScalar wheelFriction, btScalar rollInfluence,
	btScalar suspensionStiffness, btScalar suspensionDamping, btScalar suspensionCompression
) {
	if (!carVehicle) {
		LOG_WARNING("call addWheel() without carVehicle");
		return;
	}
	
	LOG_INFO("Create wheel with:" <<
		" pos=("    << node->getPosition() + Ogre::Vector3(0, suspensionRestLength, 0)  << ")" <<
		" suspLen=" << suspensionRestLength <<
		" radius="  << wheelRadius <<
		" isFront=" << isFrontWheel
	);
	
	const btVector3 wheelDirectionCS0(0,-1,0);
	const btVector3 wheelAxleCS(1,0,0);
	
	carWheels.push_back(node);
	
	btWheelInfo& wheel = carVehicle->addWheel(
		BtOgre::Convert::toBullet(node->getPosition() + Ogre::Vector3(0, suspensionRestLength-1, 0)),
		wheelDirectionCS0,    wheelAxleCS,
		suspensionRestLength, wheelRadius,
		carTuning,            isFrontWheel
	);
	
	wheel.m_frictionSlip = wheelFriction;
	wheel.m_rollInfluence = rollInfluence;
	
	if (std::isnan(suspensionStiffness))
		wheel.m_suspensionStiffness = carTuning.m_suspensionStiffness;
	else
		wheel.m_suspensionStiffness = suspensionStiffness;
	
	if (std::isnan(suspensionDamping))
		wheel.m_wheelsDampingRelaxation = carTuning.m_suspensionDamping;
	else
		wheel.m_wheelsDampingRelaxation = suspensionDamping;
	
	if (std::isnan(suspensionCompression))
		wheel.m_wheelsDampingCompression = carTuning.m_suspensionCompression;
	else
		wheel.m_wheelsDampingCompression = suspensionCompression;
}

void MGE::Car::go(float accel, float turn, float brk, float dt) {
	if (accel>0)
		throttle += exp(-throttle);
	else if (accel==0)
		throttle -= 2.0*(exp(throttle)-exp(-throttle)) * dt;
	else
		throttle -= exp(-throttle);
		
	throttle = Ogre::Math::Clamp(throttle, -1.f, 1.f);
	
	if (turn==0)
		steering *= 0.95; // steering *= 1 - exp(-3);
	else
		steering += turn * dt;
	steering = Ogre::Math::Clamp(steering, -steerLimit, steerLimit);
	
	brakes += (brk>0 ? exp(-brakes*dt)*dt : -exp(dt)*dt);
	brakes = Ogre::Math::Clamp(brakes, 0.f, 1.f);
	
	if ( brakes > 0 && throttle < 0.0001)
		throttle=0;
	
	LOG_DEBUG("car.go"
		<< " accel=" << accel << " turn=" << turn << " brk=" << brk << " dt=" << dt
		<< " ==>> steering=" << steering << " throttle=" << throttle << " brakes=" << brakes
	);
	
	carVehicle->setSteeringValue(steering, 0);
	carVehicle->setSteeringValue(steering, 1);
	
	carVehicle->applyEngineForce(engineMax * throttle, 0);
	carVehicle->applyEngineForce(engineMax * throttle, 1);
	carVehicle->applyEngineForce(engineMax * throttle, 2);
	carVehicle->applyEngineForce(engineMax * throttle, 3);
	
	carVehicle->setBrake(brakeForce * brakes, 0);
	carVehicle->setBrake(brakeForce * brakes, 1);
	carVehicle->setBrake(brakeForce * brakes, 2);
	carVehicle->setBrake(brakeForce * brakes, 3);
	
	for (int i=0; i<carVehicle->getNumWheels(); ++i) {
		carVehicle->updateWheelTransform(i,true);
		btWheelInfo wi = carVehicle->getWheelInfo(i);
		
		Ogre::Quaternion q;
		Ogre::Quaternion q1;
		Ogre::Quaternion q2;
		if(wi.m_bIsFrontWheel)
			q.FromAngleAxis(Ogre::Radian(wi.m_steering), Ogre::Vector3::UNIT_Y);
		q1.FromAngleAxis(Ogre::Degree(i%2==0?180:0), Ogre::Vector3::UNIT_Y);
		q2.FromAngleAxis(Ogre::Radian(wi.m_rotation*(i%2==0?-1:1)), Ogre::Vector3::UNIT_X);
		
		carWheels[i]->setOrientation(q*q1*q2);
	}
}

#endif
