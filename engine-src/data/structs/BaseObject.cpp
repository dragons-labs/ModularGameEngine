/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/BaseObject.h"
#include "data/structs/factories/ActorFactory.h"
#include "data/structs/factories/PrototypeFactory.h"

/**
@page XMLSyntax_BasicElements

@section MGERefElementsXML XML Syntax for game engine object (by name) reference

@subsection XMLNode_PrototypeRef \<Prototype\>

@c \<Prototype\> xml node is used for specify prototype config source by attributes:
    - @c name  name of prototype
    - @c file  file name with prototype config (see @ref XMLSyntax_PrototypeConfigFile)
    - @c group resources group with this file

@subsection XMLNode_ActorName \<ActorName\>

@c \<ActorName\> is used for store actor name as node value.
*/

MGE::NamedObject* MGE::NamedObject::get(const pugi::xml_node& xmlNode) {
	pugi::xml_node xmlSubNode;
	if ( (xmlSubNode = xmlNode.child("ActorName")) ) {
		return MGE::ActorFactory::getPtr()->getActor(
			xmlSubNode.text().as_string()
		);
	} else if ( (xmlSubNode = xmlNode.child("Prototype")) ) {
		return MGE::PrototypeFactory::getPtr()->getPrototype(xmlSubNode);
	} else {
		return NULL;
	}
}
