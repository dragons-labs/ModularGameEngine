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

namespace MGE { struct BaseActor; }
namespace MGE { struct NamedObject; }
namespace MGE { struct ActionPrototype; }

#include <OgreVector3.h>

#include <stdint.h>
#include <list>
#include <set>

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Struct for description queued actor actions
 */
struct Action {
	/// list of points
	std::list<Ogre::Vector3>    targetPoints;
	
	/// list of target objects
	std::set<MGE::BaseActor*>   targetObjects;
	
	/// list of tool objects
	std::set<MGE::NamedObject*> toolObjects;
	
	/// time in seconds to wait on this action (when @ref type & WAIT_FOR_TIMEOUT != 0)
	float                       timer;
	
	/// action mode (used for action with sub-menus)
	/// corresponding to index of selected entry in MGE::ActionPrototype::subMenuText
	int                         mode;
	
	/// when set finish this action (when @ref type & WAIT_FOR_READY_FLAG != 0)
	bool                        ready;
	
	/// when set action will not be store in game save file (default false)
	bool                        do_not_save;
	
	/// constructor - create action from action prototype
	Action(MGE::ActionPrototype* a = NULL, uint32_t t = 0);
	
	/// constructor from xml serialization archive
	Action(const pugi::xml_node& xmlNode);
	
	/// store to xml serialization archive
	void storeToXML(pugi::xml_node& xmlNode) const;
	void storeToXML(pugi::xml_node&& xmlNode) const { storeToXML(xmlNode); };
	
	/// destruktor
	~Action();
	
	/// return script name associated with this action
	const std::string& getScriptName() const;
	
	/// set script name associated with this action
	void setScriptName(std::string_view name);
	
	/// return action type, see @ref MGE::ActionPrototype::ActionType
	inline uint32_t getType() const {
		return type;
	}
	
	/// set action type, see @ref MGE::ActionPrototype::ActionType
	void setType(uint32_t t);
	
	/// return action prototype
	inline const MGE::ActionPrototype* getPrototype() const {
		return actionProto;
	}
	
	/// set action prototype (and - when @a a != NULL - action type based on prototype type)
	void setPrototype(MGE::ActionPrototype* a);
	
	/// set action prototype and action type based on prototype type
	void setPrototype(const std::string_view& name);
	
	/// init action (before executed), for return values see @ref InitState
	inline int init(MGE::BaseActor* actor) {
		if (owner) return NOT_NEED_INIT;
		return _init(actor);
	}
	
	/// return values for @ref init function
	enum InitState {
		/// action was init previously
		NOT_NEED_INIT    = 0,
		/// init done OK and action can be continue in current step
		INIT_DONE_OK     = 1,
		/// init should be call again in next actions step
		INIT_NEED_RECALL = 2,
		/// init fail, action queue should be clean
		INIT_FAIL        = 3
	};
	
protected:
	/// pointer to action description struct, see @ref MGE::ActionPrototype
	/// when NULL action is hidden/silent action
	MGE::ActionPrototype*  actionProto;
	
	/// name of script to run (used when action is NULL)
	std::string            scriptName;
	
	/// action type, see @ref MGE::ActionPrototype::ActionType
	/// @note This is typically the same type as in the action prototype (if set) but sometimes can be overwritten
	uint32_t               type;
	
	/// pointer to  actor owned this action (when not NULL action start executed)
	MGE::BaseActor*        owner;
	
	/// real init action (before executed)
	int _init(MGE::BaseActor* actor);
};

/// @}

}
