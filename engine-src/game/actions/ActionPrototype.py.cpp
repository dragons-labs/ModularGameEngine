/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "game/actions/ActionPrototype.h"

#include "ScriptsInterface.h"

#include "data/structs/BaseActor.h"
#include "game/actions/Action.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(ActionPrototype) {
	m.def("actorCanEmmitAction",    &MGE::ActionPrototype::actorCanEmmitAction,
		DOC(MGE, ActionPrototype, actorCanEmmitAction)
	);
	py::enum_<MGE::ActionPrototype::ActionType>(
		m, DOC(MGE, ActionPrototype, ActionType)
	)
		.value("EMPTY",                MGE::ActionPrototype::EMPTY)
		.value("RUN_SCRIPT",           MGE::ActionPrototype::RUN_SCRIPT)
		.value("MOVE",                 MGE::ActionPrototype::MOVE)
		.value("EXIT",                 MGE::ActionPrototype::EXIT)
		.value("ENTER",                MGE::ActionPrototype::ENTER)
		.value("SELECT_TOOL",          MGE::ActionPrototype::SELECT_TOOL)
		.value("GET_TOOLS",            MGE::ActionPrototype::GET_TOOLS)
		.value("PUT_TOOLS",            MGE::ActionPrototype::PUT_TOOLS)
		.value("ENUMERATIVE_MASK",     MGE::ActionPrototype::ENUMERATIVE_MASK)
		.value("WAIT_FOR_TIMEOUT",     MGE::ActionPrototype::WAIT_FOR_TIMEOUT)
		.value("WAIT_FOR_NEXT_ACTION", MGE::ActionPrototype::WAIT_FOR_NEXT_ACTION)
		.value("MOVING",               MGE::ActionPrototype::MOVING)
		//.export_values()
	;
}
#endif
