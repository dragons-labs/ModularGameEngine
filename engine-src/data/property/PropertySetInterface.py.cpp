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

#include "data/property/PropertySetInterface.h"
#include "data/property/Any.h"

#include "ScriptsInterface.h"

#include "data/property/XmlUtils_Ogre.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"
#include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(Property) {
	py::class_<MGE::PropertySetInterface>(m, "PropertySetInterface", DOC(MGE, PropertySetInterfaceTmpl))
		.def("hasProperty", &MGE::PropertySetInterface::hasProperty,
			 DOC(MGE, PropertySetInterfaceTmpl, hasProperty))
		.def("remProperty", &MGE::PropertySetInterface::remProperty,
			 DOC(MGE, PropertySetInterfaceTmpl, remProperty))
		
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<bool>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<int>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<float>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<std::string>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<Ogre::Vector2>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<Ogre::Vector3>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		.def("getProperty", &MGE::PropertySetInterface::getPropertyValue<std::list<std::string>>,
			 DOC(MGE, PropertySetInterfaceTmpl, getPropertyValue))
		
		.def("addProperty", &MGE::PropertySetInterface::addProperty<bool>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<int>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<float>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<std::string>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<Ogre::Vector2>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<Ogre::Vector3>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		.def("addProperty", &MGE::PropertySetInterface::addProperty<std::list<std::string>>,
			 DOC(MGE, PropertySetInterfaceTmpl, addProperty))
		
		.def("setProperty", &MGE::PropertySetInterface::setProperty<bool>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<int>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<float>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<std::string>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<Ogre::Vector2>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<Ogre::Vector3>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		.def("setProperty", &MGE::PropertySetInterface::setProperty<std::list<std::string>>,
			 DOC(MGE, PropertySetInterfaceTmpl, setProperty))
		;
}
#endif
