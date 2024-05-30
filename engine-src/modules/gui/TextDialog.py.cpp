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

#include "modules/gui/TextDialog.h"

#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(TextDialog) {
	py::class_<MGE::TextDialog, std::unique_ptr<MGE::TextDialog, py::nodelete>>(
		m, "TextDialog", DOC(MGE, TextDialog)
	)
		.def("runDialog", py::overload_cast<const std::string_view&, int, bool>(&MGE::TextDialog::runDialog),
			DOC(MGE, TextDialog, runDialog)
		)
		.def("runDialog", py::overload_cast<const std::string_view&, int>(&MGE::TextDialog::runDialog),
			DOC(MGE, TextDialog, runDialog, 2)
		)
		.def("showText", &MGE::TextDialog::showText,
			DOC(MGE, TextDialog, showText)
		)
		.def("setImage", &MGE::TextDialog::setImage,
			DOC(MGE, TextDialog, setImage)
		)
		.def("unsetImage", &MGE::TextDialog::unsetImage,
			DOC(MGE, TextDialog, unsetImage)
		)
		.def("addAnswer", &MGE::TextDialog::addAnswer,
			DOC(MGE, TextDialog, addAnswer)
		)
		.def("showAnswers", &MGE::TextDialog::showAnswers,
			DOC(MGE, TextDialog, showAnswers)
		)
		.def("onDialog", &MGE::TextDialog::onDialog,
			DOC(MGE, TextDialog, onDialog)
		)
		.def_static("get", &MGE::TextDialog::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("TextDialog"))
	;
}
#endif
