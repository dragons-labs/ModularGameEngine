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

#include "data/structs/components/ScriptComponent.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/BaseActor.h"

#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	MGE::ScriptComponent* getScriptComponentFromActor(MGE::BaseActor* gameObj, int typeID) {
		return gameObj->getComponent<MGE::ScriptComponent>(typeID);
	}
	
	py::object getScriptComponentPythonObjectFromActor(MGE::BaseActor* gameObj, int typeID) {
		auto tmp = gameObj->getComponent<MGE::ScriptComponent>(typeID);
		if (tmp)
			return tmp->getPythonObject();
		else
			return py::object();
	}
	
	MGE::ScriptComponent* getScriptComponentFromActorByName(MGE::BaseActor* gameObj, const char* typeID) {
		return getScriptComponentFromActor(gameObj, MGE::ComponentFactory::getPtr()->getID(typeID));
	}
	
	py::object getScriptComponentPythonObjectFromActorByName(MGE::BaseActor* gameObj, const char* typeID) {
		return getScriptComponentPythonObjectFromActor(gameObj, MGE::ComponentFactory::getPtr()->getID(typeID));
	}
} }

MGE_SCRIPT_API_FOR_MODULE(ScriptComponent, 17) {
	py::class_<MGE::ScriptComponent, MGE::BaseComponent, std::unique_ptr<MGE::ScriptComponent, py::nodelete>>(
		m, "ScriptComponent", DOC(MGE, ScriptComponent)
	)
		.def_static("setup",             &MGE::ScriptComponent::setup,
			DOC(MGE, ScriptComponent, getPythonObject)
		)
		
		.def("getPythonObject",          &MGE::ScriptComponent::getPythonObject,
			DOC(MGE, ScriptComponent, getPythonObject)
		)
		
		.def_static("getPythonObjectFromActor", &getScriptComponentPythonObjectFromActor,
			"get Python object used by ScriptComponent from BaseActor and script component typeID"
		)
		.def_static("getPythonObjectFromActor", &getScriptComponentPythonObjectFromActorByName,
			"get Python object used by ScriptComponent from BaseActor and script component typeID"
		)
		
		.def_static("getFromActor",      &getScriptComponentFromActor,
			py::return_value_policy::reference,
			"get ScriptComponent from BaseActor and script component typeID"
		)
		.def_static("getFromActor",      &getScriptComponentFromActorByName,
			py::return_value_policy::reference,
			"get ScriptComponent from BaseActor and script component typeID"
		)
	;
}
#endif
