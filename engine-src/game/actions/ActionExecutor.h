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
#include "MainLoopListener.h"
#include "ModuleBase.h"

namespace MGE { struct ActionQueue; }

#include <set>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief class for processing actions
 */
struct ActionExecutor :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Unloadable,
	public MGE::Singleton<ActionExecutor>
{
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// list of active (non empty) action queues
	std::set<MGE::ActionQueue*>    activeActionQueue;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
	ActionExecutor();
	
private:
	void _process(MGE::ActionQueue* actionQueue, float gameTimeStep, bool paused);
};

/// @}

}
