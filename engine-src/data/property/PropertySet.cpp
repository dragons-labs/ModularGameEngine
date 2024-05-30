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

#include "data/property/PropertySet.h"
#include "data/property/XmlUtils_Ogre.h"
#include "pragma.h"

MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors")

const MGE::Any         MGE::Any::EMPTY;
const MGE::PropertySet MGE::PropertySet::EMPTY;

std::map<const std::type_index, std::string> MGE::PropertySet::typeToStringMap = {
	{typeid(int8_t),         "int"},
	{typeid(int16_t),        "int"},
	{typeid(int32_t),        "int"},
	{typeid(int64_t),        "int"},
	{typeid(uint8_t),        "int"},
	{typeid(uint16_t),       "int"},
	{typeid(uint32_t),       "int"},
	{typeid(uint64_t),       "int"},
	{typeid(float),          "float"},
	{typeid(double),         "double"},
	{typeid(std::string),    "String"},
	{typeid(Ogre::Vector2),  "Vector2"},
	{typeid(Ogre::Vector3),  "Vector3"},
	{typeid(std::list<std::string>),             "ListOfStr"},
	{typeid(std::map<std::string, std::string>), "Map_StrStr"},
	{typeid(std::map<std::string, int>),         "Map_StrInt"},
	{typeid(std::map<std::string, float>),       "Map_StrFloat"}
};

std::map<const std::string, MGE::PropertySet::StringToTypeConverter, std::less<>> MGE::PropertySet::stringToAnyTypeMap = {
	{"int",          MGE::Any::getAnyFromXML<int64_t>},
	{"float",        MGE::Any::getAnyFromXML<float>},
	{"double",       MGE::Any::getAnyFromXML<double>},
	{"String",       MGE::Any::getAnyFromXML<std::string>},
	
	{"Vector2",      MGE::Any::getAnyFromXML<Ogre::Vector2>},
	{"Vector3",      MGE::Any::getAnyFromXML<Ogre::Vector3>},
	
	{"ListOfStr",    MGE::Any::getAnyListFromXML<std::string>},
	{"Map_StrStr",   MGE::Any::getAnyMapFromXML<std::string, std::string>},
	{"Map_StrInt",   MGE::Any::getAnyMapFromXML<std::string, int>},
	{"Map_StrFloat", MGE::Any::getAnyMapFromXML<std::string, float>},
};

MGE_CLANG_WARNING_POP


