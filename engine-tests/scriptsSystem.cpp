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
#define BOOST_TEST_MODULE ScriptsSystem
#include <boost/test/unit_test.hpp>

#include "ScriptsSystem.h"
#include "LogSystem.h"

namespace MGE {
	Log* defaultLog = nullptr;
	ScriptsSystem* scriptsSystem = nullptr;
	
	struct Globals {
		Globals()   {
			defaultLog = new Log();
			scriptsSystem = new ScriptsSystem();
		}
		~Globals()  {
			delete scriptsSystem;
			delete defaultLog;
		}
	};
}

using namespace  MGE;

BOOST_GLOBAL_FIXTURE( Globals );


BOOST_AUTO_TEST_CASE( interpreter_simple ) {
	scriptsSystem->runString("x=13", Py_single_input);
	
	auto ret = scriptsSystem->runString("2+x", Py_eval_input, scriptsSystem->getGlobals(), scriptsSystem->getGlobals());
	if (ret.ptr())
		BOOST_CHECK_EQUAL(ret.cast<int>(), 15);
	else
		BOOST_CHECK_MESSAGE(false, "Unexpected error in python, returned null object.");
	
	ret = scriptsSystem->runString("0/0");
	BOOST_CHECK_MESSAGE(ret.ptr() == nullptr, "Should be null object. Divided by zero is error in python.");
	
	// pass args via local
	auto locals = pybind11::dict(pybind11::arg("x")=17, pybind11::arg("y")=13);
	ret = scriptsSystem->runString("x-y", Py_eval_input, locals.ptr());
	if (ret.ptr())
		BOOST_CHECK_EQUAL(ret.cast<int>(), 4);
	else
		BOOST_CHECK_MESSAGE(false, "Unexpected error in python, returned null object.");
	
	// locals shouldn't override global value
	ret = scriptsSystem->runString("x", Py_eval_input);
	if (ret.ptr())
		BOOST_CHECK_EQUAL(ret.cast<int>(), 13);
	else
		BOOST_CHECK_MESSAGE(false, "Unexpected error in python, returned null object.");
}

BOOST_AUTO_TEST_CASE( global_dict ) {
	BOOST_CHECK_EQUAL(scriptsSystem->getGlobalsDict()["x"].cast<int>(), 13);
	
	int ok = -1;
	try {
		auto u = scriptsSystem->getGlobalsDict()["missing_variable"];
		if (u.ptr())
			std::cout << "Success\n" << "\n";
		else
			std::cout << "Error\n";
		ok = 1;
	} catch(pybind11::error_already_set& e) {
		ok = 0;
	}
	std::cout << "AAA " << ok << "\n";
	BOOST_CHECK_MESSAGE(ok == 0, "Missing key vaule exception on getGlobalsDict with not existing key");
}

std::string script_stdout;
void get_script_stdout(const std::string& txt, void* ar) {
	BOOST_CHECK_MESSAGE(ar == (void*)1357, "Error in pass extra argument to ScriptOutputListener");
	script_stdout.append(txt);
}
BOOST_AUTO_TEST_CASE( script_output ) {
	scriptsSystem->setScriptOutputListener(
		"", std::bind(get_script_stdout, std::placeholders::_1, (void*)1357)
	);
	scriptsSystem->runString(R"(print('ABC 123  xyz\n→', end='\n'))");
	scriptsSystem->runString("print('---')");
	scriptsSystem->setScriptOutputListener("", nullptr);
	BOOST_CHECK_EQUAL(script_stdout, "ABC 123  xyz\n→\n---\n");
}

const char* python_swig_output = R"(1
Vector2(13, 17)
showV1: Vector2(13, 17)
showV1: Vector2(99, 17)
Vector2(99, 17)
1
Vector2(3, 15)
showV2: Vector2(3, 15)
showV2: Vector2(99, 15)
Vector2(99, 15)
1
Vector2(99, 15)
showV2: Vector2(10, 21)
showV2: Vector2(99, 15)
Vector2(99, 15)
Vector2(208, 53)
showV2: Vector2(208, 53)
Radian(5.32687)
Radian(5.30718)
showV1: Vector2(99, 17)
)";
BOOST_AUTO_TEST_CASE( python_swig ) {
	script_stdout = "";
	scriptsSystem->setScriptOutputListener(
		"", std::bind(get_script_stdout, std::placeholders::_1, (void*)1357)
	);
	
	auto ret = scriptsSystem->runFile("python_swig.py");
	BOOST_CHECK_MESSAGE(ret.ptr() != nullptr, "Error in execute python_swig.py.");
	
	scriptsSystem->setScriptOutputListener("", nullptr);
	BOOST_CHECK_EQUAL(script_stdout, python_swig_output);
}

BOOST_AUTO_TEST_CASE( runObject ) {
	auto ret = scriptsSystem->runFile("python1.py");
	BOOST_CHECK_MESSAGE(ret.ptr() != nullptr, "Error in execute python code from files.");
	
	// calls from class name (static and class methods)
	auto w = scriptsSystem->runObject("xyz.EngineDemos.getA", 3);
	BOOST_CHECK_MESSAGE(w, "Error in xyz.EngineDemos.getA(3)");
	BOOST_CHECK_EQUAL(w.cast<int>(), 13*3);
	
	w = scriptsSystem->runObject("xyz.EngineDemos.getB", 7);
	BOOST_CHECK_MESSAGE(w, "Error in xyz.EngineDemos.getB(7)");
	BOOST_CHECK_EQUAL(w.cast<int>(), 13*5+7);
	
	w = scriptsSystem->runObject("xyz.EngineDemos.getC", 2);
	BOOST_CHECK_MESSAGE(!w, "No error in xyz.EngineDemos.getC(2)");
	
	auto ww = scriptsSystem->runObjectWithCast("xyz.EngineDemos.getC", 9999, 3);
	BOOST_CHECK_EQUAL(ww, 9999);
	
	// calls from object
	w = scriptsSystem->runObject("ed.getA", 3);
	BOOST_CHECK_MESSAGE(w, "Error in ed.getA(3)");
	BOOST_CHECK_EQUAL(w.cast<int>(), 13*3);
	
	w = scriptsSystem->runObject("ed.getB", 1);
	BOOST_CHECK_MESSAGE(w, "Error in ed.getB(1)");
	BOOST_CHECK_EQUAL(w.cast<int>(), 13*5+1);
	
	w = scriptsSystem->runObject("ed.getC", 2);
	BOOST_CHECK_MESSAGE(w, "Error in ed.getC(2)");
	BOOST_CHECK_EQUAL(w.cast<int>(), 17-2);
}
