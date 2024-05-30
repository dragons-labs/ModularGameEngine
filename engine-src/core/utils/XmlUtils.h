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

#include "force_inline.h"
#include "StringUtils.h"
#include "Concepts.h"

#if !defined(LOG_WARNING) || !defined(LOG_DEBUG)
	#include "LogSystem.h"
#endif

#include <pugixml.hpp>

#include <stdint.h>
#include <list>
#include <map>

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/**
 * @brief Helper functions for XML support.
 * 
 * `#include <xmlUtils.h>`
 */
namespace XMLUtils {
	/**
	 * @brief Return value of xml node by doing conversion to @a T type.
	 * 
	 * @param[in] xmlNode   xml node to retrieve value
	 */
	template<typename T> T getValue(const pugi::xml_node& xmlNode);
	
	/**
	 * @brief Load list of values from xml node by doing conversion to @a ValueType type.
	 * 
	 * @param[in]  xmlNode  xml node for read value
	 * @param[out] list     list to put values from XML
	 */
	template<typename ValueType, typename ListType = std::list<ValueType>>
	inline void getListOfValues(const pugi::xml_node& xmlNode, ListType* list) {
		for (const auto& itemNode : xmlNode.children("item")) {
			list->push_back( MGE::XMLUtils::getValue<ValueType>(itemNode) );
		}
	}
	
	/**
	 * @brief Load map of values from xml node by doing conversion to @a ValueType and @a KeyType type.
	 * 
	 * @param[in]  xmlNode  xml node for read value
	 * @param[out] map      map to put values from XML
	 */
	template<typename KeyType, typename ValueType, typename MapType = std::map<KeyType, ValueType>>
	inline void getMapOfValues(const pugi::xml_node& xmlNode, MapType* map) {
		for (const auto& itemNode : xmlNode.children("item")) {
			map->insert( std::make_pair(
				MGE::XMLUtils::getValue<KeyType>(itemNode.child("key")),
				MGE::XMLUtils::getValue<ValueType>(itemNode.child("val"))
			) );
		}
	}
	
	/**
	 * @brief Return value of xml node by doing conversion to @a T type
	 *        when @a xmlNode is empty or conversion fail return @a defaultValue.
	 * 
	 * @param[in] xmlNode      xml node to retrieve value
	 * @param[in] defaultValue value to use when @a xmlElement is NULL or it's not possible to convert value of @a xmlElement to type @a T
	 */
	template<typename T> inline T getValue(const pugi::xml_node& xmlNode, const T& defaultValue) {
		if (xmlNode) {
			try {
				return getValue<T>(xmlNode);
			} catch(std::exception& e) {
				LOG_WARNING("Exception while processing xml to value: " << e.what());
			} catch(...) {
				LOG_WARNING("Exception while processing xml to value");
			}
		}
		return defaultValue;
	}
	
	/**
	 * @copybrief getValue(const pugi::xml_node&, const T&)
	 * Pointer (e.g. `const char*`) @a defaultValue variant.
	 * 
	 * @copydetails getValue(const pugi::xml_node&, const T&)
	 */
	template<typename T> FORCE_INLINE T* getValue(const pugi::xml_node& xmlNode, T* defaultValue) {
		return getValue<T*>(xmlNode, defaultValue);
	}
	
	/**
	 * @copybrief getValue(const pugi::xml_node&, const T&)
	 * Non-const rvalue class object reference @a defaultValue variant.
	 * 
	 * @copydetails getValue(const pugi::xml_node&, const T&)
	 */
	template<typename T> requires std::is_class<T>::value FORCE_INLINE T getValue(const pugi::xml_node& xmlNode, T&& defaultValue) {
		return getValue<T>(xmlNode, defaultValue);
	}
	
	/**
	 * @brief Return content of xml node (text and subnodes) as string.
	 * 
	 * @param xmlNode      xml node to retrieve content
	 * @param indent       string used to making indent (see pugi::xml_node::print)
	 * @param flags        formating flags (see pugi::xml_node::print)
	 */
	inline std::string nodeAsString(const pugi::xml_node& xmlNode, const char* indent = "", unsigned int flags = (pugi::format_raw | pugi::format_no_declaration)) {
		std::ostringstream xml_str;
		xmlNode.print(xml_str, indent, flags);
		return xml_str.str();
	}
	
