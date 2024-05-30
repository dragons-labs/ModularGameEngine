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

#pragma   once

#include "data/structs/BaseComponent.h"
#include "data/structs/ComponentsCollection.h"
#include "utils/BaseClasses.h"

#include <unordered_map>
#include <map>

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief factory for game objects Component
 */
struct ComponentFactory :
	public MGE::Singleton<ComponentFactory>
{
public:
	/// type of static function for register in @ref registerComponent
	///
	/// @param      parent        pointer to parent (actor or prototype) object for created component
	/// @param      config        pointer to optional XML config for creating component (can be empty, in this case components that need this config can trow exception)
	/// @param[out] typeIDs       pointer to std::set for put all typeIDs with which component should be registered
	/// @param[in]  createdForID  value of typeID for which component will be created
	typedef MGE::BaseComponent* (*ComponentCreator)(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID);
	
	/**
	 * @brief register component in factory (by classID)
	 *
	 * @param classID         unique numeric id identifying registered component class
	 * @param stringClassID   string coresponding to @classID for use in @ref nameToID map for parse config files
	 * @param creatorFunction pointer to static function creating component class instance (and return pointer to it)
	 */
	void registerComponent(int classID, const std::string& stringClassID, ComponentCreator creatorFunction);
	
	/**
	 * @brief create and return component based on classID
	 *
	 * @param classID         numeric id identifying component class to create
	 * @param components      pointer to map of component (for check if typeIDs used by creating component are free and to register it with all typeIDs)
	 * @param parent          pointer to parent (actor or prototype) object for created component
	 * @param config          pointer to optional XML config for creating component (can be empty)
	 */
	MGE::BaseComponent* createComponent(
		int classID,
		ComponentsCollection* components,
		MGE::NamedObject* parent,
		const pugi::xml_node& config = MGE::EMPTY_XML_NODE
	);
	
	/// @{
	/**
	 * @brief store components collection to xml serialization archive
	 * 
	 * @param[in,out]  xmlArch      xml archive object
	 * @param[in]      mapPtr       pointer to map of components to store
	 */
	void storeComponents( pugi::xml_node& xmlNode, const ComponentsCollection* mapPtr );
	FORCE_INLINE void storeComponents( pugi::xml_node&& xmlNode, const ComponentsCollection* mapPtr ) {
		storeComponents(xmlNode, mapPtr);
	}
	/// @}
	
	/**
	 * @brief restore components collection from xml
	 * 
	 * @param[in]  xmlArchive   xml archive object, with pointer to xml node which will be using for load state of this object
	 * @param[in]  mapPtr       pointer to map of components to put restored components pointers
	 * @param[in]  parent       pointer to owner of component map (actor / prototype)
	 * @param[in]  sceneNode    pointer to main scene node of @a parent (can be NULL)
	 * @param[in]  callInit     if false do not call init() on restored components
	 */
	void restoreComponents(
		const pugi::xml_node& xmlNode,
		ComponentsCollection* mapPtr,
		MGE::NamedObject* parent,
		Ogre::SceneNode* sceneNode,
		bool callInit = true
	);
	
	/**
	 * @brief destroy single component from map
	 * 
	 * @param      typeID       numeric type of component to destroy
	 * @param      mapPtr       pointer to map of components to clear
	 * 
	 * @note really this function DO NOT remove entry from map but only put in this entry 0
	 */
	static void removeFromMap(
		int typeID,
		ComponentsCollection* mapPtr
	);
	
	/**
	 * @brief destroy all components from map and clear it
	 * 
	 * @param      mapPtr       pointer to map of components to clear
	 */
	static void clearMap(
		ComponentsCollection* mapPtr
	);
	
	/// return numeric value of typeID / classID from string, use nameToID to parse names
	int getID(null_end_string str) const;
	
	/// return "name" string for numeric value of typeID / classID use idToNames to parse ids
	std::string getName(int typeID) const;
	
	/// constructor ... register components by processing @ref MGE::ComponentFactoryRegistrar
	ComponentFactory();
	
protected:
	/// map of classID and pointer to ComponentCreator functions for all registered components
	std::unordered_map<int, ComponentCreator> registeredComponents;
	
	//@{
	/// map of names for component (names can be used in config file instead of numeric id)
	std::unordered_map<std::string, int> nameToID;
	std::unordered_map<int, std::string> idToNames;
	//@}
	
protected:
	~ComponentFactory() = default;
};


/// @}

}
