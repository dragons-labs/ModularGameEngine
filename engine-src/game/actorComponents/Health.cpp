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

#include "game/actorComponents/Health.h"

#include "ConfigParser.h"
#include "Engine.h"

#include "data/structs/factories/ComponentFactory.h"
#include "physics/TimeSystem.h"
#include "data/structs/BaseActor.h"

/*--------------------- HealthSubSystem ---------------------*/

MGE::HealthSubSystem::HealthSubSystem() :
	MGE::Unloadable(250)
{
	// register actors component
	MGE::ComponentFactory::getPtr()->registerComponent(MGE::Health::classID, "Health", MGE::Health::create);
	
	// register main loop listener for processing scene objects
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER_ACTIONS);
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_FireSystem \<HealthSystem\>

@c \<HealthSystem\> is used for creating <b>Health System</b> used by @ref ActorComponent_Health (including register this component).
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(HealthSystem) {
	if (!MGE::HealthSubSystem::getPtr())
		return new MGE::HealthSubSystem();
	return nullptr;
}

bool MGE::HealthSubSystem::unload() {
	unwellObjects.clear();
	return true;
}

bool MGE::HealthSubSystem::update(float gameTimeStep, float realTimeStep) {
	if (gameTimeStep == 0.0f)
		return false;
	
	for(auto& iter : unwellObjects) {
		iter->process(gameTimeStep);
	}
	return true;
}


/*--------------------- Health ---------------------*/

bool MGE::Health::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_child("health") << health;
	xmlNode.append_child("status") << status;
	xmlNode.append_child("healthMax") << healthMax;
	xmlNode.append_child("healthMin") << healthMin;
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Health Health

Store / restore from its @c \<Component\> node subnodes:
  - @c \<health\>
    - current value of health
  - @c \<healthMax\>
    - maximum health level (positive value)
  - @c \<healthMin\>
    - minimum health level (zero or negative value), aka dead level
  - @c \<status\>
    - (optional) numeric health status value (see @ref MGE::Health::StatusFlags),
    - use zero (default) for calculate status based on health level

See @ref MGE::Health for details.
*/
bool MGE::Health::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	auto xmlSubNode = xmlNode.child("health");
	
	if (xmlSubNode) {
		health    = xmlSubNode.text().as_float();
		status    = xmlNode.child("status").text().as_int(0);
		healthMax = xmlNode.child("healthMax").text().as_float(healthMax);
		healthMin = xmlNode.child("healthMin").text().as_float(healthMin);
		if (!status) {
			updateHealth(0); // set status flags
		}
	}
	return true;
}

MGE::BaseComponent* MGE::Health::create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID) {
	typeIDs->insert(classID);
	return new MGE::Health(parent);
}

MGE::Health::Health(MGE::NamedObject* parent) :
	status      (IS_HEALTHY),
	health      (100),
	healthMax   (100),
	healthMin   (-50),
	owner( static_cast<MGE::BaseActor*>(parent) )
{}

MGE::Health::~Health() {
	MGE::HealthSubSystem::getPtr()->unwellObjects.erase(this);
}

void MGE::Health::updateHealth(float val) {
	health = health + val;
	
	if (health < healthMin) {
		if (status != IS_DEAD_OR_DESTROY) {
			status = IS_DEAD_OR_DESTROY;
			MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::HealthSubSystem::ActorDeathMsg(owner), owner );
		}
		health = healthMin;
	} else if (health < 0 && !(status & IS_INJURED)) {
		status = (status & INJURED_SUB_INFO_MASK) | IS_INJURED;
		MGE::HealthSubSystem::getPtr()->unwellObjects.insert(this);
	} else if (status & IS_INJURED && health > 0) {
		// after injured do not return to normal healthy on scene
		health = 0;
	} else if (health > healthMax) {
		health = healthMax;
	}
}

void MGE::Health::process(float gameTimeStep) {
	if (health < 0) {
		health = health -0.2 * gameTimeStep;
		
		if (health < healthMin) {
			if (status != IS_DEAD_OR_DESTROY) {
				status = IS_DEAD_OR_DESTROY;
				MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::HealthSubSystem::ActorDeathMsg(owner), owner );
			}
			health = healthMin;
		}
	}
}
