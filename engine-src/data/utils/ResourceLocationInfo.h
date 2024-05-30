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

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup OgreWorldUtils
/// @{
/// @file

/**
 * @brief Struct to store location in Ogre resources system (e.g. object configuration location for store/restore system).
 */
struct ResourceLocationInfo {
	/// in-file name (e.g. name of XML node)
	std::string name;
	/// file name in Ogre resources system
	std::string fileName;
	/// group name in Ogre resources system
	std::string fileGroup;
	
	inline void set(const std::string& _name, const std::string& _fileName, const std::string& _fileGroup) {
		name      = _name;
		fileName  = _fileName;
		fileGroup = _fileGroup;
	}
	
	inline void restoreFromXML(const pugi::xml_node& xmlNode) {
		set(
			xmlNode.attribute("name").as_string(),
			xmlNode.attribute("file").as_string(),
			xmlNode.attribute("group").as_string()
		);
	}
	
	inline void storeToXML(pugi::xml_node& xmlNode) const {
		xmlNode.append_attribute("name")  << name;
		xmlNode.append_attribute("file")  << fileName;
		xmlNode.append_attribute("group") << fileGroup;
	}
	
	inline void storeToXML(pugi::xml_node&& xmlNode) const {
		storeToXML(xmlNode);
	}
	
	friend pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const ResourceLocationInfo& val) {
		val.storeToXML(xmlNode);
		return xmlNode;
	}
	
	friend pugi::xml_node& operator<<(pugi::xml_node&& xmlNode, const ResourceLocationInfo& val) {
		val.storeToXML(xmlNode);
		return xmlNode;
	}
	
	ResourceLocationInfo( const std::string& _name, const std::string& _fileName, const std::string& _fileGroup ) {
		set(_name, _fileName, _fileGroup);
	}
	
	ResourceLocationInfo(const pugi::xml_node& xmlNode) {
		restoreFromXML(xmlNode);
	}
};

/// @}

}
