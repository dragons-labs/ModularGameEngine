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

-- Based on / Inspired by: Ogre Property Component, original copyright information follows --
Copyright (c) 2000-2014 Torus Knot Software Ltd
Distributed under the MIT license terms (see ADDITIONAL_COPYRIGHT/Ogre.txt file)
-- End original copyright --
*/

#pragma   once

#include "force_inline.h"

#include "data/property/PropertySetInterface.h"
#include "data/property/Any.h"

#include <map>
#include <typeindex>

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

/**
 * @brief Class representing and manage (single) property set, see too @ref XMLSyntax_Property.
 */
class PropertySet : public MGE::PropertySetInterface
{
public:
	/// constructor
	PropertySet() {}
	
	/// destructor
	virtual ~PropertySet() = default;
	
	
	/// @copydoc MGE::PropertySetInterface::getProperty
	const MGE::Any& getProperty(const std::string_view& key) const override {
		auto iter = properties.find(key);
		if (iter != properties.end()) {
			return iter->second;
		}
		
		return MGE::Any::EMPTY;
	}
	
	/// @copydoc MGE::PropertySetInterface::remProperty
	size_t remProperty(const std::string_view& key) override {
		#if __cplusplus > 202002L /* C++23 */
		return properties.erase(key);
		#else
		int count = 0;
		for (auto iter = properties.find(key); iter != properties.end(); iter = properties.find(key)) {
			++count;
			properties.erase(iter);
		}
		return count;
		#endif
	}
	
	/// @copydoc MGE::PropertySetInterface::addProperty(const std::string_view&, const MGE::Any&, bool replace)
	bool addProperty(const std::string_view& key, const MGE::Any& val, bool replace = false) override {
		auto ret = properties.insert(std::make_pair(key, val));
		if (!ret.second && replace) {
			ret.first->second = val;
			return true;
		}
		return ret.second;
	}
	
	/// @copydoc MGE::PropertySetInterface::setProperty(const std::string_view&, const MGE::Any&)
	bool setProperty(const std::string_view& key, const MGE::Any& val) override {
		auto iter = properties.find(key);
		if (iter != properties.end()) {
			iter->second = val;
			return true;
		}
		return false;
	}
	
	/**
	 * @brief remove all propeties
	 */
	void clearAll() {
		properties.clear();
	}
	
	
	/**
	 * @brief Load (restore) property set elements from XML
	 * 
	 * @param[in]  xmlNode       xml node, that will be using for load state of this object
	 * @param[in]  lang          language code for matching with @c lang attribute of \<G11n\> subnodes
	 * @param[in]  clear         when true clear set before load
	 *
	 * @note
	 *   - when not found \<G11n\> subnode with matching @c lang attribute → try use default \<G11n\> subnode without @c lang attribute
	 *   - when @a lang is empty or not found any matching nor default \<G11n\> subnodes → use direct content of \<Property\> node
	 * 
	 * @see
	 *   - description of @ref XMLNode_Property XML node
	 */
	void restoreFromXML(const pugi::xml_node& xmlNode, const std::string_view& lang = MGE::EMPTY_STRING_VIEW, bool clear = false);
	
	/**
	 * @brief Store property set elements to XML
	 * 
	 * @param[in,out]  xmlNode xml node to add subnodes with properties stored in this set
	 */
	void storeToXML(pugi::xml_node& xmlNode) const;
	
	/// operator for store in xml serialization archive
	friend FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const PropertySet& obj) {
		obj.storeToXML(xmlNode);
		return xmlNode;
	}
	
	/// operator for write to text output stream
	friend inline std::ostream& operator<<(std::ostream& s, const PropertySet& obj) {
		s << "MGE::PropertySet {";
		for (auto& it : obj.properties)
			s << it.first << "=>" << it.second << " ";
		return s << "}";
	}
	
	
	/// empty property set object
	static const PropertySet EMPTY;
	
	/// type of static function for register in @ref addType
	typedef const MGE::Any (*StringToTypeConverter)(const pugi::xml_node& valueNode);
	
	/**
	 * @brief register type in PropertySet
	 * 
	 * @param[in]  typeName     type name used in XML files
	 * @param[in]  callback     pointer to static function returned MGE::Any from XML node representing value of registred type
	 * @param[in]  types        vector of std::type_index that will be write as @a typeName in XML
	 */
	static void addType(const std::string& typeName, StringToTypeConverter callback, const std::vector<std::type_index>& types);
	
	/**
	 * @brief unregister type in PropertySet
	 * 
	 * @param[in]  typeName     type name used in XML files
	 */
	static void remType(const std::string_view& typeName);
	
	/**
	 * @brief return name of @a tInfo type or empty string when @a tInfo type is not registred in PropertySet
	 */
	static const std::string& typeToString(const std::type_info& tInfo);
	
protected:
	/// map of type's names
	static std::map<const std::type_index, std::string> typeToStringMap;
	
	/// map of static function doing conversion from named type (type name as string, value as XML node) to real value in 
	static std::map<const std::string, StringToTypeConverter, std::less<>> stringToAnyTypeMap;
	
	/// map of of properties using wrapped value types (MGE::Any) 
	std::map<std::string, MGE::Any, std::less<>> properties;
};

/// @}

}
