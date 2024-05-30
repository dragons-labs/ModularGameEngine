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

#include "gui/modules/GuiConsole.h"

#include "ScriptsInterface.h"

#include "gui/utils/CeguiString.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	void addTextToConsole(MGE::GUIConsole &c, const std::string& n) {
		c.addTextToConsole( STRING_TO_CEGUI(n) );
	}
} }

MGE_SCRIPT_API_FOR_MODULE(GUIConsole) {
	py::class_<MGE::GUIConsole, std::unique_ptr<MGE::GUIConsole, py::nodelete>>(
		m, "GUIConsole", DOC(MGE, GUIConsole)
	)
		.def("addText",   &addTextToConsole,
			DOC(MGE, GUIConsole, addTextToConsole)
		)
		.def("isVisible", &MGE::GUIConsole::isVisible,
			DOC(MGE, GUIConsole, isVisible)
		)
		.def("show", &MGE::GUIConsole::show,
			DOC(MGE, GUIConsole, show)
		)
		.def("hide", &MGE::GUIConsole::hide,
			DOC(MGE, GUIConsole, hide)
		)
		.def("toggleVisibility", &MGE::GUIConsole::toggleVisibility,
			DOC(MGE, GUIConsole, toggleVisibility)
		)
		.def("addScript", &MGE::GUIConsole::addConsoleScript,
			DOC(MGE, GUIConsole, addConsoleScript)
		)
		.def_static("get", &MGE::GUIConsole::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("GUIConsole"))
	;
}
#endif
