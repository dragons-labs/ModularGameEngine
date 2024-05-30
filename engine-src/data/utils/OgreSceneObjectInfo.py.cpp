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

#include "data/utils/OgreSceneObjectInfo.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(OgreSceneObjectInfo) {
	py::class_<MGE::SceneObjectInfo, std::unique_ptr<MGE::SceneObjectInfo, py::nodelete>>(
		m, "SceneObjectInfo", DOC(MGE, SceneObjectInfo)
	)
		.def_readwrite("node", &MGE::SceneObjectInfo::node, DOC(MGE, SceneObjectInfo, node))
		.def_readwrite("movable", &MGE::SceneObjectInfo::movable, DOC(MGE, SceneObjectInfo, movable))
	;
	py::class_<MGE::LoadingContext, std::unique_ptr<MGE::LoadingContext, py::nodelete>>(
		m, "LoadingContext", DOC(MGE, LoadingContext)
	)
		.def_readwrite("scnMgr", &MGE::LoadingContext::scnMgr, DOC(MGE, LoadingContext, scnMgr))
		.def_readwrite("preLoad", &MGE::LoadingContext::preLoad, DOC(MGE, LoadingContext, preLoad))
		.def_readwrite("linkToXML", &MGE::LoadingContext::linkToXML, DOC(MGE, LoadingContext, linkToXML))
		.def_readwrite("defaultResourceGroup", &MGE::LoadingContext::defaultResourceGroup, DOC(MGE, LoadingContext, defaultResourceGroup))
	;
}
#endif
