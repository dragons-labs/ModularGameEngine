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

#include "game/actorComponents/FlammableObject.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "Engine.h"

#include "data/structs/BaseActor.h"
#include "data/structs/components/3DWorld.h"
#include "data/structs/factories/ComponentFactory.h"
#include "physics/TimeSystem.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

/*--------------------- FireSubSystem ---------------------*/

MGE::FireSubSystem::FireSubSystem() :
	MGE::Unloadable(250)
{
	// register actors component
	MGE::ComponentFactory::getPtr()->registerComponent(MGE::FlammableObject::classID, "FlammableObject", MGE::FlammableObject::create);
	
	// register main loop listener for processing scene objects
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER_ACTIONS);
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_FireSystem \<FireSystem\>

@c \<FireSystem\> is used for creating <b>Fire System</b> used by @ref ActorComponent_FlammableObject (including register this component).
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(FireSystem) {
	if (!MGE::FireSubSystem::getPtr())
		return new MGE::FireSubSystem();
	return nullptr;
}

bool MGE::FireSubSystem::unload() {
	objectsOnFire.clear();
	return true;
}

bool MGE::FireSubSystem::update(float gameTimeStep, float realTimeStep) {
	if (gameTimeStep == 0.0f)
		return false;
	
	for(auto& iter : objectsOnFire) {
		iter->process(gameTimeStep);
	}
	return true;
}


/*--------------------- FlammableObject ---------------------*/

bool MGE::FlammableObject::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_child("isFlammable") << isFlammable;
	xmlNode.append_child("flashPoint") << flashPoint;
	xmlNode.append_child("fireTemperature") << fireTemperature;
	xmlNode.append_child("explosionPoint") << explosionPoint;
	xmlNode.append_child("isOnFire") << isOnFire;
	xmlNode.append_child("fuelLevel") << fuelLevel;
	xmlNode.append_child("temperature") << temperature;
	xmlNode.append_child("timeToExplosion") << timeToExplosion;
	xmlNode.append_child("coolingEfficiency") << coolingEfficiency;
	
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_FlammableObject FlammableObject

Store / restore from its @c \<Component\> node subnodes:
  - @c \<isFlammable\>
  - @c \<flashPoint\>
  - @c \<fireTemperature\>
  - @c \<explosionPoint\>
  - @c \<isOnFire\>
  - @c \<fuelLevel\>
  - @c \<temperature\>
  - @c \<timeToExplosion\>
  - @c \<coolingEfficiency\>

See @ref MGE::FlammableObject for details.
*/
bool MGE::FlammableObject::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	isFlammable = xmlNode.child("isFlammable").text().as_bool();
	flashPoint = xmlNode.child("flashPoint").text().as_float();
	fireTemperature = xmlNode.child("fireTemperature").text().as_float();
	explosionPoint = xmlNode.child("explosionPoint").text().as_float();
	
	isOnFire = xmlNode.child("isOnFire").text().as_bool();
	fuelLevel = xmlNode.child("fuelLevel").text().as_float();
	temperature = xmlNode.child("temperature").text().as_float();
	timeToExplosion = xmlNode.child("timeToExplosion").text().as_float();
	coolingEfficiency = xmlNode.child("coolingEfficiency").text().as_float();
	
	return true;
}


MGE::BaseComponent* MGE::FlammableObject::create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID) {
	typeIDs->insert(classID);
	return new MGE::FlammableObject(parent);
}

MGE::FlammableObject::FlammableObject(MGE::NamedObject* parent) : 
	isFlammable (false)
{
	owner = static_cast<MGE::BaseActor*>(parent);
}

MGE::FlammableObject::~FlammableObject() {
	MGE::FireSubSystem::getPtr()->objectsOnFire.erase(this);
}

void MGE::FlammableObject::setFire(int state) {
	if (state == -1) {
		fuelLevel = 0;
		temperature = 0;
		unsetOnFire();
	} else if (state == 0) {
		temperature = 0;
		unsetOnFire();
	} else if (state == 1) {
		temperature = flashPoint;
		setOnFire();
	} else if (state == 2) {
		temperature = fireTemperature;
		setOnFire();
	}
}

void MGE::FlammableObject::process(float gameTimeStep) {
	if(!isFlammable)
		return;
	
	DEBUG2_LOG("T[" << owner->getName() << "] = " << temperature << " onFire=" << isOnFire);
	
	for (auto& iter : MGE::FireSubSystem::getPtr()->objectsOnFire) {
		if (iter->owner == owner)
			break;
		float distance = owner->getComponent<MGE::World3DObject>()->getWorldPosition().distance(
			iter->owner->getComponent<MGE::World3DObject>()->getWorldPosition()
		) + 0.001;
		float newTemperature = iter->temperature / distance;
		DEBUG2_LOG("   " << distance << " " << iter->temperature << " " << newTemperature);
		if (newTemperature > temperature) {
			temperature = newTemperature;
		}
	}
	
	if (temperature > 0) {
		temperature -= 3;
		/// @todo TODO.5: use coolingEfficiency
	}
	
	if (isOnFire) {
		fuelLevel -= 1;
		if (temperature < fireTemperature) {
			temperature += 6;
		} else if (temperature < flashPoint || fuelLevel <= 0) {
			unsetOnFire();
		}
	} else if (temperature > flashPoint && fuelLevel > 0) {
		setOnFire();
	}
}
