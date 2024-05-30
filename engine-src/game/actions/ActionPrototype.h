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

#include "StringUtils.h"
#include "game/misc/ActorFilter.h"

namespace MGE { class ActionFactory; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Struct for description actor actions
 */
struct ActionPrototype {
	/// types of actions, up to ENUMERATIVE_MASK value (0xffff) is enumerate, over 0xffff is bit-mask
	enum ActionType {
		/// do nothing
		EMPTY = 0,
		/// run script
		RUN_SCRIPT,
		/// move to point
		MOVE,
		/// real start move to point (after pathfinding, internal use)
		START_MOVE,
		/// exit from vehicle, building, etc (create or show actor, ...)
		EXIT,
		/// enter to vehicle, building, etc (hide actor, ...)
		/// this is only "ENTER" action, goto "entry point", enter animation, etc must be doing externaley (e.g. from scripts system)
		ENTER,
		/// select tool via ActorInfo window
		SELECT_TOOL,
		/// getting tool from vehicle, etc
		/// this is only "GET" action, goto "entry point", getting animation, etc must be doing externaley (e.g. from scripts system)
		GET_TOOLS,
		/// putting (eturn) tool to vehicle, etc
		/// this is only "PUT" action, goto "entry point", getting animation, etc must be doing externaley (e.g. from scripts system)
		PUT_TOOLS,
		
		/// mask for selecting enumerative part of action type
		ENUMERATIVE_MASK = 0x0000ffff,
		
		/// continue curent action util flag is not set
		WAIT_FOR_READY_FLAG = 1 << 21,
		/// continue curent action (eg animation) for N ms
		WAIT_FOR_TIMEOUT = 1 << 22,
		/// continue curent action (eg animation) until in queue is not new action
		WAIT_FOR_NEXT_ACTION = 1 << 23,
		/// actor is curently moving (doing move step)
		MOVING = 1 << 24,
		
		/// this action will be add at front of query (even before current executed action);
		/// when use with RUN_ON_PAUSE action will be run immediately
		ADD_AT_FRONT = 1 << 30,
		/// this action (when is first in query) can be run on interactive (not on full) pause
		RUN_ON_PAUSE = 1 << 31,
	};
	
	/**
	 * @brief Convert string notation of ActionType to numeric value
	 * 
	 * @param s        string to convert
	 */
	inline static uint32_t stringToActionTypeFlag(const std::string_view& s) {
		if (s == "EMPTY")                      return EMPTY;
		else if (s == "RUN_SCRIPT")            return RUN_SCRIPT;
		else if (s == "MOVE")                  return MOVE;
		else if (s == "EXIT")                  return EXIT;
		else if (s == "ENTER")                 return ENTER;
		else if (s == "SELECT_TOOL")           return SELECT_TOOL;
		else if (s == "GET_TOOLS")             return GET_TOOLS;
		else if (s == "PUT_TOOLS")             return PUT_TOOLS;
		else if (s == "WAIT_FOR_READY_FLAG")   return WAIT_FOR_READY_FLAG;
		else if (s == "WAIT_FOR_TIMEOUT")      return WAIT_FOR_TIMEOUT;
		else if (s == "WAIT_FOR_NEXT_ACTION")  return WAIT_FOR_NEXT_ACTION;
		else if (s == "MOVING")                return MOVING;
		else if (s == "ADD_AT_FRONT")          return ADD_AT_FRONT;
		else if (s == "RUN_ON_PAUSE")          return RUN_ON_PAUSE;
		return MGE::StringUtils::toNumeric<uint32_t>(s);
	}
	
	/// action target "need" type bit flags
	enum TargetType {
		/// no required
		NEED_NONE              = 0,
		/// need single point
		NEED_POINT             = (1 << 0),
		/// need rectangular
		NEED_AREA              = (1 << 1),
		/// need polygonal-chain
		NEED_POLYGONAL_CHAIN   = (1 << 2),
		/// need single actors
		NEED_TARGET_ACTOR      = (1 << 3),
		/// need single actors
		NEED_SELECTABLE_ACTOR  = (1 << 5),
		/// mask for need multiple or single actors
		NEED_ACTOR             = NEED_TARGET_ACTOR | NEED_SELECTABLE_ACTOR,
	};
	
	/**
	 * @brief Convert string notation of TargetType to numeric value
	 * 
	 * @param s       string to convert
	 */
	inline static uint32_t stringToTargetTypeFlag(const std::string_view& s) {
		if (s == "NONE")                   return NEED_NONE;
		else if (s == "POINT")             return NEED_POINT;
		else if (s == "AREA")              return NEED_AREA;
		else if (s == "POLYGONAL_CHAIN")   return NEED_POLYGONAL_CHAIN;
		else if (s == "TARGET_ACTOR")      return NEED_TARGET_ACTOR;
		else if (s == "SELECTABLE_ACTOR")  return NEED_SELECTABLE_ACTOR;
		return MGE::StringUtils::toNumeric<uint32_t>(s);
	}
	
	/// unique name used to identifies action, and identifies script to execute
	std::string                  name;
	
	/// action type, see @ref ActionType
	uint32_t                     type;
	
	/// name of script to execute (when not empty) when action is start executed (before first time executed)
	std::string                  scriptOnStart;
	
	/// name of script to execute (when not empty and @ref MGE::Action::running == true) when action is finished / destroyed
	std::string                  scriptOnEnd;
	
	/// this action needs target type @ref TargetType
	uint32_t                     needMask;
	
	/// action name for use in menu, etc
	std::string                  menuText;
	
	/// pointer to map of sub menu texts (sub menu is used when subMenuText != NULL)
	std::map<int, std::string>*  subMenuText;
	
	/// action icon for use in menu, etc
	std::string                  menuIcon;
	
	/// filter for supported action executor actors
	MGE::ActorFilter             executorFilter;
	
	/// filter for supported action target actors
	MGE::ActorFilter             targetFilter;
	
	/**
	 * @brief return true if actor support (can emit) action
	 * 
	 * @param actor       pointer to actor to check
	 * @param actionName  name of action to check
	 * @param fullCheck   when true do full test (check actor properties and components)
	 */
	static bool actorCanEmmitAction(
		const MGE::BaseActor* actor,
		const std::string_view& actionName,
		bool fullCheck = true
	);
	
	/**
	 * @brief return true if action of this prototype can be emit (is supported) by actor
	 * 
	 * @param actor       pointer to actor to check
	 * @param fullCheck   when true do full test (check actor properties and components)
	 */
	inline bool canBeEmitBy(MGE::BaseActor* actor, bool fullCheck = true) const {
		return actorCanEmmitAction(actor, name, fullCheck);
	}
	
	/**
	 * @brief return true if @a actor is valid target for action of this prototype
	 * 
	 * @param actor       pointer to actor to check
	 * 
	 * @note function return true when action do not need actor as target
	 */
	inline bool isValidTarget(MGE::BaseActor* actor) const {
		if (needMask & MGE::ActionPrototype::NEED_ACTOR)
			return targetFilter.fullCheck(actor);
		else
			return true;
	}
	
protected:
	friend class  MGE::ActionFactory;
	
	/**
	 * @brief constructor
	 * 
	 * @param  xmlArchive   xml archive object, with pointer to xml node which will be using for load state of this object
	 */
	ActionPrototype(const pugi::xml_node& xmlNode);
	
	/// destructor
	~ActionPrototype();
	
	/// initialize python interface for this class
	static void initScriptsInterface();
	
	/// priority used to deal with duplicated actions with this same name
	int priority;
};

/// @}

}
