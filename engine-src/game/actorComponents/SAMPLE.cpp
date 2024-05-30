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

#include "game/actorComponents/SAMPLE.h"

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"

/*#include "data/structs/BaseActor.h"*/

MGE::SAMPLE::SAMPLE(MGE::NamedObject* parent) /*:
	owner( static_cast<MGE::BaseActor*>(parent) )*/
{
	/* do NOT use parent components here (could not exist yet) ... can be used in init() */
}

MGE::SAMPLE::~SAMPLE() {
}

// MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(MGE::SAMPLE, SAMPLE)
//  or
// MGE_ACTOR_COMPONENT_CREATOR(MGE::SAMPLE, SAMPLE) { /* setup code here */ }


bool MGE::SAMPLE::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	return true;
}

/* *
@page XMLSyntax_ActorComponent

@subsection ActorComponent_SAMPLE SAMPLE

Use subnodes:
  - @c \<SAMPLE\> for ...
*/
bool MGE::SAMPLE::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	/* do NOT use parent components here (could not exist yet) ... can be used in init() */
	return true;
}

/*void MGE::SAMPLE::init(MGE::NamedObject* parent) {
}*/
