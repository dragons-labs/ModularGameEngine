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

#include "data/structs/BasePrototype.h"
#include "utils/BaseClasses.h"

#include <unordered_map>

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief factory for game objects prototypes
 */
struct PrototypeFactory :
	public MGE::Singleton<PrototypeFactory>
{
public:
	/**
	 * @brief return actor prototype identifying by @a name,
	 *        when can't find and @a name and @a configFile and @a configGroup are non empty string create and return prototype
	 * 
	 * @param name            name of prototype to get
	 * @param configFileName  (optional) config file name to load (create new) prototype
	 * @param configFileGroup (optional) config file group to load (create new) prototype
	 */
	MGE::BasePrototype* getPrototype(
		const std::string_view& name,
		const std::string_view& fileName  = MGE::EMPTY_STRING_VIEW,
		const std::string_view& fileGroup = MGE::EMPTY_STRING_VIEW
	);
	
	/**
	 * @brief return actor prototype identifying by LocationInfo
	 * 
	 * @param config  reference to LocationInfo object with description (name, config file and group) of prototype (as node attributes)
	 */
	inline MGE::BasePrototype* getPrototype(const MGE::ResourceLocationInfo& config) {
		return getPrototype( config.name, config.fileName, config.fileGroup );
	}
	
	/**
	 * @brief return actor prototype identifying by XML config
	 * 
	 * @param xmlNode  pointer to xml node with description (name, config file and group) of prototype (as node attributes)
	 */
	inline MGE::BasePrototype* getPrototype(const pugi::xml_node& xmlNode) {
		return getPrototype( MGE::ResourceLocationInfo( xmlNode ) );
	}
	
	/// constructor
	PrototypeFactory();
	
protected:
	friend struct BasePrototypeImpl;
	
	/// list of all prototypes for game object (as map name -> object pointer)
	std::unordered_map<std::string, MGE::BasePrototype*, MGE::string_hash, std::equal_to<>> allPrototypes;
	
protected:
	~PrototypeFactory() = default;
};


/// @}

}
