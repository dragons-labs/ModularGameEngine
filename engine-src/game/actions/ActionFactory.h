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
#include "StringTypedefs.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

namespace MGE { struct ActionPrototype; }

namespace Ogre { class SceneManager; }

#include <unordered_map>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief class for creating ActionPrototype
 */
class ActionFactory :
	public MGE::Unloadable,
	public MGE::Module,
	public MGE::Singleton<ActionFactory>
{
public:
	/// create and load (based on config file) or get existed action identified by name
	MGE::ActionPrototype* getAction(const std::string_view& name);
	
	/// constructor
	ActionFactory(const pugi::xml_node& xmlNode);
	
protected:
	/// destructor
	~ActionFactory();
	
	/// list of All loaded actions
	std::unordered_map<std::string, MGE::ActionPrototype*, MGE::string_hash, std::equal_to<>>  allActions;
	
	void loadActionsFromFile(const std::string& file);
};

/// @}

}
