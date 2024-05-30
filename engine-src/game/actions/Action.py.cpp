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

#include "game/actions/Action.h"

#include "ScriptsInterface.h"

#include "data/structs/BaseObject.py.h"
#include "data/structs/BaseActor.py.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::Action* actionCreate(int _type, float _timer) {
		MGE::Action* action = new MGE::Action(NULL, _type);
		action->timer  = _timer;
		
		return action;
	}
	
	uintptr_t getUniqueID(const MGE::Action& a) {
		return reinterpret_cast<uintptr_t>(&a);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(Action) {
	m.def("createAction", actionCreate,
		py::return_value_policy::reference,
		"create and return action"
	);
	
	py::enum_<MGE::Action::InitState>(
		m, "ActionInitState", DOC(MGE, Action, InitState)
	)
		.value("NOT_NEED_INIT",    MGE::Action::NOT_NEED_INIT)
		.value("INIT_DONE_OK",     MGE::Action::INIT_DONE_OK)
		.value("INIT_NEED_RECALL", MGE::Action::INIT_NEED_RECALL)
		.value("INIT_FAIL",        MGE::Action::INIT_FAIL)
		//.export_values()
	;
	
	py::class_<MGE::Action, std::unique_ptr<MGE::Action, py::nodelete>>(
		m, "Action", DOC(MGE, Action)
	)
		.def("getID",                     &getUniqueID,
			"return unique ID of C++ object"
		)
		.def("getScriptName",             &MGE::Action::getScriptName,
			DOC(MGE, Action, getScriptName)
		)
		.def("setScriptName",             &MGE::Action::setScriptName,
			DOC(MGE, Action, setScriptName)
		)
		.def("getType",                   &MGE::Action::getType,
			DOC(MGE, Action, getType)
		)
		.def("setType",                   &MGE::Action::setType,
			DOC(MGE, Action, setType)
		)
		.def("setPrototype",              static_cast< void(MGE::Action::*)(const std::string_view&) >(&MGE::Action::setPrototype),
			DOC(MGE, Action, setPrototype, 2)
		)
		.def_readwrite("targetPoints",    &MGE::Action::targetPoints,
			DOC(MGE, Action, targetPoints)
		)
		.def_readwrite("targetObjects",   &MGE::Action::targetObjects,
			DOC(MGE, Action, targetObjects)
		)
		.def_readwrite("toolObjects",     &MGE::Action::toolObjects,
			DOC(MGE, Action, toolObjects)
		)
		.def_readwrite("timer",           &MGE::Action::timer,
			DOC(MGE, Action, timer)
		)
		.def_readwrite("mode",            &MGE::Action::mode,
			DOC(MGE, Action, mode)
		)
		.def_readwrite("ready",           &MGE::Action::ready,
			DOC(MGE, Action, ready)
		)
		.def_readwrite("do_not_save",     &MGE::Action::do_not_save,
			DOC(MGE, Action, do_not_save)
		)
	;
}
#endif
