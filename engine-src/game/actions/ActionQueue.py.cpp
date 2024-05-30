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

#include "game/actions/ActionQueue.h"

#include "ScriptsInterface.h"

// no Ogre in this API #include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"
#include <pybind11/chrono.h>

#include "data/structs/BaseActor.h"
#include "game/actions/Action.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::ActionQueue* getActionQueueFromActor(MGE::BaseActor* gameObj) {
		return gameObj->getComponent<MGE::ActionQueue>();
	}
} }

MGE_SCRIPT_API_FOR_MODULE(ActionQueue, 17) {
	py::class_< MGE::ActionQueue, MGE::BaseComponent, std::unique_ptr<MGE::ActionQueue, py::nodelete> >(
		m, "ActionQueue", DOC(MGE, ActionQueue)
	)
		.def("isEmpty",                   &MGE::ActionQueue::isEmpty,
			DOC(MGE, ActionQueue, isEmpty)
		)
		.def("getLength",                 &MGE::ActionQueue::getLength,
			DOC(MGE, ActionQueue, getLength)
		)
		.def("getFirstAction",            &MGE::ActionQueue::getFirstAction,
			py::return_value_policy::reference,
			DOC(MGE, ActionQueue, getFirstAction)
		)
		.def("addActionAtFront",          &MGE::ActionQueue::addActionAtFront,
			DOC(MGE, ActionQueue, addActionAtFront)
		)
		.def("addActionAtEnd",            &MGE::ActionQueue::addActionAtEnd,
			DOC(MGE, ActionQueue, addActionAtEnd)
		)
		.def("clear",                     &MGE::ActionQueue::clear,
			DOC(MGE, ActionQueue, clear)
		)
		.def("finishAction",              &MGE::ActionQueue::finishAction,
			DOC(MGE, ActionQueue, finishAction)
		)
		.def("getLastUpdateTime",         &MGE::ActionQueue::getLastUpdateTime,
			DOC(MGE, ActionQueue, getLastUpdateTime)
		)
		.def("__len__",                   &MGE::ActionQueue::getLength)
		.def("__iter__",                [](MGE::ActionQueue &q) { return py::make_iterator<py::return_value_policy::reference>(q.begin(), q.end()); },
			py::keep_alive<0, 1>(), // keep ActionQueue alive while using iterator
			"return iterator"
		)
		.def_static("getFromActor",       &getActionQueueFromActor,
			py::return_value_policy::reference,
			"get ActionQueue from BaseActor"
		)
	;
}
#endif