	/**
	 * @brief Create / update / remove xml attribute.
	 * 
	 * @param xmlNode      Xml node owned attribute.
	 * @param attribName   Attribute name.
	 * @param newValue     New value to set. When empty (by string compare), then remove attribute.
	 * 
	 * @tparam ValType     Type of newValue. Must have compare operator with std::string_view.
	 * 
	 * @return True when attribute value was changed. False when new value is the same as old value (by string compare).
	 */
	template<typename ValType> bool updateXMLNodeAttrib(pugi::xml_node xmlNode, MGE::null_end_string attribName, ValType newValue) {
		auto xmlAttrib = xmlNode.attribute(attribName);
		std::string_view oldVal = xmlAttrib.as_string();
		
		if (newValue != oldVal) {
			LOG_DEBUG("change/set attribute " << attribName << " to " << newValue);
			
			// if newValue is not empty string → add or change attribute
			if (newValue != MGE::EMPTY_STRING_VIEW) {
				if (!xmlAttrib) {
					xmlAttrib = xmlNode.append_attribute(attribName);
				}
				xmlAttrib.set_value(newValue);
			// otherwise remove attribute
			} else if(xmlAttrib) {
				xmlNode.remove_attribute(xmlAttrib);
			}
			return true;
		}
		return false;
	}
	
	/**
	 * @brief Open XML file and return root node with error checking (do not throw, only write message to log).
	 * 
	 * @param[out] xmlDoc      xml document to use for opening file
	 * @param[in]  filePath    path to XML file to open
	 * @param[in]  nodeName    name of root node
	 * 
	 * @return Return XML node. Can return empty node.
	 */
	pugi::xml_node openXMLFile(pugi::xml_document& xmlDoc, MGE::null_end_string filePath, MGE::null_end_string nodeName = nullptr);
}

/// Empty xml node object (to use in reference return, etc)
extern const pugi::xml_node  EMPTY_XML_NODE;

/// @}
}

#ifndef __DOCUMENTATION_GENERATOR__
// specializations MGE::XMLUtils::getValue template for some standard types (conversion supported by pugixml)
template<> FORCE_INLINE bool      MGE::XMLUtils::getValue<bool>(const pugi::xml_node& xmlNode)       { return xmlNode.text().as_bool();   }
template<> FORCE_INLINE int32_t   MGE::XMLUtils::getValue<int32_t>(const pugi::xml_node& xmlNode)    { return xmlNode.text().as_int();    }
template<> FORCE_INLINE uint32_t  MGE::XMLUtils::getValue<uint32_t>(const pugi::xml_node& xmlNode)   { return xmlNode.text().as_uint();   }
template<> FORCE_INLINE int64_t   MGE::XMLUtils::getValue<int64_t>(const pugi::xml_node& xmlNode)    { return xmlNode.text().as_llong();  }
template<> FORCE_INLINE uint64_t  MGE::XMLUtils::getValue<uint64_t>(const pugi::xml_node& xmlNode)   { return xmlNode.text().as_ullong(); }
template<> FORCE_INLINE float     MGE::XMLUtils::getValue<float>(const pugi::xml_node& xmlNode)      { return xmlNode.text().as_float();  }
template<> FORCE_INLINE double    MGE::XMLUtils::getValue<double>(const pugi::xml_node& xmlNode)     { return xmlNode.text().as_double(); }
template<> FORCE_INLINE std::string          MGE::XMLUtils::getValue<std::string>(const pugi::xml_node& xmlNode)          { return xmlNode.text().as_string(); }
template<> FORCE_INLINE std::string_view     MGE::XMLUtils::getValue<std::string_view>(const pugi::xml_node& xmlNode)     { return xmlNode.text().as_string(); }
template<> FORCE_INLINE MGE::null_end_string MGE::XMLUtils::getValue<MGE::null_end_string>(const pugi::xml_node& xmlNode) { return xmlNode.text().as_string(); }

