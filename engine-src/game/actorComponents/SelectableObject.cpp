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

#include "game/actorComponents/SelectableObject.h"

#include "pragma.h"
#include "MessagesSystem.h"
#include "Engine.h"

#include "data/structs/BaseActor.h"
#include "data/structs/components/3DWorld.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "data/structs/ActorMessages.h"

#include <OgreSceneNode.h>
#include <stdlib.h>

MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors")

std::set<MGE::SelectableObject*> MGE::SelectableObject::allSelectableObject;

MGE_CLANG_WARNING_POP


MGE::SelectableObject::SelectableObject(MGE::NamedObject* parent) :
	status(0), miniMapSymbolWidth(0), miniMapSymbolHeight(0), miniMapSymbol(NULL)
{
	owner = static_cast<MGE::BaseActor*>(parent);
	allSelectableObject.insert(this);
}

MGE::SelectableObject::~SelectableObject() {
	allSelectableObject.erase(this);
	delete miniMapSymbol;
}

MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(MGE::SelectableObject, SelectableObject)


bool MGE::SelectableObject::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_attribute("selectionMask") <<  status;
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_SelectableObject SelectableObject

Store / restore from its @c \<Component\> subnodes:
  - @c SelectionMask
    - numeric or space delimited list of strings value of selectionMask
    - strings will be converted to numeric by @ref MGE::SelectableObject::stringToStatusMask
      (can be a space delimited list of flags – internal use @ref MGE::SelectableObject::stringToStatusFlag and @ref MGE::Utils::stringToNumericMask)
    - strings is literally identical to @ref MGE::SelectableObject::StatusFlags enum elements names
  - @c \<MiniMapSymbol\> (optional) with required attributes @c width and @c height,
    content of this node is hex encoded of ARGB (4bit per channel) picture of minimap symbol

@subsubsection ActorComponent_SelectableObject_Example Example
@code{.xml}
	<Component classID="SelectableObject" selectionMask="IS_SELECTABLE">
		<miniMapSymbol width="3" height="3">
			ff00 ff00 ff00
			ff00 ffff ff00
			ff00 ff00 ff00
		</miniMapSymbol>
	</Component>
@endcode
*/
bool MGE::SelectableObject::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	pugi::xml_node xmlSubNode;
	
	xmlSubNode = xmlNode.child("SelectionMask");
	if (xmlSubNode) {
		status = stringToStatusMask( xmlSubNode.text().as_string() );
	}
	
	xmlSubNode = xmlNode.child("MiniMapSymbol");
	if (xmlSubNode) {
		miniMapSymbolWidth  = xmlSubNode.attribute("width").as_int(0);
		miniMapSymbolHeight = xmlSubNode.attribute("height").as_int(0);
		
		std::string_view tmp = xmlSubNode.text().as_string();
		size_t size = miniMapSymbolWidth * miniMapSymbolHeight;
		size_t i = 0, j = 0;
		delete miniMapSymbol;
		miniMapSymbol = new uint16_t[size];
		
		while (i < tmp.length()) {
			if (tmp[i] == ' ' || tmp[i] == '\t' || tmp[i] == '\n') {
				++i;
			} else {
				if (j >= size)
					break;
				
				char buf[4], *bufEnd;
				memcpy( buf, tmp.data() + i, 4 );
				
				miniMapSymbol[j++] = strtol(buf, &bufEnd, 16);
				i = i + (bufEnd - buf);
			}
		}
	}
	return true;
}

void MGE::SelectableObject::setAvailable(bool isAvailable, bool setVisible) {
	if (isAvailable) {
		status &= (~MGE::SelectableObject::IS_UNAVAILABLE);
		MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorAvailableEventMsg(owner), owner );
	} else {
		status |= MGE::SelectableObject::IS_UNAVAILABLE;
		MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::ActorNotAvailableEventMsg(owner), owner );
	}
	
	MGE::World3DObject* w3dObj = owner->getComponent<MGE::World3DObject>();
	if (setVisible && w3dObj)
		w3dObj->getOgreSceneNode()->setVisible(isAvailable);
}


void MGE::SelectableObject::getMiniMapSymbol(const uint16_t*& buf, int& width, int& height) {
	width = miniMapSymbolWidth;
	height = miniMapSymbolHeight;
	buf = miniMapSymbol;
}

MGE::SelectableObject::status_t MGE::SelectableObject::stringToStatusMask(const std::string_view& s) {
	return MGE::StringUtils::stringToNumericMask<MGE::SelectableObject::status_t>(s, &MGE::SelectableObject::stringToStatusFlag);
}
