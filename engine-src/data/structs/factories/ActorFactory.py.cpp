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

#include "data/structs/factories/ActorFactory.h"
#include "data/structs/ActorMessages.h"
#include "data/structs/BasePrototype.h"
#include "data/structs/utils/ActorFromMessage.py.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
#include "data/property/pybind11_stl.py.h"
PYBIND11_MAKE_OPAQUE(std::unordered_map<std::string, MGE::BaseActor*, MGE::string_hash, std::equal_to<>>);

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(ActorFactory) {
	py::class_<MGE::ActorFactory, std::unique_ptr<MGE::ActorFactory, py::nodelete>>(
		m, "ActorFactory", DOC(MGE, ActorFactory)
	)
		.def("getActor",      &MGE::ActorFactory::getActor,
			DOC(MGE, ActorFactory, getActor),
			py::return_value_policy::reference
		)
		.def("findActors",    py::overload_cast<const Ogre::Vector3&, float>(&MGE::ActorFactory::findActors),
			DOC(MGE, ActorFactory, findActors, 2)
		)
		.def("createActor",   &MGE::ActorFactory::createActor,
			py::return_value_policy::reference,
			DOC(MGE, ActorFactory, createActor)
		)
		.def("destroyActor",  &MGE::ActorFactory::destroyActor,
			DOC(MGE, ActorFactory, destroyActor)
		)
		.def_readonly("allActors", &MGE::ActorFactory::allActors,
			DOC(MGE, ActorFactory, allActors)
		)
		.def_static("get", &MGE::ActorFactory::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("ActorFactory"))
	;
	m.def("getActorFromEventMsg", &getActorFromEventMsg<MGE::ActorDestroyEventMsg*>,
		py::return_value_policy::reference,
		"get actor from MGE::ActorDestroyEventMsg event message"
	);
	m.def("getActorFromEventMsg", &getActorFromEventMsg<MGE::ActorCreatedEventMsg*>,
		py::return_value_policy::reference,
		"get actor from MGE::ActorCreatedEventMsg event message"
	);
	MGE::py_bind_const_map<std::unordered_map<std::string, MGE::BaseActor*, MGE::string_hash, std::equal_to<>>>(m, "ActorMap");
}
#endif
