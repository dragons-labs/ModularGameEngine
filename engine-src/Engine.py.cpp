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

#include "Engine.h"
#include "ScriptsInterface.h"

#include "MessagesSystem.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	[[ noreturn ]] void crash(const Engine*, const std::string& errorMessage) {
		Engine::crash("ScriptsInterface", errorMessage);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(Engine) {
	py::class_<MGE::Engine, std::unique_ptr<MGE::Engine, py::nodelete>>(
		m, "Engine", DOC(MGE, Engine)
	)
		.def("shutdown", &MGE::Engine::shutDown,
			DOC(MGE, Engine, shutDown)
		)
		.def("getMessagesSystem", &MGE::Engine::getMessagesSystem,
			DOC(MGE, Engine, getMessagesSystem)
		)
		.def("crash", &MGE::ScriptsInterface::crash,
			 "Crash engine (show crash message, write on-crash save and exit). For call on critical error at Python side. \n\n"
			 "Exceptions in script code only break current script code execution and log error message, so to enforce engine crash is require call this method.",
			 py::arg("errorMessage")
		)
		.def_static("get", &MGE::Engine::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("Engine"))
	;
}
#endif
