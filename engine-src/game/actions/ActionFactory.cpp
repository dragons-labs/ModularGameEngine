/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "game/actions/ActionFactory.h"

#include "LogSystem.h"
#include "ConfigParser.h"

#include "data/utils/OgreResources.h"
#include "data/structs/BaseActor.h"
#include "game/actions/ActionPrototype.h"
#include "game/actions/Action.h"

#include "game/actions/ActionExecutor.h" // only for use in MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_Actions \<Actions\>

@c \<Actions\> is used for configure set of available actions. This xml node contains a set of @c \<File\> subnodes. Each @c \<File\> subnode have attributes:
  - @c name   file name of actions configuration file
  - @c group  resource group to search this file.

Each actions configuration file is XML file with @c \<Actions\> root node, that contains a set of @ref XMLNode_Action subnodes.
@c \<Actions\> root node can have optional @c priority argument, it value is used to select between files with this same name and this same resource group (default 0, used is file with highest value).
*/
MGE::ActionFactory::ActionFactory(const pugi::xml_node& xmlNode) : 
	MGE::Unloadable(300)
{
	for (auto xmlSubNode : xmlNode.children("File")) {
		std::list<std::string> pathList;
		MGE::OgreResources::getResourcePaths(
			xmlSubNode.attribute("name").as_string(),
			xmlSubNode.attribute("group").as_string("ActionsConfig"),
			&pathList, false
		);
		for (auto& path : pathList)
			loadActionsFromFile(path);
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(Actions) {
	if (!MGE::ActionExecutor::getPtr())
		new MGE::ActionExecutor();
	return new MGE::ActionFactory(xmlNode);
}

MGE::ActionFactory::~ActionFactory() {
	LOG_INFO("destroy ActionFactory ... destroy all action prototypes");
	
	auto iter = allActions.begin();
	while(iter != allActions.end()) {
		MGE::ActionPrototype* a = iter->second;
		++iter;
		delete a;
	}
	allActions.clear();
}

void MGE::ActionFactory::loadActionsFromFile(const std::string& file) {
	LOG_INFO("Load ActionPrototypes from: " + file);
	
	pugi::xml_document xmlFile;
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(xmlFile, file.c_str(), "Actions");
	
	int filePriority = xmlRootNode.attribute("priority").as_int(0);
	
	for (auto xmlSubNode : xmlRootNode.children("Action")) {
		// create action prototype
		MGE::ActionPrototype* actionProto = new MGE::ActionPrototype( xmlSubNode );
		
		// check unique names
		auto currentAction = allActions.find(actionProto->name);
		if (currentAction != allActions.end()) {
			if (currentAction->second->priority < filePriority) {
				LOG_INFO("Remove old \"" << actionProto->name << "\" action prototype with priority=" << currentAction->second->priority << ", new priority=" << filePriority);
				delete currentAction->second;
			} else {
				LOG_INFO("Ignore new \"" << actionProto->name << "\" action prototype with priority=" << filePriority << ", old priority=" << currentAction->second->priority);
				delete actionProto;
				continue;
			}
		}
		
		// register action
		actionProto->priority = filePriority;
		allActions[actionProto->name] = actionProto;
		
		LOG_INFO("Created action prototype: " << actionProto->name << " with type=" << actionProto->type << " and needMask=" << actionProto->needMask << " @ " << actionProto);
	}
}

MGE::ActionPrototype* MGE::ActionFactory::getAction(const std::string_view& name) {
	if (name == "NULL")
		return NULL;
	
	auto iter = allActions.find( name );
	if (iter != allActions.end()) {
		return iter->second;
	}
	
	LOG_DEBUG("Can't find action prototype: " << name);
	return NULL;
}
