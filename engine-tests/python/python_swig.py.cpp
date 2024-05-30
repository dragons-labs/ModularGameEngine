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

#include "pybind11_swig_cast.inl"

#include <OgreVector2.h>
#include <OgreSceneManager.h>

PYBIND11_SWIG_GENERATE_CAST_FULL(Ogre, Vector2) // <=> PYBIND11_SWIG_GENERATE_CAST(Ogre::Vector2, "Ogre.Vector2", PYBIND11_SWIG_CONTRUCTABLE)
PYBIND11_SWIG_GENERATE_CAST_ONLYPTR(Ogre, SceneManager) // <=> PYBIND11_SWIG_GENERATE_CAST(Ogre::SceneManager, "Ogre.SceneManager", PYBIND11_SWIG_NON_CONTRUCTABLE)


#include <sstream>
Ogre::Vector2 v1(13, 17);

Ogre::Vector2* getV1() {
	return &v1;
}

void showV1() {
	std::stringstream out;
	out << "showV1: " << v1; // compose text in C++ because we test C++ code here
	pybind11::print(out.str()); // use python print for printing (because we want catch output as python script output)
}

Ogre::Vector2 getV2() {
	return Ogre::Vector2(3, 15);
}

void showV2(Ogre::Vector2& x) {
	std::stringstream out;
	out << "showV2: " << x; // compose text in C++ because we test C++ code here
	pybind11::print(out.str()); // use python print for printing (because we want catch output as python script output)
}

PYBIND11_MODULE(swig_test, m) {
    m.def("getV1", &getV1, pybind11::return_value_policy::reference);
    m.def("getV2", &getV2);
    m.def("showV1", &showV1);
    m.def("showV2", &showV2);
}


/*
swig -python -external-runtime; c++ -O3 -Wall -shared -std=c++17 -fPIC -I. -I ../engine-src/utils/ -I /usr/include/python3.10/ -I /usr/local/include/OGRE/ python_swig.py.cpp /usr/local/lib/libOgreMain.so -o swig_test$(python3-config --extension-suffix)
*/