/**
@page XMLSyntax_Property PropertySet XML syntax
@tableofcontents

@section PropertySet PropertySet

PropertySet is dictionary-like structure, that can store values of different types.
Each value is identified by a unique name. Values are stored as MGE::Any.

PropertySet can be created/restored from XML config/save file.
Restoring function (MGE::PropertySet::load) gets the next @b \<Property\> xml nodes, that are sub nodes of xml node passed to it.

@subsection XMLNode_Property \<Property\>

\<Property\> element node value is used to get value of property. Element has attributes:
  - @b name      specifies name of property (key value in dictionary)
  - @b type      determined type of stored value, "out of the box" supported types are:
    - "int"
      - interpret value as integer number
        - text content of <i>value node</i> is the string converted to a number
      - use int64_t to store it
    - "float"
      - interpret value as floating point number
        - text content of <i>value node</i> is the string converted to a number
      - use float to store it
    - "String"
      - store value as string
        - text content of <i>value node</i> is the string
      - use std::string to store it
    - "Vector2"
      - interpret value as 2 elements vector of floating point number
        - <i>value node</i> node must have 2 sub nodes: \<x\> and \<y\> or \<z\> with values of corresponding elements
      - use Ogre::Vector2 to store it
    - "Vector3"
      - interpret value as 3 elements vector of floating point number
        - <i>value node</i> node must have 3 sub nodes: \<x\>, \<y\> and \<z\> with values of corresponding elements
      - use Ogre::Vector2 to store it
    - "ListOfStr"
      - interpret value as list of strings
        - \<Property\> node must have sets of \<item\> nodes with strings values
      - use std::list\<std::string\> to store it
    - "Map_StrStr"
      - interpret value as map with string keys and string values
        - \<Property\> node must have sets of \<item\> nodes with \<key\> and \<val\> sub nodes
      - use std::map\<std::string, std::string> to store it
    - "Map_StrInt"
      - interpret value as map with string keys and floating point values
        - \<Property\> node must have sets of \<item\> nodes with \<key\> and \<val\> sub nodes
      - use std::map\<std::string, float\> to store it
    - "Map_StrFloat"
      - interpret value as map with string keys and floating point values
        - \<Property\> node must have sets of \<item\> nodes with \<key\> and \<val\> sub nodes
      - use std::map\<std::string, float\> to store it
    - "PropertySet"
      - interpret value as property set
        - \<Property\> node must have sets of correct \<Property> nodes
      - use MGE::PropertySet to store it
    - addional types can be registred via @ref MGE::PropertySet::addType.
  - @b isList    when exist and set to "1", "yes" or "true"
    - interpret value of \<Property\> as list of elements with type determined by @em type attribute
      - \<Property\> node must have sets of \<item\> nodes converted to type determined by @em type attribute
    - use std::list\<MGE::Any\> to store it

Depending on property type <i>Value node</i> is:
  - \<item\> child of \<Property\> node for list based properties (e.g. @a ListOfStr or with @a isList=true)
    - see @ref MGE::Any::getAnyListFromXML (→ @ref MGE::XMLUtils::getListOfValues)
      and `operator<<(pugi::xml_node& xmlNode, const ValueType& value)` template specification
      for implementation of restore / store for list based properties.
  - \<key\> and \<val\> childs of \<item\> child of \<Property\> node for map based properties (e.g. @a Map_StrStr)
    - see @ref MGE::Any::getAnyMapFromXML (→ @ref MGE::XMLUtils::getMapOfValues)
      and `operator<<(pugi::xml_node& xmlNode, const ValueType& value)` template specification
      for implementation of restore / store for list based properties.
  - \<value\> child of \<Property\> node in other cases.

Content of any type of \<Property\> node can be covered in \<G11n\> subnodes for multi languages support.
Loading procedure try loading \<Property\> from \<G11n\> subnode or \<Property\> itself in the following order:
  -# \<G11n\> subnode with matching @c lang attribute
  -# (when not found) \<G11n\> subnode without any @c lang attribute
  -# (when not found) \<Property\> node itself

@subsection Example Example
@code{.xml}
<SomeNode>
  <Property name="integer number" type="int">12</Property>
  <Property name="2d vector" type="Vector2">
    <value><x>13.4</x><y>17.6</y></value>
  </Property>
  <Property name="list of strings" type="ListOfStr">
    <item>element 1</item>
    <item>element 2</item>
  </Property>
  <Property name="list of strings 2" type="ListOfStr">
    <G11n>
      <item>element 1 DEFAULT</item>
      <item>element 2 DEFAULT</item>
    </G11n>
    <G11n lang="en">
      <item>element 1 EN</item>
      <item>element 2 EN</item>
    </G11n>
  </Property>
  <Property name="map of string to integer number" type="Map_StrInt">
    <item><key>a</key><val>1</val></item>
    <item><key>b</key><val>2</val></item>
  </Property>
  <Property name="sub property set" type="PropertySet">
    <Property name="aa" type="int">15</Property>
    <Property name="bb" type="float">16.3</Property>
  </Property>
  <Property name="list of vectors" type="Vector3" isList="true">
    <item><x>1.1</x><y>2.1</y><z>3.1</z></item>
    <item><x>13.1</x><y>12.1</y><z>-5.1</z></item>
  </Property>
</SomeNode>
@endcode
*/
void MGE::PropertySet::restoreFromXML(const pugi::xml_node& xmlNode, const std::string_view& lang, bool clear) {
	if (clear)
		properties.clear();
	
	for (auto propNode : xmlNode.children("Property")) {
		std::string      propName  = propNode.attribute("name").as_string();
		std::string_view propType  = propNode.attribute("type").as_string();
		bool             isList    = propNode.attribute("isList").as_bool(false);
		
		auto convIter = stringToAnyTypeMap.find(propType);
		if (convIter == stringToAnyTypeMap.end()) {
			LOG_WARNING("Unknown type " + propType + " for value while parsing properties from XML");
			continue;
		}
		StringToTypeConverter valueConverter = convIter->second;
		
		// by default read property from <Property> node itself
		pugi::xml_node* propertyContentNode = &propNode;
		
		// try find matching or default <G11n> subtag to read property
		if (!lang.empty()) {
			for (auto xmlSubNode : propertyContentNode->children("G11n")) {
				std::string_view nodeLang = xmlSubNode.attribute("lang").as_string();
				if (nodeLang.empty()) {
					// remember <G11n> subtag without lang attribute, but continue searching for matching
					propertyContentNode = &xmlSubNode;
				} else if (nodeLang == lang) {
					// use <G11n> subtag without matching lang attribute
					propertyContentNode = &xmlSubNode;
					break;
				}
			}
		}
		
		if (isList) {
			std::list<MGE::Any> tmpList;
			auto iter = properties.insert(std::make_pair(propName, MGE::Any(tmpList))).first;
			std::list<MGE::Any>* listPtr = iter->second.getValuePtr<std::list<MGE::Any>>();
			
			for (auto propItemNode : propertyContentNode->children("item")) {
				listPtr->push_back(valueConverter(propItemNode));
			}
		} else if (propType == "PropertySet") {
			PropertySet tmpSet;
			auto iter = properties.insert(std::make_pair(propName, MGE::Any(tmpSet))).first;
			PropertySet* setPtr = iter->second.getValuePtr<PropertySet>();
			
			setPtr->restoreFromXML( *propertyContentNode, lang, false );
		} else {
			auto xmlSubNode = propertyContentNode->child("value");
			if (xmlSubNode) {
				addProperty(propName, valueConverter(xmlSubNode), true);
			} else if (propertyContentNode->child("item")) {
				addProperty(propName, valueConverter(*propertyContentNode), true);
			} else {
				LOG_WARNING("Property without <value> / <item> subnode at byte " << propertyContentNode->offset_debug());
			}
		}
	}
}

