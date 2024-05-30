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

#include "ConfigParser.h"
#include "XmlUtils.h"
#include "LogSystem.h"

#undef  LOG_MODULE_NAME
#define LOG_MODULE_NAME "ConfigParser"

void MGE::ConfigParser::initMainConfig(null_end_string path, MGE::null_end_string rootNodeName) {
	LOG_INFO("Parse main config file: " << path);
	
	mainConfigRootNode = MGE::XMLUtils::openXMLFile(mainConfig, path, rootNodeName);
}

pugi::xml_node MGE::ConfigParser::getMainConfig(null_end_string nodeName) {
	return mainConfigRootNode.child(nodeName);
}

int MGE::ConfigParser::createAndConfigureModules(LoadedModulesSet& createdModules, const std::string_view& xmlNodeName, const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, int runlevel) {
		LOG_INFO("Start processing XML node \"" << xmlNode.name() << "\" used for create module \"" << xmlNodeName << "\"");
		
		LoadedModuleInfo info;
		info.id = xmlNodeName;
		info.runlevel = runlevel;
		
		int ret = 0;
		for (auto&& [tagName, listener] : MGE::Range(configParserListeners.listeners, xmlNodeName)) {
			info.ptr = (listener)(xmlNode, context);
			if (info.ptr) {
				createdModules.insert(info);
				++ret;
			}
		}
		if (ret) {
			LOG_INFO("Finish processing XML node and create module \"" << xmlNodeName << "\", successfully call " << ret << " config parser listeners");
		} else {
			LOG_WARNING("Not found config parser listeners for XML node \"" << xmlNodeName << "\" or listeners call fail.");
		}
		
		return ret;
}

void MGE::ConfigParser::createAndConfigureModules(LoadedModulesSet& createdModules, const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, int runlevel) {
	LOG_INFO("ConfigParser", "Create and configure modules based on XML config node: " << xmlNode.name());
	
	for (auto& xmlSubNode : xmlNode) {
		createAndConfigureModules(createdModules, xmlSubNode.name(), xmlSubNode, context, runlevel);
	}
}

bool MGE::ConfigParser::addConfigParseListener(const std::string_view& tagName, SceneConfigParseFunction callbackFunction) {
	// do NOT use LOG here - addConfigParseListener() can be used in global-constructors (before create default log)
	return configParserListeners.addListener(callbackFunction, tagName);
}

void MGE::ConfigParser::remConfigParseListener(SceneConfigParseFunction callbackFunction) {
	configParserListeners.remListener(callbackFunction);
}

void MGE::ConfigParser::listListeners() {
	LOG_VERBOSE("ConfigParser", "Registered XML node names:");
	for (auto&& [tagName, listener] : configParserListeners.listeners) {
		LOG_VERBOSE("ConfigParser", " * " << tagName);
	}
}
