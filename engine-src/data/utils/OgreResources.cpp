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

-- Based on / Inspired by: Ogre Resource System, original copyright information follows --
Copyright (c) 2000-2014 Torus Knot Software Ltd
Distributed under the MIT license terms (see ADDITIONAL_COPYRIGHT/Ogre.txt file)
-- End original copyright --
*/

#include "data/utils/OgreResources.h"
#include "data/property/G11n.h"

#include "LogSystem.h"
#include "ConfigParser.h"

#include "config.h"

#include <OgreString.h>
#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#ifdef TARGET_SYSTEM_IS_UNIX
	#include <glob.h>
#else
	#include <Poco/Glob.h>
	#define USE_POCO_GLOB 1
#endif

#undef LOG_MODULE_NAME
#define LOG_MODULE_NAME "Setup resources"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG_GET_FILE_PRIORITY_LOG(a) LOG_DEBUG(a)
#else
#define DEBUG_GET_FILE_PRIORITY_LOG(a)
#endif

/**
@page XMLSyntax_ResourcesConfig Resources XML syntax

@section ResourcesConfig Ogre Resources System Config

@subsection XMLNode_ResourcesConfig \<Resources\>

\<Resources\> node use next xml sub nodes (in any combination):
  - @c \<Group\> with attribute @c name (for specify created resources group name) and @ref XMLNode_ResourcesConfigEntry subnodes
  - @c \<ResourcesConfigFile\> with path to resources configuration file (with next \<Resources\> xml element as root node),
    path can contain shell-style patterns (@c *, @c ?, etc) and is used to search matching files

@subsection XMLNode_ResourcesConfigEntry \<Entry\>

\<Entry\> node use next attributes:
  - @c type for resource entry type:
    - @c dir for filesystem directory entry
    - any value suppoorted as locType by Ogre::ResourceGroupManager::addResourceLocation (eg. @c Zip for zip file archive)
  - @c path for resource entry path (related to game working directory)
  - @c doInit when set to false do not initialise this resource group (useful for mods adding some resources to standard map groups)
  - @c recursionMode optional and use only when type=="dir", for selecting recursion type, support next string values:
    - @c asFiles do recursion over filesystem and add each resource individually (resource name is not relationship with directory tree)
    - @c asSubPaths call Ogre::ResourceGroupManager::addResourceLocation with recursive=true (resource names are prefixed by sub-directory names)
*/

