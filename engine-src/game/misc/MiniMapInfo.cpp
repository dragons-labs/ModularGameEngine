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

#include "game/misc/MiniMapInfo.h"

#include "ConfigParser.h"

#include "data/structs/BaseActor.h"
#include "data/structs/components/3DWorld.h"

void MGE::MiniMapSelectableObjectsInfoProvider::resetMinimapInfo() {
	iter = MGE::SelectableObject::allSelectableObject.begin();
}

bool MGE::MiniMapSelectableObjectsInfoProvider::getNextMinimapInfo(const uint16_t*& buf, int& width, int& height, Ogre::Vector3& worldPos) {
	if (iter == MGE::SelectableObject::allSelectableObject.end())
		return false;
	
	(*iter)->getMiniMapSymbol(buf, width, height);
	worldPos = (*iter)->owner->getComponent<MGE::World3DObject>()->getWorldPosition();
	++iter;
	return true;
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_InitSceneObjects \<MiniMapUseSelectableObjects\>

@c \<MiniMapUseSelectableObjects\> is used in @ref MapConfig to configure mini map to show / operate on all Actors with @ref MGE::SelectableObject component.
Should be used after @ref XMLNode_MiniMap xml node.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(MiniMapUseSelectableObjects) {
	if (MGE::MiniMap::getPtr()) {
		static MGE::MiniMapSelectableObjectsInfoProvider mmsoip;
		MGE::MiniMap::getPtr()->setObjectInfoProvider(&mmsoip);
		return &mmsoip;
	}
	return nullptr;
}
