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

#include "data/utils/OgreResources.h"

#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(OgreResources) {
	m.def("getResourcePaths", py::overload_cast<const Ogre::String&, const Ogre::String&, bool, const std::string_view&>( &MGE::OgreResources::getResourcePaths ),
		DOC(MGE, OgreResources, getResourcePaths, 2)
	);
	m.def("getResourcePath", py::overload_cast<const Ogre::String&, const Ogre::String&, const std::string_view&>( &MGE::OgreResources::getResourcePath ),
		DOC(MGE, OgreResources, getResourcePath, 2)
	);
}
#endif