namespace pugi {
	// unqualified names are looked up when the template is defined, not when it's instantiated  ( https://clang.llvm.org/compatibility.html#dep_lookup )
	//
	// `operator<<(pugi::xml_node& xmlNode, ValueType val)` is used in some templates and we want looked up for them,
	// when they are instantiated (because some of operator<<(pugi::xml_node& xmlNode, ValueType val) ae define in other places)
	// 
	// so we put them in argument (pugi::xml_node) namespace
	
	#ifndef __PYTHON_DOCUMENTATION_GENERATOR__
	// XML stream write operators for standard types, value is stored as text content of current node  (compatible with the above getValue templates)
	template <MGE::Arithmetic ValueType> FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, ValueType val) { // for bool, int*_t, uint*_t, float, double
		xmlNode.text().set(val);
		return xmlNode;
	}
	FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, MGE::null_end_string val) {
		xmlNode.text().set(val);
		return xmlNode;
	}
	FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const std::string& val) {
		xmlNode.text().set(val.c_str());
		return xmlNode;
	}
	FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const std::string_view& val) {
		xmlNode.text().set(static_cast<std::string>(val).c_str());
		return xmlNode;
	}
	FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const MGE::x_string_view& val) {
		xmlNode.text().set(X_STRING_C_STR(val));
		return xmlNode;
	}
	
	// XML stream write operators for std::list and std::map  (compatible with getListOfValues / getMapOfValues)
	template<typename ValueType> requires MGE::ListType<ValueType> inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const ValueType& val) {
		for (auto& iter : val) {
			xmlNode.append_child("item") << iter;
		}
		return xmlNode;
	}
	template<typename ValueType> requires MGE::MapType<ValueType> inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const ValueType& val) {
		for (auto& iter : val) {
			auto xmlStoreNode = xmlNode.append_child("item");
			xmlStoreNode.append_child("key") << iter.first;
			xmlStoreNode.append_child("val") << iter.second;
		}
		return xmlNode;
	}
	#else // else for `ifndef __PYTHON_DOCUMENTATION_GENERATOR__` → this code will never be build into execution code
	template<typename ValueType> FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, ValueType val);
	#endif
	
	// processing rvalue reference as lvalue reference
	// needed for use as `xmlNode.append_child("something") << value`
	template<typename ValueType> FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node&& xmlNode, const ValueType&  val) { return operator<<(xmlNode, val); }
	template<typename ValueType> FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node&& xmlNode, const ValueType&& val) { return operator<<(xmlNode, val); }
	template<typename ValueType> FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node&  xmlNode, const ValueType&& val) { return operator<<(xmlNode, val); }
	
	
	#ifndef __PYTHON_DOCUMENTATION_GENERATOR__
	// write simple types as attrib value via strem << operator
	template <MGE::Arithmetic ValueType> FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute& xmlAtrib, ValueType val) { // for bool, int*_t, uint*_t, float, double
		xmlAtrib.set_value(val);
		return xmlAtrib;
	}
	FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute& xmlAtrib, MGE::null_end_string val) {
		xmlAtrib.set_value(val);
		return xmlAtrib;
	}
	FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute& xmlAtrib, const std::string val) {
		xmlAtrib.set_value(val.data(), val.length());
		return xmlAtrib;
	}
	FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute& xmlAtrib, const std::string_view val) { // MGE::x_string_view is derived from std::string_view
		xmlAtrib.set_value(val.data(), val.length());
		return xmlAtrib;
	}
	#else // else for `ifndef __PYTHON_DOCUMENTATION_GENERATOR__` → this code will never be build into execution code
	template<typename ValueType> FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute& xmlAtrib, ValueType val);
	#endif
	
	template<typename ValueType> FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute&& xmlAtrib, const ValueType& val)  { return operator<<(xmlAtrib, val); }
	template<typename ValueType> FORCE_INLINE pugi::xml_attribute& operator<<(pugi::xml_attribute&& xmlAtrib, const ValueType&& val) { return operator<<(xmlAtrib, val); }
}
#endif
