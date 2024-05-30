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

#include "data/property/G11n.h"

#include "LogSystem.h"
#include "XmlUtils.h"
#include "ConfigParser.h"

///////////////////  create and init  ///////////////////

MGE::G11n::G11n(const std::string_view& language, MGE::null_end_string translationFile) {
	LOG_INFO("Initialising G11n subsystem: lang=" << lang << "translationFile=" << translationFile);
	
	lang = language;
	
	if (translationFile && translationFile[0] != '\0') {
		pugi::xml_document xmlDoc;
		translation.restoreFromXML( MGE::XMLUtils::openXMLFile(xmlDoc, translationFile, "Translations"), lang, true );
	}
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_G11nConfig \<G11n\>

@c \<G11n\> node for <b>language/translation settings</b> next child nodes:
  - @c \<Language\> with ISO-639 language code
  - @c \<TranslationFile\> with path to xml config file with string translation for @ref getLocaleString function
*/
MGE::G11n::G11n(const pugi::xml_node& xmlNode) :
	G11n(
		xmlNode.child("Language").text().as_string(),
		xmlNode.child("TranslationFile").text().as_string()
	)
{}

MGE::G11n::~G11n() {
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(G11n) {
	return new MGE::G11n(xmlNode);
}


///////////////////  get lang code and translations  ///////////////////

const std::string& MGE::G11n::getLang() {
	if (!getPtr())
		return MGE::EMPTY_STRING;
	
	return getPtr()->lang;
}

MGE::null_end_string MGE::G11n::getLocaleStringFromXML(
	const pugi::xml_node& xmlNode, MGE::null_end_string subNodesName, MGE::null_end_string defVal
) {
	if (!getPtr())
		return defVal;
	
	for (auto xmlSubNode : xmlNode.children(subNodesName)) {
		auto xmlAttrib = xmlSubNode.attribute("lang");
		if (xmlAttrib) {
			if (xmlAttrib.as_string() == getPtr()->lang) {
				return xmlSubNode.text().as_string();
			}
		} else {
			defVal = xmlSubNode.text().as_string();
		}
	}
	return defVal;
}

MGE::x_string_view MGE::G11n::getLocaleString(const std::string_view& str) {
	if (!getPtr())
		return str;
	
	const MGE::Any& any = getPtr()->translation.getProperty(str);
	if (any.isEmpty()) {
		LOG_WARNING("Not found translate for: " + str);
		return str;
	} else {
		return *(any.getValuePtr<std::string>());
	}
}
