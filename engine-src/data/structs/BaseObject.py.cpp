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

#include "data/structs/BaseObject.py.h"
#include "data/structs/BaseComponent.h"

#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::BaseComponent* getComponent(MGE::NamedObject& obj, int typeID) {
		return obj.getComponent(typeID);
	}
	
	uintptr_t getUniqueID(const MGE::NamedObject& a) {
		return reinterpret_cast<uintptr_t>(&a);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(NamedObject, 13) {
	py::class_<MGE::BaseObject, std::unique_ptr<MGE::BaseObject, py::nodelete>>(
		m, "BaseObject", DOC(MGE, BaseObject)
	)
	;
	py::class_<MGE::NamedObject, MGE::BaseObject, MGE::PropertySetInterface, std::unique_ptr<MGE::NamedObject, py::nodelete>>(
		m, "NamedObject", DOC(MGE, NamedObject)
	)
		.def("getID",            &getUniqueID,
			"return unique ID of C++ object"
		)
		.def("getName",          &MGE::NamedObject::getName,
			DOC(MGE, NamedObject, getName)
		)
		.def("getComponent",     &getComponent,
			py::return_value_policy::reference,
			DOC(MGE, NamedObject, getComponent)
		)
	;
	MGE::py_bind_set<std::set<MGE::NamedObject*>>(m, "NamedObjectList");
}
#endif
