/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "SceneLoader.h"

#include "LogSystem.h"
#include <pugixml.hpp>

bool MGE::SceneLoader::addSceneNodesCreateListener(const std::string_view& tagName, SceneNodesCreateFunction callbackFunction) {
	return sceneNodesCreateListeners.addListener(callbackFunction, tagName);
}

void MGE::SceneLoader::remSceneNodesCreateListener(SceneNodesCreateFunction callbackFunction) {
	sceneNodesCreateListeners.remListener(callbackFunction);
}

void MGE::SceneLoader::parseSceneXMLNode(
	const pugi::xml_node&        xmlNode,
	const MGE::LoadingContext*   context,
	const MGE::SceneObjectInfo&  parent
) {
	for (auto xmlSubNode : xmlNode) {
		std::string_view xmlSubNodeName = xmlSubNode.name();
		
		if (xmlSubNodeName.empty())
			continue;
		
		auto range = MGE::Range(sceneNodesCreateListeners.listeners, xmlSubNodeName);
		for (auto&& [tagName, listener] : range) {
			LOG_VERBOSE("SceneLoader", "parse tag: " << xmlSubNodeName);
			(listener)(xmlSubNode, context, parent);
		}
		#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
		if (range.begin() == sceneNodesCreateListeners.listeners.end()) {
			LOG_DEBUG("SceneLoader", "ignoring unregistered tag: " << xmlSubNodeName);
			// this is not an error, because this is called also for some other subnodes handled internally
			// by function call parseSceneXMLNode (in recursive calling case)
		#endif
		}
	}
}

void MGE::SceneLoader::listListeners() {
	LOG_VERBOSE("SceneLoader", "Registered XML node names:");
	for (auto&& [tagName, listener] : sceneNodesCreateListeners.listeners) {
		LOG_VERBOSE("SceneLoader", " * " << tagName);
	}
}