void MGE::OgreResources::processResourcesXMLNode(const pugi::xml_node& xmlNode) {
	for (auto xmlSubNode : xmlNode) {
		std::string_view xmlSubNodeName = xmlSubNode.name();
		if (xmlSubNodeName == "Group") {
			Ogre::String groupName = xmlSubNode.attribute("name").as_string();
			if (groupName.empty()) {
				LOG_WARNING("ignore resources group without name");
				continue;
			}
			processResourcesEntriesXMLNode(groupName, xmlSubNode);
			if (xmlSubNode.attribute("doInit").as_bool(true)) {
				LOG_INFO("initialise resources group " << groupName);
				getSingleton().initialiseResourceGroup(
					groupName,
					true /* => temporary change locale to "C" => decimal point always is dot */
				);
			}
		} else if (xmlSubNodeName == "ResourcesConfigFile") {
			std::string filePath = xmlSubNode.text().as_string();
			if (filePath.empty()) {
				LOG_WARNING("ignore <ResourcesConfigFile> without file path");
				continue;
			}
			#ifdef USE_POCO_GLOB
			std::set<std::string> files;
			Poco::Glob::Glob::glob(filePath, files);
			for (auto& iter : files) {
				filePath = iter;
			#else
			glob_t globbuf;
			glob(filePath.c_str(), 0, NULL, &globbuf);
			for (size_t i=0; i<globbuf.gl_pathc; ++i) {
				filePath = globbuf.gl_pathv[i];
			#endif
				LOG_INFO("processing \"" << filePath << "\" resources config file");
				
				pugi::xml_document subFileXML;
				processResourcesXMLNode( MGE::XMLUtils::openXMLFile(subFileXML, filePath.c_str(), "Resources") );
			}
			#ifndef USE_POCO_GLOB
			globfree(&globbuf);
			#endif
		}
	}
}

void MGE::OgreResources::processResourcesEntriesXMLNode(const Ogre::String& groupName, const pugi::xml_node& xmlNode) {
	for (auto xmlSubNode : xmlNode.children("Entry")) {
		processResourcesEntryXMLNode(groupName, xmlSubNode);
	}
}

void MGE::OgreResources::processResourcesEntryXMLNode(const Ogre::String& groupName, const pugi::xml_node& xmlNode) {
	std::string type = xmlNode.attribute("type").as_string();
	std::string path = xmlNode.attribute("path").as_string();
	
	if (getSingleton().resourceLocationExists( path, groupName )) {
		LOG_INFO("Location " + path  + " is currently in " + groupName  + " group ... skiping");
		return;
	}
	
	if (type == "dir") {
		std::string_view recursionMode = xmlNode.attribute("recursionMode").as_string();
		if (recursionMode == "asFiles") {
			LOG_INFO("add " << path << " to resources group " << groupName << " with MGE recursion (asFiles)");
			recursiveAdd( groupName, std::filesystem::path(path) );
		} else if (recursionMode == "asSubPaths") {
			LOG_INFO("add " << path << " to resources group " << groupName << " with Ogre recursion (asSubPaths)");
			getSingleton().addResourceLocation( path, "FileSystem", groupName, true );
		} else {
			LOG_INFO("add " << path << " to resources group " << groupName << " without recursion");
			getSingleton().addResourceLocation( path, "FileSystem", groupName, false );
		}
	} else {
		LOG_INFO("add " << path << " as " << type << " to resources group " << groupName << " without recursion");
		getSingleton().addResourceLocation( path, type, groupName, false );
	}
}

void MGE::OgreResources::recursiveAdd(const Ogre::String& groupName, const std::filesystem::path& path) {
	std::string spath = path.string();
	LOG_INFO("add " << spath);
	getSingleton().addResourceLocation( spath, "FileSystem", groupName, false );
	
	std::filesystem::recursive_directory_iterator end_iter;
	std::filesystem::recursive_directory_iterator iter(path);
	for(; iter != end_iter; ++iter) {
		if (std::filesystem::is_directory(iter->path())) {
			recursiveAdd( groupName, iter->path() );
		}
	}
}

/**
@page XMLSyntax_MainConfig

@subsection ResourcesConfigMainFileNode \<Resources\>

@c \<Resources\> used for top level configuration of <b>Ogre Resources System</b>. This node (in main config file) is standard @ref XMLNode_ResourcesConfig node.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(Resources) {
	LOG_HEADER("Initialise Resources");
	
	if (!xmlNode) {
		LOG_WARNING("Initialise Resources", "xmlNode is empty");
	}
	
	auto system = static_cast<MGE::OgreResources*>(Ogre::ResourceGroupManager::getSingletonPtr());
	system->processResourcesXMLNode(xmlNode);
	system->processResourcesXMLNode(xmlNode);
	
	return system;
}


///////////////////  utility functions - get path, get priority, ...  ///////////////////

#undef LOG_MODULE_NAME
#define LOG_MODULE_NAME ""

int MGE::OgreResources::getXMLFilePriority(const std::string& filePath, const std::string_view& rootNodeName) {
	DEBUG_GET_FILE_PRIORITY_LOG("getXMLFilePriority for file: " << filePath << " with rootNodeName: " << rootNodeName);
	
	const std::string tagName("<"+rootNodeName);
	const std::string attribName("priority=\"");
	
	// we don't use xml parser to avoid read and parse full file
	// we only read file until we found end of opening root node tag
	
	std::string   xmlTag;
	std::ifstream xmlFile;
	xmlFile.open(filePath.c_str());
	
	// 0) read file "line" by "line", when "line" is delimiter by XML tag closing char ('>')
	while ( std::getline(xmlFile, xmlTag, '>') ) {
		// 1) search opening XML tag with 'rootNodeName' in current "line"
		DEBUG_GET_FILE_PRIORITY_LOG("1. " << xmlTag);
		auto pos = xmlTag.find(tagName);
		if (pos != std::string::npos) { 
			// 2) found '<rootNodeName' substring
			DEBUG_GET_FILE_PRIORITY_LOG("2. " << xmlTag.data()+pos);
			pos += tagName.length();
			// 3) check if after '<rootNodeName' is space or tabulation
			DEBUG_GET_FILE_PRIORITY_LOG("3. " << xmlTag.data()+pos);
			if (xmlTag[pos] == ' ' || xmlTag[pos] == '\t' || xmlTag[pos] == '\0') { // don't check length ... in the worst case xmlTag[pos] == '\0'
				// 4) search attribute name ... in loop because tag can have multiple attributes ending with attribName
				DEBUG_GET_FILE_PRIORITY_LOG("4. " << xmlTag.data()+pos);
				while( (pos = xmlTag.find(attribName, pos)) != std::string::npos ) {
					// 5) found 'attribName' substring
					DEBUG_GET_FILE_PRIORITY_LOG("5. " << xmlTag.data()+pos);
					pos += attribName.length();
					// 6) check if before 'attribName' is space or tabulation
					DEBUG_GET_FILE_PRIORITY_LOG("6. " << xmlTag.data()+pos);
					if (pos != std::string::npos && (xmlTag[pos-1] == ' ' || xmlTag[pos-1] == '\t')) {
						// 7) if it's, we found attribute → return attribute value as number
						LOG_DEBUG("7. " << xmlTag.data()+pos);
						int priority = std::strtoll(xmlTag.data()+pos, nullptr, 0); // don't search ending '"', because strtoll end parsing on non number char
						LOG_VERBOSE("file " << filePath << " has priority " << priority);
						return priority;
					}   // 7b) if it isn't, continue loop to try found next matching candidate
				}
				// 5b) if 'rootNodeName' tag don't have attribName don't try next node ... just return 0;
				LOG_VERBOSE("getXMLFilePriority", "file " << filePath << " don't have priority - using 0");
				return 0;
			}
		}   // 2b) if not found '<rootNodeName' substring, get next "line"
	}
	
	LOG_WARNING("getXMLFilePriority", "file " << filePath << " don't have correct root XML node: " << rootNodeName);
	return 0;
}

int MGE::OgreResources::getResourcePaths(
	const Ogre::String& file, const Ogre::String& group,
	std::list<std::string>* paths,
	bool unique, const std::string_view& rootNodeName
) {
	Ogre::FileInfoListPtr filesInfo = getSingleton().findResourceFileInfo(
		group, file
	);
	
	if (filesInfo->size() == 0) {
		LOG_WARNING("Not found file \"" + file + "\" in group \"" + group + "\"");
		printResourceGroup(group);
		return 0;
	}
	
	if (unique) {
		// paths can have only one elements
		if (filesInfo->size() == 1) {
			paths->push_back( filesInfo->front().archive->getName() + "/" + filesInfo->front().filename );
		} else {
			LOG_WARNING("Found multiple files \"" + file + "\" in group \"" + group + "\"");
			printResourceGroup(group);
			return 0;
		}
	} else if (rootNodeName.empty()) {
		// paths is unsorted
		for (auto& f : *filesInfo) {
			paths->push_back(f.archive->getName() + "/" + f.filename);
		}
	} else {
		// paths is sorted by priority
		std::list<int> prioList;
		for (auto& f : *filesInfo) {
			std::string filePath = f.archive->getName() + "/" + f.filename;
			int filePriority = getXMLFilePriority(filePath, rootNodeName);
			
			auto prioIt = prioList.begin();
			auto pathIt = paths->begin();
			
			// increament both iterator until filePriority is less then value in prioList or list ends
			while (prioIt != prioList.end() && filePriority < *prioIt) {
				prioIt++; pathIt++;
			}
			prioList.insert(prioIt, filePriority);
			paths->insert(pathIt, filePath);
		}
	}
	
	LOG_VERBOSE("getResourcePath for " << file << " in " << group << " find " << paths->size() << " paths, first path is: " << paths->front());
	
	return paths->size();
}

std::string MGE::OgreResources::getResourcePath(const Ogre::String& file, const Ogre::String& group, const std::string_view& rootNodeName) {
	std::list<std::string> res;
	if (rootNodeName.empty()) {
		if (getResourcePaths(file, group, &res, true))
			return res.front();
	} else {
		if (getResourcePaths(file, group, &res, false, rootNodeName))
			return res.front();
	}
	return MGE::EMPTY_STRING;
}

std::string MGE::OgreResources::getResourcePath(const pugi::xml_node& xmlNode, const std::string& defaultGroup) {
	std::string filePath  = xmlNode.attribute("path").as_string();
	if (filePath.empty()) {
		filePath = getResourcePath(
			xmlNode.attribute("name").as_string(),
			xmlNode.attribute("group").as_string(defaultGroup.c_str())
		);
	}
	return filePath;
}

void MGE::OgreResources::printResourceGroup(const Ogre::String& groupName) {
	LOG_INFO("resource group " << groupName << " contains:");
	auto locations = getSingleton().getResourceLocationList(groupName);
	for (auto& l : locations) {
		LOG_INFO(" - " << l->archive->getName());
		auto files = l->archive->list();
		for (auto& f : *files) {
			LOG_INFO("   - " << f);
		}
	}
}
