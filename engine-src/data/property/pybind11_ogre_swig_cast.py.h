/*
Copyright (c) 2022-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#ifndef __DOCUMENTATION_GENERATOR__

#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>
#include <OgreMath.h>
#include <OgreRay.h>
#include <OgreAxisAlignedBox.h>

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreBillboardSet.h>

#include "pybind11_swig_cast.inl"

PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Vector2)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Vector3)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Ray)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Quaternion)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, ColourValue)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Radian)
PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, AxisAlignedBox)

PYBIND11_SWIG_GENERATE_CAST_ONLYPTR(Ogre, SceneNode)
PYBIND11_SWIG_GENERATE_CAST_ONLYPTR(Ogre, MovableObject)
PYBIND11_SWIG_GENERATE_CAST_ONLYPTR(Ogre, SceneManager)

PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre::v1, BillboardSet)

#include "data/property/pybind11_stl.py.h"

PYBIND11_MAKE_OPAQUE(std::list<Ogre::Vector2>);
PYBIND11_MAKE_OPAQUE(std::list<Ogre::Vector3>);
PYBIND11_MAKE_OPAQUE(std::list<Ogre::MovableObject*>);

#endif
