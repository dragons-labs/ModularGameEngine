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

#include "data/structs/components/3DWorld.h"
#include "data/structs/BaseActor.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::World3DObject* getWorld3DObjectFromActor(MGE::BaseActor* gameObj) {
		return gameObj->getComponent<MGE::World3DObject>();
	}
} }

MGE_SCRIPT_API_FOR_MODULE(World3DObject, 17) {
	py::class_<MGE::World3DObject, MGE::BaseComponent, std::unique_ptr<MGE::World3DObject, py::nodelete>>(
		m, "World3DObject", DOC(MGE, World3DObject)
	)
		.def("getWorldPosition",            &MGE::World3DObject::getWorldPosition,
			DOC(MGE, World3DObject, getWorldPosition)
		)
		.def("getWorldOrientation",         &MGE::World3DObject::getWorldOrientation,
			DOC(MGE, World3DObject, getWorldOrientation)
		)
		.def("getWorldDirection",           &MGE::World3DObject::getWorldDirection,
			DOC(MGE, World3DObject, getWorldDirection)
		)
		
		.def("setWorldPosition",            &MGE::World3DObject::setWorldPosition,
			DOC(MGE, World3DObject, setWorldPosition)
		)
		.def("setWorldPositionOnGround",    &MGE::World3DObject::setWorldPositionOnGround,
			DOC(MGE, World3DObject, setWorldPositionOnGround)
		)
		.def("setWorldOrientation",         &MGE::World3DObject::setWorldOrientation,
			DOC(MGE, World3DObject, setWorldOrientation)
		)
		.def("setWorldDirection",           &MGE::World3DObject::setWorldDirection,
			DOC(MGE, World3DObject, setWorldDirection)
		)
		.def("getOgreSceneNode",            &MGE::World3DObject::getOgreSceneNode,
			py::return_value_policy::reference,
			DOC(MGE, World3DObject, getOgreSceneNode)
		)
		.def_static("getFromActor",         &getWorld3DObjectFromActor,
			py::return_value_policy::reference,
			"get World3DObject from BaseActor"
		)
	;
}
#endif
