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

#include "game/actorComponents/Light.h"

#include "ScriptsInterface.h"

#include "data/structs/BaseActor.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::Light* getLightFromActor(MGE::BaseActor* gameObj) {
		return gameObj->getComponent<MGE::Light>();
	}
} }

MGE_SCRIPT_API_FOR_MODULE(Light, 17) {
	py::class_<MGE::Light, MGE::BaseComponent, std::unique_ptr<MGE::Light, py::nodelete>>(
		m, "Light", DOC(MGE, Light)
	)
		.def("isGroupOn",           &MGE::Light::isGroupOn,
			DOC(MGE, Light, isGroupOn)
		)
		.def("setGroupOn",          &MGE::Light::setGroupOn,
			DOC(MGE, Light, setGroupOn)
		)
		.def("setGroupOff",         &MGE::Light::setGroupOff,
			DOC(MGE, Light, setGroupOff)
		)
		.def("setAllOn",            &MGE::Light::setAllOn,
			DOC(MGE, Light, setAllOn)
		)
		.def("setAllOff",           &MGE::Light::setAllOff,
			DOC(MGE, Light, setAllOff)
		)
		
		.def_static("getFromActor", &getLightFromActor,
			py::return_value_policy::reference,
			"get Light component from BaseActor"
		)
	;
}
#endif
