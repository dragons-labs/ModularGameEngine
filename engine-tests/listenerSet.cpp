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
#define BOOST_TEST_MODULE ListenerSet
#include <boost/test/unit_test.hpp>

#include "ListenerSet.h"

int a, b, c;
void* d;
bool z;

//////////////////////////////////////////////////////////////////////

typedef bool (*CmdDelegate)(int arg);

bool function_1(int x) {
	a = x;
	c = 1;
	return false;
}

bool function_2(int x) {
	b = 2 * x;
	c = 2;
	return true;
}

bool function_3(int x) {
	c = 3;
	return true;
}

BOOST_AUTO_TEST_CASE( function_listener ) {
	const int call_value = 17;
	a = b = c = 0;
	
	MGE::FunctionListenerSet<CmdDelegate> myListener;
	myListener.addListener(function_1, 100);
	myListener.addListener(function_2, 120);
	myListener.addListener(function_3, 111);
	myListener.callAll(call_value);
	
	BOOST_CHECK_EQUAL(a, call_value);
	BOOST_CHECK_EQUAL(b, 2 * call_value);
	BOOST_CHECK_EQUAL(c, 2); // max key value => call last, so final value for c should be set by function_2
	
	z = myListener.callFirst(call_value);
	
	BOOST_CHECK_EQUAL(c, 3); // min key value => call first, so final value for c should be set by function_3 (because function_1 return false)
	BOOST_CHECK_EQUAL(z, true); // some function return true
}

//////////////////////////////////////////////////////////////////////

struct ListenerClass {
	bool callA(int x) {
		a = x;
		return true;
	}
	bool callB() {
		d = (void*)this;
		return true;
	}
};

BOOST_AUTO_TEST_CASE( class_listener ) {
	const int call_value = 13;
	a = b = c = 0;
	
	MGE::ClassListenerSet<ListenerClass> myListener;
	auto l1 = new ListenerClass();
	auto l2 = new ListenerClass();
	auto l3 = new ListenerClass();
	myListener.addListener(l1, 200);
	myListener.addListener(l2, 70);
	myListener.addListener(l3, 111);
	myListener.callAll(&ListenerClass::callA, call_value);
	myListener.callAll(&ListenerClass::callB);
	
	BOOST_CHECK_EQUAL(a, call_value);
	BOOST_CHECK_EQUAL(d, l1); // max key value => call last, so final value for d should be set by l1 listener
}

//////////////////////////////////////////////////////////////////////

typedef MGE::FunctorListenerClassBase<bool, int> ListenerClass2;

bool function_4(int x, int y) {
	b = x + y;
	c = 4;
	return false;
}

BOOST_AUTO_TEST_CASE( functor_listener ) {
	const int static_value = 12;
	const int call_value = 5;
	a = b = c = 0;
	
	MGE::ClassListenerSet<ListenerClass2, ListenerClass2> myListener;
	myListener.addListener(ListenerClass2(std::bind(function_4, std::placeholders::_1, static_value), 4321), 20);
	myListener.addListener(ListenerClass2(function_1, (uintptr_t)&function_1), 10);
	z = myListener.addListener(ListenerClass2(std::bind(function_4, std::placeholders::_1, 0), 4321), 20); // invalid registration, because the same value of FunctorListenerClassBase::id
	
	BOOST_CHECK_EQUAL(z, false);
	
	myListener.callAll(&ListenerClass2::call, call_value);
	
	BOOST_CHECK_EQUAL(a, call_value); // function_1 call ok
	BOOST_CHECK_EQUAL(b, static_value + call_value); // function_4 call ok
	
	BOOST_CHECK_EQUAL(c, 4); // function_4 call  as last
	
	z = myListener.callFirst(&ListenerClass2::call, call_value);
	BOOST_CHECK_EQUAL(c, 4); // function_4 call  as last
	BOOST_CHECK_EQUAL(z, false); // all call function returned false
}
