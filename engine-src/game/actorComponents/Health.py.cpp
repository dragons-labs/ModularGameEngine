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

#include "game/actorComponents/Health.h"

#include "ScriptsInterface.h"

// no Ogre in this API #include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"

#include "data/structs/BaseActor.h"
#include "data/structs/utils/ActorFromMessage.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::Health* getHealthFromActor(MGE::BaseActor* gameObj) {
		return gameObj->getComponent<MGE::Health>();
	}
} }

MGE_SCRIPT_API_FOR_MODULE(Health, 17) {
	py::class_< MGE::Health, MGE::BaseComponent, std::unique_ptr<MGE::Health, py::nodelete> >(
		m, "Health", DOC(MGE, Health)
	)
		.def_readwrite("health",    &MGE::Health::health,
			DOC(MGE, Health, health)
		)
		.def_readwrite("healthMax",    &MGE::Health::healthMax,
			DOC(MGE, Health, healthMax)
		)
		.def_readwrite("healthMin",    &MGE::Health::healthMin,
			DOC(MGE, Health, healthMin)
		)
		
		.def("isInjured",              &MGE::Health::isInjured,
			DOC(MGE, Health, isInjured)
		)
		.def("isDead",                 &MGE::Health::isDead,
			DOC(MGE, Health, isDead)
		)
		.def("getHealthLevel",         &MGE::Health::getHealthLevel,
			DOC(MGE, Health, getHealthLevel)
		)
		.def("getNormalHealthLevel",   &MGE::Health::getNormalHealthLevel,
			DOC(MGE, Health, getNormalHealthLevel)
		)
		.def("getInjuredHealthLevel",  &MGE::Health::getInjuredHealthLevel,
			DOC(MGE, Health, getInjuredHealthLevel)
		)
		.def("updateHealth",           &MGE::Health::updateHealth,
			DOC(MGE, Health, updateHealth)
		)
		.def_static("getFromActor",    &getHealthFromActor,
			py::return_value_policy::reference,
			"get Health from BaseActor"
		)
	;
	m.def("getActorFromEventMsg", &getActorFromEventMsg<MGE::HealthSubSystem::ActorDeathMsg*>,
		py::return_value_policy::reference,
		"get actor from ActorDeathMsg event message"
	);
}
#endif
