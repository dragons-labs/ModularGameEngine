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

#pragma   once

#include "input/SelectionSet.h"

#include "data/property/pybind11_stl.py.h"

namespace MGE { namespace ScriptsInterface {

/**
 * template for exposing SelectionSet template to python script interface
 */
template<class TYPE, int MASK, class SCLASS, class ATYPE> inline void ExposingSelectionSet(py::module_& m, const MGE::null_end_string name, const MGE::null_end_string desc) {
	py::class_<  MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE> >(
		m, name, desc
	)
		.def_readwrite("selection", &MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::selection, DOC(MGE, SelectionSetTemplate, selection))
		
		.def("select",          static_cast< void(MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::*)(TYPE, int, bool) >(
				&MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::select
			), DOC(MGE, SelectionSetTemplate, select)
		)
		.def("unselect",        static_cast< int(MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::*)(TYPE, int) >(
				&MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::unselect
			), DOC(MGE, SelectionSetTemplate, unselect)
		)
		.def("unselectAll",     &MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::unselectAll,
			DOC(MGE, SelectionSetTemplate, unselectAll)
		)
		.def("isSelected",      &MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::isSelected,
			DOC(MGE, SelectionSetTemplate, isSelected)
		)
		.def("switchSelection", &MGE::SelectionSet<TYPE, MASK, SCLASS, ATYPE>::switchSelection,
			DOC(MGE, SelectionSetTemplate, switchSelection)
		)
	;
}

} }
