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

#include "rendering/CameraNode.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(CameraNode) {
	py::class_<MGE::CameraNode, std::unique_ptr<MGE::CameraNode, py::nodelete>>(
		m, "Camera",  DOC(MGE, CameraNode)
	)
		.def("getName",         &MGE::CameraNode::getName,
			 DOC(MGE, CameraNode, getName)
		)
		.def("setMode",         &MGE::CameraNode::setMode,
			DOC(MGE, CameraNode, setMode)
		)
		.def("setOwner",        py::overload_cast< const std::string_view&, bool, bool >(&MGE::CameraNode::setOwner),
			DOC(MGE, CameraNode, setOwner)
		)
		.def("setOwner",        py::overload_cast< const Ogre::SceneNode*, bool, bool >(&MGE::CameraNode::setOwner),
			DOC(MGE, CameraNode, setOwner)
		)
		
		.def("getPosition",    &MGE::CameraNode::getPosition,
			DOC(MGE, CameraNode, getPosition)
		)
		.def("getOrientation", &MGE::CameraNode::getOrientation,
			DOC(MGE, CameraNode, getOrientation)
		)
		.def("setPosition",     &MGE::CameraNode::setPosition,
			DOC(MGE, CameraNode, setPosition)
		)
		.def("move",            &MGE::CameraNode::move,
			DOC(MGE, CameraNode, move)
		)
		.def("setOrientation",  &MGE::CameraNode::setOrientation,
			DOC(MGE, CameraNode, setOrientation)
		)
		.def("rotate",          py::overload_cast< const Ogre::Vector3&, const Ogre::Radian& >(&MGE::CameraNode::rotate),
			DOC(MGE, CameraNode, rotate)
		)
		.def("rotate",          py::overload_cast< const Ogre::Quaternion& >(&MGE::CameraNode::rotate),
			DOC(MGE, CameraNode, rotate)
		)
		.def("setFixedYawAxis", &MGE::CameraNode::setFixedYawAxis,
			DOC(MGE, CameraNode, setFixedYawAxis)
		)
		.def("setDirection",    &MGE::CameraNode::setDirection,
			DOC(MGE, CameraNode, setDirection)
		)
		.def("setDirection",    [](MGE::CameraNode &c, const Ogre::Vector3& vec) { c.setDirection (vec, Ogre::Node::TS_LOCAL, Ogre::Vector3::NEGATIVE_UNIT_Z); },
			DOC(MGE, CameraNode, setDirection)
		)
		.def("lookAt",          &MGE::CameraNode::lookAt,
			DOC(MGE, CameraNode, lookAt)
		)
		.def("lookAt",          [](MGE::CameraNode &c, const Ogre::Vector3& targetPoint) { c.lookAt(targetPoint, Ogre::Node::TS_LOCAL, Ogre::Vector3::NEGATIVE_UNIT_Z); },
			DOC(MGE, CameraNode, lookAt)
		)
		
		.def("setDistance",     &MGE::CameraNode::setDistance,
			DOC(MGE, CameraNode, setDistance)
		)
		.def("setYaw",          &MGE::CameraNode::setYaw,
			DOC(MGE, CameraNode, setYaw)
		)
		.def("setPitch",        &MGE::CameraNode::setPitch,
			DOC(MGE, CameraNode, setPitch)
		)
		.def("setFOV",          &MGE::CameraNode::setFOV,
			DOC(MGE, CameraNode, setFOV)
		)
		.def("incDistance",     &MGE::CameraNode::incDistance,
			DOC(MGE, CameraNode, incDistance)
		)
		.def("incYaw",          &MGE::CameraNode::incYaw,
			DOC(MGE, CameraNode, incYaw)
		)
		.def("incPitch",        &MGE::CameraNode::incPitch,
			DOC(MGE, CameraNode, incPitch)
		)
		.def("incFOV",          &MGE::CameraNode::incFOV,
			DOC(MGE, CameraNode, incFOV)
		)
		.def("getZoom",         &MGE::CameraNode::getZoom,
			DOC(MGE, CameraNode, getZoom)
		)
		.def("getYaw",          &MGE::CameraNode::getYaw,
			DOC(MGE, CameraNode, getYaw)
		)
		.def("getPitch",        &MGE::CameraNode::getPitch,
			DOC(MGE, CameraNode, getPitch)
		)
		.def("getFOV",          &MGE::CameraNode::getFOV,
			py::return_value_policy::reference,
			DOC(MGE, CameraNode, getFOV)
		)
		
		.def("writeScreenshot", static_cast< void(MGE::CameraNode::*)(const Ogre::String&) const >(&MGE::CameraNode::writeScreenshot),
			DOC(MGE, CameraNode, writeScreenshot)
		)
		.def("writeScreenshot", static_cast< std::string(MGE::CameraNode::*)(const std::string_view&, const std::string_view&) const >(&MGE::CameraNode::writeScreenshot),
			DOC(MGE, CameraNode, writeScreenshot)
		)
		
		.def_readwrite("limitPositionMax",  &MGE::CameraNode::limitPositionMax,
			DOC(MGE, CameraNode, limitPositionMax)
		)
		.def_readwrite("limitPositionMin",  &MGE::CameraNode::limitPositionMin,
			DOC(MGE, CameraNode, limitPositionMin)
		)
		.def_readwrite("limitZoomMax",      &MGE::CameraNode::limitZoomMax,
			DOC(MGE, CameraNode, limitZoomMax)
		)
		.def_readwrite("limitZoomMin",      &MGE::CameraNode::limitZoomMin,
			DOC(MGE, CameraNode, limitZoomMin)
		)
		.def_readwrite("limitPitchMax",     &MGE::CameraNode::limitPitchMax,
			DOC(MGE, CameraNode, limitPitchMax)
		)
		.def_readwrite("limitPitchMin",     &MGE::CameraNode::limitPitchMin,
			DOC(MGE, CameraNode, limitPitchMin)
		)
		.def_readwrite("limitFOVMax",       &MGE::CameraNode::limitFOVMax,
			DOC(MGE, CameraNode, limitFOVMax)
		)
		.def_readwrite("limitFOVMin",       &MGE::CameraNode::limitFOVMin,
			DOC(MGE, CameraNode, limitFOVMin)
		)
		
		.def_readwrite("kbdMoveStep",       &MGE::CameraNode::kbdMoveStep,
			DOC(MGE, CameraNode, kbdMoveStep)
		)
		.def_readwrite("kbdZoomStep",       &MGE::CameraNode::kbdZoomStep,
			DOC(MGE, CameraNode, kbdZoomStep)
		)
		.def_readwrite("kbdFOVStep",        &MGE::CameraNode::kbdFOVStep,
			DOC(MGE, CameraNode, kbdFOVStep)
		)
		.def_readwrite("kbdRotateStep",     &MGE::CameraNode::kbdRotateStep,
			DOC(MGE, CameraNode, kbdRotateStep)
		)
		.def_readwrite("shiftMultiplier",   &MGE::CameraNode::shiftMultiplier,
			DOC(MGE, CameraNode, shiftMultiplier)
		)
		.def_readwrite("zoomMultiplier",    &MGE::CameraNode::zoomMultiplier,
			DOC(MGE, CameraNode, zoomMultiplier)
		)
		.def_readwrite("mouseMoveStep",     &MGE::CameraNode::mouseMoveStep,
			DOC(MGE, CameraNode, mouseMoveStep)
		)
		.def_readwrite("mouseZoomStep",     &MGE::CameraNode::mouseZoomStep,
			DOC(MGE, CameraNode, mouseZoomStep)
		)
		.def_readwrite("mouseFOVStep",      &MGE::CameraNode::mouseFOVStep,
			DOC(MGE, CameraNode, mouseFOVStep)
		)
		.def_readwrite("mouseRotateStep",   &MGE::CameraNode::mouseRotateStep,
			DOC(MGE, CameraNode, mouseRotateStep)
		)
		.def_readwrite("mouseMaginSize",    &MGE::CameraNode::mouseMaginSize,
			DOC(MGE, CameraNode, mouseMaginSize)
		)
		.def_readwrite("mouseMaginStep",    &MGE::CameraNode::mouseMaginStep,
			DOC(MGE, CameraNode, mouseMaginStep)
		)
	;
}
#endif
