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

#pragma   once

#include "XmlUtils.h"
#include "EngineModule.h"

#include <list>
#include <filesystem>
#include <OgreCommon.h>
#include <OgreResourceGroupManager.h>

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup OgreWorldUtils
/// @{
/// @file

/**
 * @brief Ogre resources related stuff.
 * 
 * `#include <ogreResources.h>`
 */
class OgreResources : public Ogre::ResourceGroupManager, public MGE::Module {
public:
	/**
	 * @name Processing resources configuration files.
	 * 
	 * @{
	 */
	
	/**
	 * @brief proccess resources xml config file root node
	 * 
	 * @param[in] xmlNode  main \<Resources\> XML node
	 */
	static void processResourcesXMLNode(const pugi::xml_node& xmlNode);
	
	/**
	 * @brief proccess all "Entry" subnodes of @a xmlNode and add it to resources group @a groupName
	 */
	static void processResourcesEntriesXMLNode(const Ogre::String& groupName, const pugi::xml_node& xmlNode);
	
	/**
	 * @brief proccess single "Entry"-type xml node @a xmlNode and internal call addResourceLocation() to add paths to resources group @a groupName
	 */
	static void processResourcesEntryXMLNode(const Ogre::String& groupName, const pugi::xml_node& xmlNode);
	
	/**
	 * @brief recursive add resources from @a path to resource group @a groupName
	 */
	static void recursiveAdd(const Ogre::String& groupName, const std::filesystem::path& path);
	
	/**
	 * @}
	 * 
	 * @name Convert resource name and group to filesystem path(s).
	 * 
	 * @{
	 */
	
	/**
	 * @brief get path(s) to resource @a file from group @a group
	 * 
	 * @param[in]  file          file name (path) inside resource group
	 * @param[in]  group         resource group name
	 * @param[out] paths         pointer to the list (of std::string) to which the found paths will be added (should be empty when pass to this function)
	 * @param[in]  unique        when set to true function end with error when found more than one path
	 * @param[in]  rootNodeName  when not empty use "priority" attribute of this root XML node in found files to sort paths by priority, see too: @ref getXMLFilePriority
	 */
	static int getResourcePaths(
		const Ogre::String& file,
		const Ogre::String& group,
		std::list<std::string>* paths,
		bool unique,
		const std::string_view& rootNodeName = MGE::EMPTY_STRING_VIEW
	);
	
	/**
	 * @brief get path(s) to resource @a file from group @a group
	 * 
	 * @param[in]  file          file name (path) inside resource group
	 * @param[in]  group         resource group name
	 * @param[in]  unique        when set to true function end with error when found more than one path
	 * @param[in]  rootNodeName  when not empty use "priority" attribute of this root XML node in found files to sort paths by priority, see too: @ref getXMLFilePriority
	 */
	inline static std::list<std::string> getResourcePaths(
		const Ogre::String& file,
		const Ogre::String& group,
		bool unique,
		const std::string_view& rootNodeName = MGE::EMPTY_STRING_VIEW
	) {
		std::list<std::string> paths;
		getResourcePaths( file, group, &paths, unique, rootNodeName );
		return paths;
	}
	
	/**
	 * @brief return path resource @a file from group @a group
	 * 
	 * If @a rootNodeName is empty this is shortcut to @ref getResourcePaths whith unique == true (require unique of path).
	 * If @a rootNodeName is not empty this is shortcut to @ref getResourcePaths whith unique == false and return only one (preferred by priority) path.
	 * 
	 * @param[in]  file          file name (path) inside resource group
	 * @param[in]  group         resource group name
	 * @param[in]  rootNodeName  when not empty use "priority" attribute of this root XML node in found files to sort paths by priority, see too: @ref getXMLFilePriority
	 * 
	 * @return Single resource path or (when path not found) empty string.
	 */
	static std::string getResourcePath(
		const Ogre::String& file,
		const Ogre::String& group,
		const std::string_view& rootNodeName = MGE::EMPTY_STRING_VIEW
	);
	
	/**
	 * @brief return path resource based on xml node (with attributes path="" or file="" and group="")
	 * 
	 * when node don't have @c path attribute this is shortcut to @ref getResourcePaths whith unique == true (require unique of path)
	 */
	static std::string getResourcePath(const pugi::xml_node& xmlNode, const Ogre::String& defaultGroup);
	
	/**
	 * @}
	 * 
	 * @name Misc utils.
	 * 
	 * @{
	 */
	
	/**
	 * @brief read file priority from XML root node
	 * 
	 * @param[in]  filePath      system path to file to read priority
	 * @param[in]  rootNodeName  name of root XML node in file to get "priority" attribute
	 */
	static int getXMLFilePriority(const std::string& filePath, const std::string_view& rootNodeName);
	
	/**
	 * @brief print to log all resources from group @a groupName
	 */
	static void printResourceGroup(const Ogre::String& groupName);
	
	/**
	 * @}
	 */
	
protected:
	/// @{
	/// Do NOT create instances of this class.
	OgreResources() = delete;
	OgreResources(const OgreResources&) = delete;
	/// @}
};

/// @}

}
