/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "ScriptsInterface.h"

#include "rendering/markers/VisualMarkers.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(VisualMarkersManager) {
	py::class_<MGE::VisualMarkersManager, std::unique_ptr<MGE::VisualMarkersManager, py::nodelete>> (
		m, "VisualMarkersManager", DOC(MGE, VisualMarkersManager)
	)
		.def("showMarker", 
			py::overload_cast<Ogre::SceneNode*, const Ogre::AxisAlignedBox*, int, const Ogre::String&, float>( &MGE::VisualMarkersManager::showMarker ),
			py::return_value_policy::reference,
			DOC(MGE, VisualMarkersManager, showMarker)
		)
		.def("hideMarker",
			&MGE::VisualMarkersManager::hideMarker,
			DOC(MGE, VisualMarkersManager, hideMarker)
		)
		.def_static("get", &MGE::VisualMarkersManager::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("VisualMarkersManager") )
	;
	
	
	py::class_<MGE::VisualMarker, std::unique_ptr<MGE::VisualMarker, py::nodelete>> vm (
		m, "VisualMarker", DOC(MGE, VisualMarker)
	);
	
	// enums in "VisualMarker" scope
	
	py::enum_<MGE::VisualMarker::PrimaryTypes>(
		vm, "VisualMarkerPrimaryTypes", py::arithmetic(), DOC(MGE, VisualMarker, PrimaryTypes)
	)
		.value("OBBOX",                      MGE::VisualMarker::OBBOX)
		.value("PLANE",                      MGE::VisualMarker::PLANE)
		.value("DECAL",                      MGE::VisualMarker::DECAL)
		.value("OUTLINE",                    MGE::VisualMarker::OUTLINE)
		.export_values()
	;
	py::enum_<MGE::VisualMarker::LineThicknessTypes>(
		vm, "VisualMarkerLineThicknessTypes", py::arithmetic(), DOC(MGE, VisualMarker, LineThicknessTypes)
	)
		.value("NO_THICKNESS",               MGE::VisualMarker::NO_THICKNESS)
		.value("ABSOLUTE_THICKNESS",         MGE::VisualMarker::ABSOLUTE_THICKNESS)
		.value("BOX_PROPORTIONAL_THICKNESS", MGE::VisualMarker::BOX_PROPORTIONAL_THICKNESS)
		.export_values()
	;
	py::enum_<MGE::VisualMarker::OOBoxSubTypes>(
		vm, "VisualMarkerOOBoxSubTypes", py::arithmetic(), DOC(MGE, VisualMarker, OOBoxSubTypes)
	)
		.value("FULL_BOX",                   MGE::VisualMarker::FULL_BOX)
		.value("CORNER_BOX",                 MGE::VisualMarker::CORNER_BOX)
		.export_values()
	;
}
#endif
