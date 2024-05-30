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

#include "physics/Raycast.h"
#include "data/structs/BaseActor.h"
#include "physics/utils/OgreColisionBoundingBox.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
#include "data/property/pybind11_stl.py.h"

PYBIND11_MAKE_OPAQUE(std::list<MGE::RayCast::ResultsEntry>);

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(Raycast) {
	py::class_<MGE::RayCast::ResultsEntry>( m, "ResultsEntry", DOC(MGE, RayCast, ResultsEntry) )
		.def_readonly("gameObject",  &MGE::RayCast::ResultsEntry::gameObject,
			py::return_value_policy::reference,
			DOC(MGE, RayCast, ResultsEntry, gameObject)
		)
		.def_readonly("ogreObject",  &MGE::RayCast::ResultsEntry::ogreObject,
			py::return_value_policy::reference,
			DOC(MGE, RayCast, ResultsEntry, ogreObject)
		)
		.def_readonly("hitPoint",    &MGE::RayCast::ResultsEntry::hitPoint,
			DOC(MGE, RayCast, ResultsEntry, hitPoint)
		)
	;
	py::class_<MGE::RayCast::Results>( m, "RayCastResults", DOC(MGE, RayCast, Results) )
		.def_readonly("hasGround",   &MGE::RayCast::Results::hasGround,
			DOC(MGE, RayCast, Results, hasGround)
		)
		.def_readonly("groundPoint", &MGE::RayCast::Results::groundPoint,
			DOC(MGE, RayCast, Results, groundPoint)
		)
		.def_readonly("hitObjects",  &MGE::RayCast::Results::hitObjects,
			DOC(MGE, RayCast, Results, hitObjects)
		)
	;
	
	MGE::py_bind_list<std::list<MGE::RayCast::ResultsEntry>>(m, "RayCastResultsList");
	
	m.def("searchFromCamera",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::Real, Ogre::Real, uint32_t, bool)> (&MGE::RayCast::searchFromCamera),
		DOC(MGE, RayCast, searchFromCamera),
		py::arg("screenx"), py::arg("screeny"), py::arg("searchMask") = 0xFFFFFFFF, py::arg("onlyFirst") = false
	);
	m.def("searchFromRay",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, const Ogre::Ray&, uint32_t, bool, Ogre::Real)> (&MGE::RayCast::searchFromRay),
		DOC(MGE, RayCast, searchFromRay),
		py::arg("scnMgr"), py::arg("ray"), py::arg("searchMask=0xFFFFFFFF"), py::arg("onlyFirst") = false, py::arg("searchDistance") = MGE::WorldSizeInfo::getRayLenght()
	);
	m.def("searchFromPoints",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, const Ogre::Vector3&, const Ogre::Vector3&, uint32_t, bool)> (&MGE::RayCast::searchFromPoints),
		DOC(MGE, RayCast, searchFromPoints),
		py::arg("scnMgr"), py::arg("rayFrom"), py::arg("rayTo"), py::arg("searchMask") = 0xFFFFFFFF, py::arg("onlyFirst") = false
	);
	m.def("searchVertical",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, Ogre::Real, Ogre::Real, uint32_t, bool, Ogre::Real, Ogre::Real)> (&MGE::RayCast::searchVertical),
		DOC(MGE, RayCast, searchVertical),
		py::arg("scnMgr"), py::arg("x"), py::arg("z"), py::arg("searchMask") = 0xFFFFFFFF, py::arg("onlyFirst") = false, py::arg("maxY") = MGE::WorldSizeInfo::getWorldMax().y, py::arg("minY") = MGE::WorldSizeInfo::getWorldMin().y
	);
	m.def("searchOnRay",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, const Ogre::Ray&, const Ogre::Vector3&, uint32_t, bool, bool)> (&MGE::RayCast::searchOnRay),
		DOC(MGE, RayCast, searchOnRay),
		py::arg("scnMgr"), py::arg("ray"), py::arg("rayTo"), py::arg("searchMask") = 0xFFFFFFFF, py::arg("onlyFirst") = false, py::arg("vertical") = false
	);
	m.def("searchOnArea",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, const std::vector<Ogre::Ray>&, uint32_t)> (&MGE::RayCast::searchOnArea),
		DOC(MGE, RayCast, searchOnArea),
		py::arg("scnMgr"), py::arg("rays"), py::arg("searchMask") = 0xFFFFFFFF
	);
	m.def("searchOnRadius",
		static_cast<MGE::RayCast::ResultsPtr(*)(Ogre::SceneManager*, Ogre::Real, const Ogre::Vector3&, uint32_t)> (&MGE::RayCast::searchOnRadius),
		DOC(MGE, RayCast, searchOnRadius),
		py::arg("scnMgr"), py::arg("radius"), py::arg("point"), py::arg("searchMask") = 0xFFFFFFFF
	);
	m.def("findFreePosition",
		static_cast<std::pair<bool, Ogre::Vector3> (*)(const Ogre::SceneNode*, const Ogre::AxisAlignedBox&, uint32_t)> (&MGE::RayCast::findFreePosition),
		DOC(MGE, RayCast, findFreePosition),
		py::arg("node"), py::arg("aabb"), py::arg("searchMask") = static_cast<int>(MGE::QueryFlags::COLLISION_OBJECT)
	);
}
#endif
