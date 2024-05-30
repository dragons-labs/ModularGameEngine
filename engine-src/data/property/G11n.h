/*
Copyright (c) 2022-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "BaseClasses.h"
#include "EngineModule.h"
#include "data/property/PropertySet.h"

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

/**
 * @brief g11n subsystem (multi languages support).
 * 
 * @note Safe to use even when not initialised/created.
 */
class G11n :
	public MGE::Singleton<G11n>,
	public MGE::Module
{
public:
	/**
	 * @brief Return string with selected language ISO-639 code.
	 */
	static const std::string& getLang();
	
	/**
	 * @brief Find XML node with current language and return it content.
	 *
	 * @param xmlNode      Xml node with set of @a subNodeName named subnodes.
	 * @param subNodesName Name of @a xmlNode subnodes with language dependents string,
	 *                     each this node should have @c lang attribute, which will be compared with value returned by @ref getLang,
	 *                     subnode without @c lang attribute will be use as default (when can't find subnode with correct @c lang attribute).
	 * @param defVal       String to return when can't find matching subnode of @a xmlNode (and no default subnode).
	 *
	 * @return Return (pointer to) string content (value) of first matching @a subNodeName named subnode of @a xmlNode.
	 *         Matching is doing by compare subnode @c lang attribute, with current ISO-639 language code returned by @ref getLang
	 *         When can't found matching subnode - use value of subnode without @c lang attribute.
	 *         When not found subnode without @c lang attribute - use @a defVal.
	 * 
	 * @note   Return pointer to internal data of @a xmlNode (or @a defVal).
	 *         Returned value is valid until @a xmlNode (or @a defVal) exists.
	 *         If need longer life of returned value (e.g. arguments are temporary / rvalue) result should be converted to std::string via:
	 *         `std::string x(getLocaleStringFromXML(...));` or via: `x = static_cast<std::string>(getLocaleStringFromXML(...))`, etc.
	 *         Before the end of evaluating the full-expression.
	 */
	static MGE::null_end_string getLocaleStringFromXML(
		const pugi::xml_node&  xmlNode,
		MGE::null_end_string   subNodesName,
		MGE::null_end_string   defVal = MGE::EMPTY_STRING_VIEW.data()
	);
	
	/**
	 * @brief Return translated text based on G11n module configuration.
	 * 
	 * @param str      Text to translate.
	 * 
	 * @return Return (reference/view to) translated text based on G11n module configuration
	 *         or (reference/view to) @a str when translation not found.
	 *         Function getLocaleString do not provide copy of string.
	 */
	static MGE::x_string_view getLocaleString(const std::string_view& str);
	
	/**
	 * @brief Constructor.
	 * 
	 * @param language         Language ISO-639 code to set.
	 * @param translationFile  XML file with string translation for @ref getLocaleString function.
	 */
	G11n(const std::string_view& language, MGE::null_end_string translationFile);
	
	/**
	 * @brief Constructor.
	 * 
	 * @param xmlNode  XML configuration node (@ref XMLNode_G11nConfig root node with info about language and translation file path in child nodes).
	 */
	G11n(const pugi::xml_node& xmlNode);
	
	/// destructor
	~G11n();
	
protected:
	/// Do NOT use getPtr() on this class, call static method instead.
	using MGE::Singleton<G11n>::getPtr;
	
	/// string with selected language ISO-639 code
	std::string lang;
	
	/// map of translated strings for @ref getLocaleString
	MGE::PropertySet translation;
};

/// @}

}
