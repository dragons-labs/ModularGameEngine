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

#pragma   once

#include "BaseClasses.h"
#include "StringUtils.h"
#include "MessagesSystem.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "data/structs/BaseComponent.h"

namespace MGE { struct BaseActor; }
namespace MGE { struct Health; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief class for processing health
 */
struct HealthSubSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Unloadable,
	public MGE::Singleton<HealthSubSystem>
{
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// list of burning object
	std::set<MGE::Health*>   unwellObjects;
	
	/// @copydoc MGE::SaveableToXML::unload
	virtual bool unload() override;
	
	/// struct for actor dead info
	struct ActorDeathMsg;
	
	HealthSubSystem();
};

/**
 * @brief struct for actors health info
 */
struct Health :
	public MGE::BaseComponent
{
	/**
	 * @brief flags using to actor selection and filtering
	 */
	enum StatusFlags {
		/// is healthy
		IS_HEALTHY            = (1 << 0),
		/// is injured
		IS_DEAD_OR_DESTROY    = (1 << 1),
		/// is injured
		IS_INJURED            = (1 << 2),
		/// is panic
		IS_PANIC              = (1 << 3),
		
		/// mask for injurned status subinfo
		INJURED_SUB_INFO_MASK = 0xf0,
		/// is hidden injured
		IS_HIDDEN_INJURED     = (1 << 4),
		/// is carry injured
		IS_CARRY_INJURED      = (1 << 5),
	};
	
	/**
	 * @brief convert string notation of StatusFlags to numeric mask (single flag value)
	 * 
	 * @param[in] s  string to convert
	 */
	inline static uint8_t stringToStatusFlag(const std::string_view& s) {
		if (s == "IS_HEALTHY")               return IS_HEALTHY;
		else if (s == "IS_DEAD_OR_DESTROY")  return IS_DEAD_OR_DESTROY;
		else if (s == "IS_INJURED")          return IS_INJURED;
		else if (s == "IS_HIDDEN_INJURED")   return IS_HIDDEN_INJURED;
		else if (s == "IS_CARRY_INJURED")    return IS_CARRY_INJURED;
		else if (s == "IS_PANIC")            return IS_PANIC;
		return MGE::StringUtils::toNumeric<uint8_t>(s);
	}
	
	uint8_t status;
	
	/// current health level
	float health;
	/// maximum health level
	float healthMax;
	/// minimum health level (dead level)
	float healthMin;
	
	/// return true when actor is injured
	inline bool isInjured() {
		return (status & IS_INJURED);
	}
	
	/// return true when actor is dead
	inline bool isDead() {
		return (status & IS_DEAD_OR_DESTROY);
	}
	
	/// return health level from -1.0 (dead) to 1.0 (100% healthy)
	inline float getHealthLevel() {
		if (health <= 0)
			return (health - healthMin) / healthMin;
		else
			return health / healthMax;
	}
	
	/// return health level from 0.0 to 1.0 (100% healthy)
	/// return value less than 0.0 means that actor is injured - check results of getInjuredHealthLevel
	inline float getNormalHealthLevel() {
		return health / healthMax;
	}
	
	/// return injured level from 0.0 (dead) to 1.0 (almost healthy),
	/// return value less than 0.0 means that actor is healthy - check results of getNormalHealthLevel
	inline float getInjuredHealthLevel() {
		return (healthMin - health) / healthMin;
	}
	
	/// add @a val to current actor health
	void updateHealth(float val);
	
	/// update health level
	void process(float gameTimeStep);
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 9;
	
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
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	/// constructor
	Health(MGE::NamedObject* parent);
	
	/// destructor
	virtual ~Health();
};

struct HealthSubSystem::ActorDeathMsg : MGE::EventMsg  {
	/// message type string
	inline static const std::string_view MsgType = "ActorDeath"sv;
	
	/// @copydoc MGE::EventMsg::getType
	const std::string_view getType() const override final {
		return MsgType;
	}
	
	/// actor who died
	MGE::BaseActor* actor;
	
	/// constructor
	ActorDeathMsg(MGE::BaseActor* _actor) :
		actor(_actor)
	{}
};

/// @}

}
