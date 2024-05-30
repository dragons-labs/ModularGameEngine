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

#include "game/actions/ActionQueue.h"

#include "LogSystem.h"
#include "Engine.h"

#include "physics/TimeSystem.h"
#include "data/structs/BaseActor.h"
#include "game/actions/Action.h"
#include "game/actions/ActionPrototype.h"
#include "game/actions/ActionExecutor.h"
#include "data/property/XmlUtils_Ogre.h"

void MGE::ActionQueue::addActionAtFront(MGE::Action* action) {
	queue.push_front( action );
	MGE::ActionExecutor::getPtr()->activeActionQueue.insert(this);
	
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( ActionQueueUpdateEventMsg(owner), owner );
	LOG_DEBUG("addActionAtFront done");
}

void MGE::ActionQueue::addActionAtEnd(MGE::Action* action) {
	if (action->getType() & MGE::ActionPrototype::ADD_AT_FRONT) {
		LOG_DEBUG("addActionAtEnd call addActionAtFront due to ADD_AT_FRONT flag");
		return addActionAtFront(action);
	}
	
	queue.push_back( action );
	MGE::ActionExecutor::getPtr()->activeActionQueue.insert(this);
	
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( ActionQueueUpdateEventMsg(owner), owner );
	LOG_DEBUG("addActionAtEnd for " << owner->getName() << " done ... queue len = " << queue.size());
}

void MGE::ActionQueue::finishAction() {
	LOG_DEBUG("remove single action from queue");
	delete queue.front();

	queue.pop_front();
	
	if (isEmpty()) {
		LOG_DEBUG("remove action queue from set of active action queue");
		MGE::ActionExecutor::getPtr()->activeActionQueue.erase(this);
	}
	
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( ActionQueueUpdateEventMsg(owner), owner );
}

void MGE::ActionQueue::clear(bool fullClear) {
	LOG_DEBUG("clear queue: fullClear=" << fullClear);
	if (fullClear) {
		for (auto& iter : queue) {
			delete iter;
		}
	}
	queue.clear();
	
	MGE::ActionExecutor::getPtr()->activeActionQueue.erase(this);
	
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( ActionQueueUpdateEventMsg(owner), owner );
}

MGE::ActionQueue::ActionQueue(MGE::NamedObject* parent) {
	owner = static_cast<MGE::BaseActor*>(parent);
}

MGE::BaseComponent* MGE::ActionQueue::create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID) {
	typeIDs->insert(classID);
	return new MGE::ActionQueue(parent);
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_ActionQueue ActionQueue

Store / restore from its @c \<Component\> node set of @c \<Action\> subnodes. Each @c \<Action\> uses subnodes:
  - @c \<prototypeName\>
    - action prototype name, use string @c NULL (case sensitive) for action without prototype
  - @c \<type\>
    - numeric type id of action (see @ref MGE::ActionPrototype::ActionType)
  - @c \<scriptName\>
    - action script name
  - @c \<timer\>
    - action timer value (floating point number)
  - @c \<targetPoints\>
    - list of target points (list/set of \<item\> nodes with @ref XML_Vector3 syntax)
  - @c \<targetObjects\>
    - list of action target object (stored as list/set of @ref XMLNode_ActorName xor @ref XMLNode_PrototypeRef nodes)
  - @c \<toolObjects\>
    - list of action tools object (stored as list/set of @ref XMLNode_ActorName xor @ref XMLNode_PrototypeRef nodes)
*/
bool MGE::ActionQueue::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	for (auto xmlSubNode : xmlNode.child("Actions")) {
		queue.push_back( new MGE::Action( xmlSubNode ) );
	}
	
	if (! isEmpty() ) {
		MGE::ActionExecutor::getPtr()->activeActionQueue.insert(this);
	}
	
	lastUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	
	return true;
}

bool MGE::ActionQueue::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	auto xmlSubNode = xmlNode.append_child("Actions");
	for (auto q : queue) {
		q->storeToXML( xmlSubNode.append_child("Action") );
	}
	
	return true;
}
