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

#include "physics/utils/OgreColisionBoundingBox.h"

#include "ScriptsInterface.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
#include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(ColisionBoundingBox) {
	m.def("intersects",
		py::overload_cast<const Ogre::AxisAlignedBox&, const Ogre::SceneNode*, const Ogre::Vector3&, const Ogre::Quaternion& , const Ogre::Vector3&, const Ogre::AxisAlignedBox&, const Ogre::SceneNode* >
			(&MGE::OgreColisionBoundingBox::intersects),
		DOC(MGE, OgreColisionBoundingBox, intersects)
	);
	m.def("intersects",
		py::overload_cast< const Ogre::AxisAlignedBox&, const Ogre::SceneNode*, const Ogre::AxisAlignedBox&, const Ogre::SceneNode* >
			(&MGE::OgreColisionBoundingBox::intersects),
		DOC(MGE, OgreColisionBoundingBox, intersects, 2)
	);
	m.def("intersects",
		py::overload_cast< const Ogre::Ray&, const Ogre::AxisAlignedBox&, Ogre::Real >
			(&MGE::OgreColisionBoundingBox::intersects),
		DOC(MGE, OgreColisionBoundingBox, intersects, 3),
			py::arg("ray"), py::arg("box"), py::arg("rayLen") = Ogre::Math::POS_INFINITY
	);
	
	/////  isFreeSphere  /////
	
	m.def("isFreeSphere",
		[](Ogre::SceneManager* scnMgr, const Ogre::Vector3& position, Ogre::Real radius, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreeSphere(scnMgr, position, radius, queryMask, nullptr);
		},
	   DOC(MGE, OgreColisionBoundingBox, isFreeSphere)
	);
	m.def("getCollidersOnSphere",
		[](Ogre::SceneManager* scnMgr, const Ogre::Vector3& position, Ogre::Real radius, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreeSphere(scnMgr, position, radius, queryMask, &ret);
			return ret;
		},
		"isFreeSphere variant returning all objects with which we collided"
	);
	
	/////  isFreePosition  /////
	
	m.def("isFreePosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& newPosition, const Ogre::Quaternion& newOrientation, const Ogre::Vector3& newScale, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, newPosition, newOrientation, newScale, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePosition)
	);
	m.def("getCollidersOnPosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& newPosition, const Ogre::Quaternion& newOrientation, const Ogre::Vector3& newScale, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, newPosition, newOrientation, newScale, queryMask, &ret);
			return ret;
		},
		"isFreePosition variant returning all objects with which we collided. "
	);
	
	m.def("isFreePosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& newPosition, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, newPosition, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePosition, 2)
	);
	m.def("getCollidersOnPosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& newPosition, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, newPosition, queryMask, &ret);
			return ret;
		},
		"isFreePosition variant returning all objects with which we collided. "
	);
	
	m.def("isFreePosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePosition, 3)
	);
	m.def("getCollidersOnPosition",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePosition(node, aabb, queryMask, &ret);
			return ret;
		},
		"isFreePosition variant returning all objects with which we collided. "
	);
	
	m.def("isFreePosition",
		[](const Ogre::SceneNode* node, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePosition(node, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePosition, 4)
	);
	m.def("getCollidersOnPosition",
		[](const Ogre::SceneNode* node, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePosition(node, queryMask, &ret);
			return ret;
		},
		"isFreePosition variant returning all objects with which we collided. "
	);
	
	/////  isFreePath  /////
	
	m.def("isFreePath",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& start, const Ogre::Vector3& end, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePath(node, aabb, start, end, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePath)
	);
	m.def("getCollidersOnPath",
		[](const Ogre::SceneNode* node, const Ogre::AxisAlignedBox& aabb, const Ogre::Vector3& start, const Ogre::Vector3& end, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePath(node, aabb, start, end, queryMask, &ret);
			return ret;
		},
		"isFreePath variant returning all objects with which we collided. "
	);
	
	m.def("isFreePath",
		[](const Ogre::SceneNode* node, const Ogre::Vector3& start, const Ogre::Vector3& end, int queryMask) {
			return MGE::OgreColisionBoundingBox::isFreePath(node, start, end, queryMask, nullptr);
		},
		DOC(MGE, OgreColisionBoundingBox, isFreePath, 2)
	);
	m.def("getCollidersOnPath",
		[](const Ogre::SceneNode* node, const Ogre::Vector3& start, const Ogre::Vector3& end, int queryMask) {
			std::list<Ogre::MovableObject*> ret;
			MGE::OgreColisionBoundingBox::isFreePath(node, start, end, queryMask, &ret);
			return ret;
		},
		"isFreePath variant returning all objects with which we collided. "
	);
}
#endif
