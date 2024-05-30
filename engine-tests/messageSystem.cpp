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

#include "MessagesSystem.h"
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


BOOST_AUTO_TEST_CASE( MessagesSystem_Python ) {
	scriptsSystem->getGlobalsDict()["MGE"] = pybind11::module::import("MGE");
	
	auto m = new MGE::MessagesSystem();
	std::cout << "\nm="<<m<<"\n";
	scriptsSystem->getGlobalsDict()["mm"] = m;
	
	scriptsSystem->runString(R"(
def capsule2int(capsule):
	import ctypes
	ctypes.pythonapi.PyCapsule_GetPointer.restype = ctypes.c_void_p
	ctypes.pythonapi.PyCapsule_GetPointer.argtypes = [ctypes.py_object, ctypes.c_char_p]
	return ctypes.pythonapi.PyCapsule_GetPointer(capsule, None)
	)");
	
	scriptsSystem->runString("odb1_count, odb2_count, odb1_check, odb2_check = 0, 0, 0, 0");
	
	scriptsSystem->runString(R"(
def odbiorca(a, b):
	global odb1_count, odb1_check
	print("odbiorca", a, capsule2int(b))
	odb1_count += 1
	if capsule2int(b) == id(odbiorca):          # we put `odbiorca` as 3th arg of registerReceiver, so we expect call with `odbiorca` as 2nd arg
		odb1_check += 1
	
mm.registerReceiver("test message", "odbiorca", odbiorca, None, None)
	)");
	
	scriptsSystem->runString(R"(
x = MGE.EventMsg("test message")
mm.sendMessage(x)

def odbiorca2(a, b):
	global odb2_count, odb2_check
	print("odbiorca2", a, capsule2int(b))
	odb2_count += 1
	if capsule2int(b) == id(odbiorca2):          # we put `odbiorca2` as 3th arg of registerReceiver, so we expect call with `odbiorca2` as 2nd arg
		odb2_check += 1

mm.registerReceiver("test message", "odbiorca2", odbiorca2, None, None)

mm.sendMessage(x, id(mm))

y = MGE.EventMsg("test message X")
mm.sendMessage(y)
	)");
	int odb1_count = scriptsSystem->getGlobalsDict()["odb1_count"].cast<int>();
	int odb2_count = scriptsSystem->getGlobalsDict()["odb2_count"].cast<int>();
	int odb1_check = scriptsSystem->getGlobalsDict()["odb1_check"].cast<int>();
	int odb2_check = scriptsSystem->getGlobalsDict()["odb2_check"].cast<int>();
	
	BOOST_CHECK_EQUAL(odb1_count, 2);
	BOOST_CHECK_EQUAL(odb2_count, 1);
	BOOST_CHECK_EQUAL(odb1_check, 2);
	BOOST_CHECK_EQUAL(odb2_check, 1);
}
