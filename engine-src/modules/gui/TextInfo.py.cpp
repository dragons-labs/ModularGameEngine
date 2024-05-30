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

#include "modules/gui/TextInfo.h"

#include "ScriptsInterface.h"

// no Ogre in this API #include "data/property/pybind11_ogre_swig_cast.py.h"
#include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(TextInfo) {
	py::enum_<MGE::TextReport::ReportType>(
		m, "ReportType", DOC(MGE, TextReport, ReportType)
	)
		.value("TXT",  MGE::TextReport::TXT)
		.value("HTML", MGE::TextReport::HTML)
		.value("URL",  MGE::TextReport::URL)
	;
	
	py::class_<MGE::TextReport>(
		m, "TextReport", DOC(MGE, TextReport)
	)
		.def("addMessage", py::overload_cast<const std::string_view&, bool>(&MGE::TextReport::addMessage),
			DOC(MGE, TextReport, addMessage)
		)
		.def("addMessage", py::overload_cast<const std::string_view&>(&MGE::TextReport::addMessage),
			DOC(MGE, TextReport, addMessage, 2)
		)
		.def_readwrite("header",             &MGE::TextReport::header,
			DOC(MGE, TextReport, header)
		)
		.def_readwrite("footer",             &MGE::TextReport::footer,
			DOC(MGE, TextReport, footer)
		)
		.def_readwrite("type",               &MGE::TextReport::type,
			DOC(MGE, TextReport, type)
		)
		.def_readwrite("msgPerPage",         &MGE::TextReport::msgPerPage,
			DOC(MGE, TextReport, msgPerPage)
		)
		.def_readwrite("autoSplit",          &MGE::TextReport::autoSplit,
			DOC(MGE, TextReport, autoSplit)
		)
		.def_readwrite("wordWrap",           &MGE::TextReport::wordWrap,
			DOC(MGE, TextReport, wordWrap)
		)
		.def_readwrite("noDuplicatedOnPrev", &MGE::TextReport::noDuplicatedOnPrev,
			DOC(MGE, TextReport, noDuplicatedOnPrev)
		)
		.def_readwrite("displayFromBack",    &MGE::TextReport::displayFromBack,
			DOC(MGE, TextReport, displayFromBack)
		)
		.def_readwrite("addToFront",         &MGE::TextReport::addToFront,
			DOC(MGE, TextReport, addToFront)
		)
		.def_readwrite("defaultAutoNewLine", &MGE::TextReport::defaultAutoNewLine,
			DOC(MGE, TextReport, defaultAutoNewLine)
		)
		.def_readwrite("entries", &MGE::TextReport::entries,
			DOC(MGE, TextReport, defaultAutoNewLine)
		)
	;
	
	py::class_<MGE::TextInfo, std::unique_ptr<MGE::TextInfo, py::nodelete>>(
		m, "TextInfo", DOC(MGE, TextInfo)
	)
		.def("show",             &MGE::TextInfo::show,
			DOC(MGE, TextInfo, show)
		)
		.def("getReport",        &MGE::TextInfo::getReport,
			py::return_value_policy::reference,
			DOC(MGE, TextInfo, getReport)
		)
		.def("setCurrentReport", py::overload_cast<const std::string_view&>(&MGE::TextInfo::setCurrentReport),
			DOC(MGE, TextInfo, setCurrentReport)
		)
		.def("setCurrentReport", py::overload_cast<MGE::TextReport*>(&MGE::TextInfo::setCurrentReport),
			DOC(MGE, TextInfo, setCurrentReport, 2)
		)
		.def("onReportUpdate",   &MGE::TextInfo::onReportUpdate,
			DOC(MGE, TextInfo, onReportUpdate)
		)
		.def_static("get", &MGE::TextInfo::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("TextInfo"))
	;
}
#endif
