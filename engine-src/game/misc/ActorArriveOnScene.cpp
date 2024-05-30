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
#include "data/structs/BasePrototype.h"
#include "data/structs/components/3DWorld.h"
#include "data/structs/components/ObjectOwner.h"
#include "data/structs/factories/ActorFactory.h"
#include "data/utils/NamedSceneNodes.h"

#include "modules/gui/WorldMap.h"

#include <OgreSceneNode.h>

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE {
	void worldMapUnitOnTheActionSite(const MGE::BasePrototype* proto, const std::unordered_map<MGE::BasePrototype*, int>& personel) {
		// create actor
		Ogre::SceneNode* entryPoint = MGE::NamedSceneNodes::getSceneNode("EntryPoint4Cars");
		MGE::BaseActor* actor = MGE::ActorFactory::getPtr()->createActor(
			proto,
			MGE::EMPTY_STRING,
			entryPoint->getPosition(),
			entryPoint->getOrientation()
		);
		
		// add to actor owned personel
		for (auto& iter : personel) {
			actor->getComponent<MGE::ObjectOwner>(MGE::ObjectOwner::classID, MGE::ObjectOwner::classID)->set(
				iter.first, iter.second, iter.second
			);
		}
	}
}
#endif

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_InitSceneObjects \<ActorArriveFromWorldMapToScene\>

@c \<ActorArriveFromWorldMapToScene\> is used in @ref MapConfig to configure world map to create actor in MGE::WorldMap::unitOnTheActionSite call.
Should be used after @ref XMLSyntax_WorldMapConfig xml node.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(ActorArriveFromWorldMapToScene) {
	if (MGE::WorldMap::getPtr()) {
		MGE::WorldMap::getPtr()->unitOnTheActionSite = MGE::worldMapUnitOnTheActionSite;
	}
	return nullptr;
}
