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

#include "game/actions/Action.h"

#include "LogSystem.h"
#include "ScriptsSystem.h"
#include "MessagesSystem.h"

#include "Engine.h"

#include "game/actions/ActionPrototype.h"
#include "game/actions/ActionFactory.h" 
#include "game/actorComponents/World3DMovable.h"
#include "data/structs/BaseObject.inl"
#include "data/property/XmlUtils_Ogre.h"

MGE::Action::Action(MGE::ActionPrototype* a, uint32_t t) :
	timer(0.0), ready(false), do_not_save(false), type(t), owner(NULL)
{
	LOG_DEBUG("constructor Action");
	setPrototype(a);
}

MGE::Action::~Action() {
	LOG_DEBUG("destructor Action type=" << type << " owner=" << owner);
	
	if (owner) { // when action was started
		if (actionProto && !actionProto->scriptOnEnd.empty()) {
			MGE::ScriptsSystem::getPtr()->runObjectWithVoid(
				actionProto->scriptOnEnd.c_str(), pybind11::cast(owner), pybind11::cast(this)
			);
		}
		
		if (type == MGE::ActionPrototype::MOVING) {
			MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorMovingEventMsg(owner, false), owner );
		}
		if (type == MGE::ActionPrototype::START_MOVE || type == MGE::ActionPrototype::MOVE ||  type == MGE::ActionPrototype::MOVING) {
			MGE::World3DMovable* movableActor = owner->getComponent<MGE::World3DMovable>();
			if (movableActor) {
				LOG_DEBUG("Cancel MOVE");
				movableActor->cancelMove();
			}
		}
	}
}

int MGE::Action::_init(MGE::BaseActor* actor) {
	if (actionProto && !actionProto->scriptOnStart.empty()) {
		int ret = MGE::ScriptsSystem::getPtr()->runObjectWithCast<int>(
			actionProto->scriptOnStart.c_str(), 3, pybind11::cast(actor), pybind11::cast(this)
		);
		if (ret != INIT_DONE_OK)
			return ret;
	}
	owner = actor;
	return INIT_DONE_OK;
}

const std::string& MGE::Action::getScriptName() const {
	if (actionProto)
		return actionProto->name;
	else
		return scriptName;
}

void MGE::Action::setType(uint32_t t) {
	type = t;
}

void MGE::Action::setPrototype(MGE::ActionPrototype* a) {
	actionProto = a;
	if (actionProto)
		type = actionProto->type;
}

void MGE::Action::setPrototype(const std::string_view& name) {
	setPrototype( MGE::ActionFactory::getPtr()->getAction(name) );
}

void MGE::Action::setScriptName(std::string_view name) {
	scriptName = name;
}

void MGE::Action::storeToXML(pugi::xml_node& xmlNode) const {
	LOG_DEBUG("Action - store");
	
	if (do_not_save)
		return;
	
	if (actionProto)
		xmlNode.append_child("prototypeName") <<  actionProto->name;
	else
		xmlNode.append_child("prototypeName") <<  "NULL";
	xmlNode.append_child("type") << type;
	xmlNode.append_child("scriptName") << scriptName;
	xmlNode.append_child("timer") << timer;
	
	auto xmlSubNode = xmlNode.append_child("targetPoints");
	for (auto& p : targetPoints) {
		xmlSubNode.append_child("point") << p;
	}
	
	xmlSubNode = xmlNode.append_child("targetObjects");
	for (auto& p : targetObjects) {
		xmlSubNode.append_child("obj") << p;
	}
	
	xmlSubNode = xmlNode.append_child("toolObjects");
	for (auto& p : toolObjects) {
		xmlSubNode.append_child("tool") << p;
	}
};

MGE::Action::Action(const pugi::xml_node& xmlNode) {
	LOG_DEBUG("Action - restore");
	
	setPrototype( MGE::ActionFactory::getPtr()->getAction(
		xmlNode.child("prototypeName").text().as_string()
	) );
	type       = xmlNode.child("type").text().as_int();
	scriptName = xmlNode.child("scriptName").text().as_string();
	timer      = xmlNode.child("timer").text().as_float();
	ready      = xmlNode.child("ready").text().as_int();
	
	do_not_save = false;
	owner = NULL;
	
	for (auto xmlSubNode : xmlNode.child("targetPoints")) {
		targetPoints.push_back( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlSubNode) );
	}
	
	MGE::NamedObject::insertToCollection<>(
		xmlNode.child("targetObjects"),
		&targetObjects
	);
	MGE::NamedObject::insertToCollection<>(
		xmlNode.child("toolObjects"),
		&toolObjects
	);
}
