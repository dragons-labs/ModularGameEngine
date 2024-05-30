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

#include "data/structs/BaseComponent.h"
#include "MessagesSystem.h"

namespace MGE { struct Action; }
namespace MGE { struct BaseActor; }
namespace MGE { struct ActionExecutor; }

#include <chrono>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements queue of actions (Action) for Actor
 */
struct ActionQueue :
	public MGE::BaseComponent
{
public:
	/// struct for action queue update message (is sending per actor with updated action queue, @b after update his queue)
	struct ActionQueueUpdateEventMsg;
	
	/**
	 * @brief return true if actionQueue is empty
	 */
	inline bool isEmpty() const {
		return queue.empty();
	}
	
	/**
	 * @brief return length of actionQueue
	 */
	inline int getLength() const {
		return queue.size();
	}
	
	/**
	 * @brief return first Action in actionQueue
	 */
	inline MGE::Action* getFirstAction() const {
		if (queue.empty())
			return NULL;
		else
			return queue.front();
	}
	
	/**
	 * @brief add action at front of the action queue and add actor to actionQueuedActors list
	 * 
	 * @param[in]     action - action name
	 */
	void addActionAtFront(MGE::Action* action);
	
	/**
	 * @brief add action at end of the action queue and add actor to actionQueuedActors list
	 * 
	 * @param[in]     action - action name
	 */
	void addActionAtEnd(MGE::Action* action);
	
	/**
	 * @brief remove all actions from actor queue and remove actor form actionQueuedActors list
	 */
	void clear(bool fullClear = true);
	
	/**
	 * @brief remove FIRST action from actor queue
	 *        and (when no more actions in queue) remove actor form actionQueuedActors list
	 */
	void finishAction();
	
	/// return begin queue iterator
	inline std::list<MGE::Action*>::const_iterator begin() {
		return queue.cbegin();
	}
	
	/// return end queue iterator
	inline std::list<MGE::Action*>::const_iterator end() {
		return queue.cend();
	}
	
	/**
	 * @brief return value of lastUpdateTime
	 */
	std::chrono::time_point<std::chrono::steady_clock> getLastUpdateTime() { return lastUpdateTime; }
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 4;
	
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
	
protected:
	friend struct ActionExecutor;
	
	/// constructor
	ActionQueue(MGE::NamedObject* parent);
	
	/// destructor
	virtual ~ActionQueue() { }
	
	/// List of ActionQueue::Action structs for actor action queue
	std::list<MGE::Action*> queue;
	
	/// pointer to owner 
	MGE::BaseActor* owner;
	
	/// last time of queue modification from @ref MGE::Engine::getMainLoopTime
	/// used to determine the necessary of updating queue information in other class
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
};

struct MGE::ActionQueue::ActionQueueUpdateEventMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActionQueueUpdate"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// actor with updated action queue
	MGE::BaseActor* actor;
	
protected:
	friend struct ActionQueue;
	
	/// constructor
	ActionQueueUpdateEventMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};

/// @}

}
