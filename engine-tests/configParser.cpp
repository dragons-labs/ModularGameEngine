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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ConfigParser
#include <boost/test/unit_test.hpp>

#include "ConfigParser.h"
#include "ModuleBase.h"
#include "LogSystem.h"

namespace MGE {
	Log* defaultLog = nullptr;
	
	struct Globals {
		Globals()   {
			defaultLog = new Log();
		}
		~Globals()  {
			delete defaultLog;
		}
	};
}

using namespace  MGE;

BOOST_GLOBAL_FIXTURE( Globals );


int load_count;
int x_check;
std::stringstream xml_str;

// default (external) registration (using global variable in dedicated namespace and non-member function)

struct Demo : MGE::Module {
	~Demo() override {}
};

// preferred "function-header" syntax

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(Demo) {
	LOG_INFO("", "Demo::load");
	x_check += xmlNode.child("x").text().as_int();
	++load_count;
	return new Demo();
}

// registration using static class members

struct Demo2 : MGE::Module {
	static MGE::Module* load(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
		LOG_INFO("", "Demo2::load");
		x_check += xmlNode.child("x").text().as_int();
		++load_count;
		
		xmlNode.print(xml_str, "", pugi::format_raw);
		LOG_INFO("", "Demo2 xml is: " << xml_str.str());
		return new Demo2();
	}
	~Demo2() override {}
private:
	static bool isRegistred;
};

MGE_REGISTER_MODULE(Demo, &Demo2::load, Demo2_AS_Demo);

// external registration can be applied also to class with member-base registration
MGE::Module* create__Demo3(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("", "Demo3::load");
	return new Demo2();
}
MGE_REGISTER_MODULE(Demo3);



std::string xml1(R"(
	<Test a="15">
		<Autostart>
			<Demo>
				<x>13</x>
			</Demo>
			<Demo2>
				<x>17</x>
			</Demo>
		</Autostart>
	</Test>
)");

BOOST_AUTO_TEST_CASE( listenersCall ) {
	BOOST_CHECK_EQUAL(ConfigParseListenerRegistration::isRegistred__Demo3, true);
	
	load_count = 0;
	x_check = 0;
	
	pugi::xml_document mainConfig;
	mainConfig.load_buffer(xml1.data(), xml1.size());
	
	MGE::ConfigParser::LoadedModulesSet loadedModulesSet;
	ConfigParser::getPtr()->createAndConfigureModules(loadedModulesSet, mainConfig.child("Test").child("Autostart"), nullptr, 0);
	
	BOOST_CHECK_EQUAL(load_count, 2);
	BOOST_CHECK_EQUAL(x_check, 2*13);
	BOOST_CHECK_EQUAL(xml_str.str(), "<Demo><x>13</x></Demo>");
	BOOST_CHECK_EQUAL(loadedModulesSet.size(), 2);
}

BOOST_AUTO_TEST_CASE( pugi_xml ) {
	pugi::xml_document mainConfig;
	mainConfig.load_buffer(xml1.data(), xml1.size());
	
	
	// empty node and attrib
	
	pugi::xml_node       xmlNode;
	pugi::xml_attribute  xmlAttrib;
	int a;
	
	if (xmlNode)
		a = 2;
	else
		a = 3;
	BOOST_CHECK_EQUAL(a, 3);
	
	
	// exist child node
	
	xmlNode = mainConfig.child("Test");
	xmlAttrib = xmlNode.attribute("a");
	BOOST_CHECK_EQUAL( xmlAttrib.as_int(1), 15);
	
	xmlAttrib = xmlNode.attribute("b");
	BOOST_CHECK_EQUAL( xmlAttrib.as_int(2), 2);
	
	if (xmlNode)
		a = 2;
	else
		a = 3;
	BOOST_CHECK_EQUAL(a, 2);
	
	
	// non-exist child node
	
	xmlNode = xmlNode.child("Abc");
	xmlAttrib = xmlNode.attribute("c");
	BOOST_CHECK_EQUAL( xmlAttrib.as_int(3), 3);
	
	BOOST_CHECK_EQUAL( mainConfig.child("Test").child("Abc").attribute("c").as_int(3), 3);
	
	if (xmlNode)
		a = 2;
	else
		a = 3;
	BOOST_CHECK_EQUAL(a, 3);
}
