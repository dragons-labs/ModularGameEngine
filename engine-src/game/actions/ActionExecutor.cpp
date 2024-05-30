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

#include "game/actions/ActionExecutor.h"

#include "with.h"
#include "ConfigParser.h"
#include "Engine.h"

#include "game/actions/ActionQueue.h"
#include "game/actions/Action.h"
#include "game/actions/ActionPrototype.h"
#include "data/structs/factories/ComponentFactory.h"
#include "physics/TimeSystem.h"
#include "game/misc/PrimarySelection.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

MGE::ActionExecutor::ActionExecutor() :
	MGE::Unloadable(250)
{
	// register actors component
	MGE::ComponentFactory::getPtr()->registerComponent(MGE::ActionQueue::classID, "ActionQueue", MGE::ActionQueue::create);
	
	// register main loop listener for processing scene objects
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER_ACTIONS);
}

// not register via MGE_REGISTER_MODULE → created by ActionFactory MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG

bool MGE::ActionExecutor::unload() {
	activeActionQueue.clear();
	return true;
}

bool MGE::ActionExecutor::update(float gameTimeStep, float realTimeStep) {
	if (MGE::TimeSystem::getPtr()->realtimeTimer->timerIsPaused())
		return false;
	
	bool paused = MGE::TimeSystem::getPtr()->gameIsPaused();
	
	auto iter = activeActionQueue.begin(); // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
	while( iter != activeActionQueue.end() ) {
		_process(*(iter++), gameTimeStep, paused);
	}
	return true;
}


/*--------------------- ActionExecutor::_process() ---------------------*/

#include "rendering/audio-video/AnimationSystem.h"
#include "ScriptsSystem.h"

#include "data/structs/BaseActor.h"
#include "data/structs/factories/ActorFactory.h"
#include "data/structs/components/ObjectOwner.h"
#include "data/structs/factories/PrototypeFactory.h"
#include "game/actorComponents/World3DMovable.h"

