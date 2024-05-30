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

#include "game/actorComponents/World3DMovable.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"

#include "game/actions/Action.h"
#include "data/structs/BaseActor.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::World3DMovable* getWorld3DMovableFromActor(MGE::BaseActor* gameObj) {
		return gameObj->getComponent<MGE::World3DMovable>();
	}
	
	void initMove(MGE::World3DMovable& w3dm, const MGE::Action* action) {
		w3dm.initMove(action->targetPoints);
	}
	
	MGE::ActorMovingEventMsg* getActorMovingEventMsgFromEventMsg(MGE::EventMsg* msg) {
		return static_cast<MGE::ActorMovingEventMsg*>(msg);
	}
	
	MGE::BaseActor* getActor(const MGE::ActorMovingEventMsg& msg) {
		return msg.actor;
	}
} }

MGE_SCRIPT_API_FOR_MODULE(World3DMovable, 19) {
	py::class_< MGE::ActorMovingEventMsg, MGE::EventMsg, std::unique_ptr<MGE::ActorMovingEventMsg, py::nodelete> >(
		m, "ActorMovingEventMsg", DOC(MGE, ActorMovingEventMsg)
	)
		.def("getActor", &getActor,
			py::return_value_policy::reference,
			DOC(MGE, ActorMovingEventMsg, actor)
		)
		.def_readonly("isMove", &MGE::ActorMovingEventMsg::isMove,
			DOC(MGE, ActorMovingEventMsg, isMove)
		)
		.def_static("getFromEventMsg", &getActorMovingEventMsgFromEventMsg,
			py::return_value_policy::reference,
			"get ActorMovingEventMsg from EventMsg"
		)
	;
	py::class_< MGE::World3DMovable, MGE::World3DObject, std::unique_ptr<MGE::World3DMovable, py::nodelete> >(
		m, "World3DMovable", DOC(MGE, World3DMovable)
	)
		.def("initMove", &initMove,
			DOC(MGE, World3DMovable, initMove)
		)
		.def_static("getFromActor", &getWorld3DMovableFromActor,
			py::return_value_policy::reference,
			"get World3DMovable from BaseActor"
		)
	;
}
#endif
