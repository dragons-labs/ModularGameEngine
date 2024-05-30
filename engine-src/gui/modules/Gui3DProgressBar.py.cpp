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

#include "gui/modules/Gui3DProgressBar.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(ProgressBar3D, 13) {
	py::class_<MGE::ProgressBar3D, MGE::GUI3D>(
		m, "ProgressBar3D", DOC(MGE, ProgressBar3D)
	)
		.def(py::init<Ogre::SceneNode*, const Ogre::String&, const Ogre::Vector3&, bool>(),
			DOC(MGE, ProgressBar3D, ProgressBar3D)
		)
		.def(py::init<Ogre::SceneNode*, const Ogre::String&, const Ogre::Vector3&>(),
			DOC(MGE, ProgressBar3D, ProgressBar3D)
		)
		.def(py::init<Ogre::SceneNode*, const Ogre::String&>(),
			DOC(MGE, ProgressBar3D, ProgressBar3D)
		)
		.def("setProgress",  static_cast< void(MGE::ProgressBar3D::*)(float) >(
				&MGE::ProgressBar3D::setProgress
			), DOC(MGE, ProgressBar3D, setProgress)
		 )
		.def("setProgress",  static_cast< void(MGE::ProgressBar3D::*)(float, CEGUI::argb_t) >(
				&MGE::ProgressBar3D::setProgress
			), DOC(MGE, ProgressBar3D, setProgress, 2)
		)
		.def("getValue", &MGE::ProgressBar3D::getValue,
			DOC(MGE, ProgressBar3D, getValue)
		)
		.def("getColour", &MGE::ProgressBar3D::getColour,
			DOC(MGE, ProgressBar3D, getColour)
		)
	;
}
#endif
