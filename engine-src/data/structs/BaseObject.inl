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

#ifndef __DOCUMENTATION_GENERATOR__

#include "data/structs/BaseObject.h"
#include "data/structs/factories/ActorFactory.h"
#include "data/structs/factories/PrototypeFactory.h"

template <typename CollectionType> void MGE::NamedObject::insertToCollection(
	const pugi::xml_node& xmlNode, CollectionType* collection
) {
	for (auto xmlSubNode : xmlNode.children("ActorName")) {
		collection->insert(
			MGE::ActorFactory::getPtr()->getActor( xmlSubNode.text().as_string() )
		);
	}
	for (auto xmlSubNode : xmlNode.children("Prototype")) {
		collection->insert(
			MGE::PrototypeFactory::getPtr()->getPrototype(xmlSubNode)
		);
	}
}

template <> void MGE::NamedObject::insertToCollection< std::set<MGE::BaseActor*> >(
	const pugi::xml_node& xmlNode, std::set<MGE::BaseActor*>* collection
) {
	for (auto xmlSubNode : xmlNode.children("ActorName")) {
		collection->insert(
			MGE::ActorFactory::getPtr()->getActor( xmlSubNode.text().as_string() )
		);
	}
}

#endif