void MGE::PropertySet::addType(const std::string& typeName, StringToTypeConverter callback, const std::vector<std::type_index>& types) {
	stringToAnyTypeMap[typeName] = callback;
	for (auto& iter : types) {
		typeToStringMap[iter] = typeName;
	}
}

void MGE::PropertySet::remType(const std::string_view& typeName) {
	#if __cplusplus > 202002L /* C++23 */
	stringToAnyTypeMap.erase(typeName);
	#else
	for (auto iter = stringToAnyTypeMap.find(typeName); iter != stringToAnyTypeMap.end(); iter = stringToAnyTypeMap.find(typeName)) { stringToAnyTypeMap.erase(iter); }
	#endif
	
	auto iter = typeToStringMap.begin();
	while (iter!=typeToStringMap.end()) {
		if (iter->second == typeName)
			typeToStringMap.erase(iter++);
		else
			++iter;
	}
}

void MGE::PropertySet::storeToXML(pugi::xml_node& xmlNode) const {
	for (auto& iter : properties) {
		auto xmlStoreNode = xmlNode.append_child("Property");
		xmlStoreNode.append_attribute("name") = iter.first.c_str();
		xmlStoreNode.append_attribute("type") = typeToString(iter.second.getType()).c_str();
		iter.second.storeToXML(xmlStoreNode);
	}
}

const std::string& MGE::PropertySet::typeToString(const std::type_info& tInfo) {
	auto iter = typeToStringMap.find(tInfo);
	if (iter != typeToStringMap.end()) {
		return iter->second;
	} else {
		return MGE::EMPTY_STRING;
	}
}