void MGE::ActionExecutor::_process(MGE::ActionQueue* actionQueue, float gameTimeStep, bool paused) {
	// get current action
	MGE::Action*    action = actionQueue->getFirstAction();
	MGE::BaseActor* actor  = actionQueue->owner;
	
	DEBUG2_LOG("process action with type=" << action->getType() << " for actor=" << actor->getName() );
	
	if (paused && !(action->getType() & MGE::ActionPrototype::RUN_ON_PAUSE))
		return;
	
	// init action as running
	int initState = action->init(actor);
	switch (initState) {
		case MGE::Action::NOT_NEED_INIT:
		case MGE::Action::INIT_DONE_OK:
			break;
		case MGE::Action::INIT_NEED_RECALL:
			return;
		case MGE::Action::INIT_FAIL:
		default:
			actionQueue->clear();
			return;
	}
	
	//
	// actions that may be performed in parallel with other (moving, waiting, ...) or INTERNAL action
	//
	// when call finishAction() or clearActionQueue() we can't proceed other action,
	// because current action is ended and action pointer is invalid ... so we end processing this actor
	//
	if (action->getType() & MGE::ActionPrototype::MOVING) {
		MGE::World3DMovable* movableActor = actor->getComponent<MGE::World3DMovable>();
		int ret = movableActor->doMoveStep(gameTimeStep);
		if ( ret == 1 ) {
			LOG_INFO("action \"move\" finish successful");
			WITH_NOT_NULL(MGE::AnimationSystem::getPtr())->setAnimation(movableActor->getOgreSceneNode(), "idle", MGE::AnimationSystem::REPLACE, true);
			actionQueue->finishAction();
			return;
		} else if ( ret == 2 ) {
			LOG_ERROR("action \"move\" finish with error");
			WITH_NOT_NULL(MGE::AnimationSystem::getPtr())->setAnimation(movableActor->getOgreSceneNode(), "idle", MGE::AnimationSystem::REPLACE, true);
			actionQueue->clear();
			return;
		}
	}
	if (action->getType() & MGE::ActionPrototype::WAIT_FOR_READY_FLAG) {
		if (action->ready) {
			LOG_INFO("action \"wait for ready\" finish");
			actionQueue->finishAction();
			return;
		}
	}
	if (action->getType() & MGE::ActionPrototype::WAIT_FOR_TIMEOUT) {
		// on timeout break current action
		action->timer = action->timer - gameTimeStep;
		if (action->timer < 0) {
			LOG_INFO("action \"timer\" finish");
			actionQueue->finishAction();
			return;
		}
	}
	if (action->getType() & MGE::ActionPrototype::WAIT_FOR_NEXT_ACTION) {
		// when is next action break current action
		if (actionQueue->getLength() > 1) {
			LOG_INFO("action \"wait for action\" finish");
			actionQueue->finishAction();
			return;
		}
	}
	
	//
	// process other (enumerative) actions
	//
	switch (action->getType() & MGE::ActionPrototype::ENUMERATIVE_MASK) {
		case MGE::ActionPrototype::RUN_SCRIPT:
		{
			bool script_ret = MGE::ScriptsSystem::getPtr()->runObjectWithCast<bool>(
				action->getScriptName().c_str(), true,
				pybind11::cast(actor), pybind11::cast(action), gameTimeStep
			);
			
			// when script return true - remove action from queue
			if (script_ret) {
				LOG_INFO("action script \"" + action->getScriptName() + "\" finish");
				actionQueue->finishAction();
			} else {
				DEBUG2_LOG("action script continue");
			}
			break;
		}
		case MGE::ActionPrototype::EXIT :
		{
			LOG_INFO("Run EXIT action");
			
			// get world position of "ExitPoint" for actor
			MGE::World3DObject* w3dActor = actor->getComponent<MGE::World3DObject>();
			Ogre::Vector3 position = w3dActor->getWorldPosition();
			LOG_INFO("parent actor position is: " << position);
			position += actor->getPropertyValue<Ogre::Vector3>("ExitPointOffset", Ogre::Vector3::ZERO);
			LOG_INFO("initial new actor position is: " << position);
			
			for (auto& iter : action->toolObjects) {
				MGE::BaseActor*      newActor;
				MGE::World3DObject*  w3dNewActor;
				
				if (iter->getType() == MGE::BasePrototype::TypeName()) {
					// create actor based on prototype at "initial new actor position"
					newActor = MGE::ActorFactory::getPtr()->createActor(
						static_cast<MGE::BasePrototype*>(iter), Ogre::BLANKSTRING,
						position, Ogre::Quaternion::IDENTITY
					);
					w3dNewActor = newActor->getComponent<MGE::World3DObject>();
				} else {
					newActor = static_cast<MGE::BaseActor*>(iter);
					// move actor to "initial new actor position"
					w3dNewActor = newActor->getComponent<MGE::World3DObject>();
					w3dNewActor->setWorldPositionOnGround(position);
					// unhide actor
					MGE::SelectableObject*  selObj = newActor->getComponent<MGE::SelectableObject>();
					if (selObj) selObj->setAvailable(true);
				}
				
				// search free position
				std::pair<bool, Ogre::Vector3> res = MGE::RayCast::findFreePosition(w3dNewActor->getOgreSceneNode(), w3dNewActor->getAABB());
				position = res.second;
				LOG_DEBUG(" - findFreePosition results is: " << res.first << " / " << position);
				
				// put on ground at final (free) position
				position.y = 0;
				w3dNewActor->setWorldPositionOnGround(position);
				w3dNewActor->updateCachedTransform();
				LOG_DEBUG(" - final position is: " << w3dNewActor->getWorldPosition());
				
				// remove one object from current set, do NOT remove from future (was removed when add action to queue)
				actor->getComponent<MGE::ObjectOwner>()->update(iter, -1, 0); 
			}
			actionQueue->finishAction();
			break;
		}
		case MGE::ActionPrototype::ENTER :
		{
			LOG_INFO("Run ENTER action");
			
			MGE::BaseActor* targetActor = static_cast<MGE::BaseActor*>(*(action->targetObjects.begin()));
			// add one object to current and future set (target ObjectOwner list is NOT update when queue action)
			targetActor->getComponent<MGE::ObjectOwner>()->update(actor, 1, 1); 
			
			MGE::SelectableObject*  selObj = actor->getComponent<MGE::SelectableObject>();
			if (selObj) selObj->setAvailable(false);
			actionQueue->clear();
			break;
		}
		case MGE::ActionPrototype::GET_TOOLS :
		{
			LOG_INFO("Run GET_TOOL action");
			
			MGE::BaseActor* targetActor = static_cast<MGE::BaseActor*>(*(action->targetObjects.begin()));
			for (auto& iter : action->toolObjects) {
				// remove one object from current set, do NOT remove from future (was removed when add action to queue)
				targetActor->getComponent<MGE::ObjectOwner>()->update(iter, -1, 0);
				
				// add one object to current set, do NOT add to future (was added when add action to queue)
				actor->getComponent<MGE::ObjectOwner>()->update(iter, 1, 0);
				
				if (iter->getPropertyValue<bool>("needRecreateActor", false)) {
					/// @todo TODO.5: maybe we should protect action queue, owned object, etc from destroy in reCreateActor()
					MGE::ActorFactory::getPtr()->reCreateActor(
						actor,
						MGE::PrototypeFactory::getPtr()->getPrototype(
							iter->getPropertyValue<std::string>("newPrototypeName",  MGE::EMPTY_STRING),
							iter->getPropertyValue<std::string>("newPrototypeFile",  MGE::EMPTY_STRING),
							iter->getPropertyValue<std::string>("newPrototypeGroup", MGE::EMPTY_STRING)
						)
					);
					return; // actionQueue was destroyed
				}
			}
			
			actionQueue->finishAction();
			break;
		}
		case MGE::ActionPrototype::PUT_TOOLS :
		{
			LOG_INFO("Run PUT_TOOL action");
			
			MGE::BaseActor* targetActor = static_cast<MGE::BaseActor*>(*(action->targetObjects.begin()));
			for (auto& iter : action->toolObjects) {
				// remove one object from current set, do NOT remove from future (was removed when add action to queue)
				actor->getComponent<MGE::ObjectOwner>()->update(iter, -1, 0);
				
				// add one object to current set, do NOT add to future (was added when add action to queue)
				targetActor->getComponent<MGE::ObjectOwner>()->update(iter, 1, 0);
			}
			
			actionQueue->finishAction();
			break;
		}
		/// @todo TODO.6: action GOTO_POSE : goto point and set orientation
		///               - need indicate orientation by set semi-transparent mesh copy (via render group + compositor OR change material?)
		///               - need new action target type (set "ghost") and "ghost" movement/rotate implementation
		///               - need parametrisation of movableActor->initMove() and MGE::PathFinder::findPath() to set orientation
		case MGE::ActionPrototype::MOVE :
		{
			LOG_INFO("Run MOVE action");
			
			MGE::World3DMovable* movableActor = actor->getComponent<MGE::World3DMovable>();
			if (movableActor) {
				action->setType( MGE::ActionPrototype::START_MOVE );
				
				auto tmpAction = new MGE::Action( NULL, MGE::ActionPrototype::WAIT_FOR_READY_FLAG );
				tmpAction->do_not_save = true;
				actionQueue->addActionAtFront( tmpAction );
				
				movableActor->initMove(action->targetPoints.front()); // must be after add WAIT_FOR_READY_FLAG action
			} else {
				LOG_WARNING("can't move non movable actor");
				actionQueue->clear();
			}
			break;
		}
		case MGE::ActionPrototype::START_MOVE :
		{
			LOG_INFO("Run START MOVE action");
			
			MGE::World3DMovable* movableActor = actor->getComponent<MGE::World3DMovable>();
			if (movableActor) {
				int x = movableActor->moveIsReady();
				if (x < 0) {
					action->setType( MGE::ActionPrototype::MOVE ); // this can happen after read saved game
				} else if (x > 0) {
					WITH_NOT_NULL(MGE::AnimationSystem::getPtr())->setAnimation(movableActor->getOgreSceneNode(), "move", MGE::AnimationSystem::REPLACE, true);
					action->setType( MGE::ActionPrototype::MOVING );
					MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorMovingEventMsg(actor, true), actor );
			} else {
					LOG_WARNING("target not available !!!");
					actionQueue->clear();
				}
			} else {
				LOG_WARNING("can't move non movable actor");
				actionQueue->clear();
			}
			break;
		}
	}
}
