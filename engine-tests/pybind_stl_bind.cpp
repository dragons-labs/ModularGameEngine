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


#include <pybind11/embed.h>
#include "pybind11_stl_bind.inl"

#include <numeric>
#include <string>
#include <list>

PYBIND11_MAKE_OPAQUE(std::list<int>);


std::list<int> list = {1, 5, 7};

std::list<int>& get() {
	return list;
}

int set_checksum = 0;
void set(std::list<int>& ll) {
	std::cout << "set\n";
	int i = 0;
	for (auto x : ll) {
		set_checksum += x * (++i);
		std::cout << x << " " << set_checksum << "\n";
	}
}

struct A {
	std::list<int> list;
	A() {
		std::cout << "constructor A()\n";
		list.push_back(1);
		list.push_back(2);
		list.push_back(4);
		list.push_back(8);
	}
	const std::list<int>& get() {
		return list;
	}
	static A* getPtr() {
		static A* a = new A();
		return a;
	}
};


PYBIND11_EMBEDDED_MODULE(demo, m) {
	MGE::py_bind_list<std::list<int>>(m, "IntList");
	pybind11::implicitly_convertible<pybind11::list, std::list<int>>();
	
	pybind11::class_<A>(m, "A")
		.def_readonly("list", &A::list)
		.def("get", &A::get)
		.def_static("getPtr", &A::getPtr, pybind11::return_value_policy::reference)
	;
		
	m.def("get1", &get, pybind11::return_value_policy::reference);
	m.def("get2", &get);
	m.attr("list") = &list;
	m.def("set", &set);
} 

BOOST_AUTO_TEST_CASE( stl_bind ) {
	// pybind11::scoped_interpreter guard{};
	pybind11::initialize_interpreter();
	pybind11::exec("import demo");
	
	std::cout << "iteration, push_back\n";
	pybind11::exec(R"(
		l = demo.get1()
		print("l:", l, len(l), type(l))
		s = 0
		for x in l:
			print(x)
			s += x
		l.push_back(s)
	)");
	
	BOOST_CHECK_EQUAL(list.back(), 13);
	BOOST_CHECK_EQUAL(list.size(), 4);
	
	std::cout << "modification without pybind11::return_value_policy\n";
	pybind11::exec(R"(
		l = demo.get2()
		print("l:", l, len(l), type(l))
		l.pop_back()
	)");
	
	BOOST_CHECK_EQUAL(list.back(), 7);
	BOOST_CHECK_EQUAL(list.size(), 3);
	
	std::cout << "insert\n";
	pybind11::exec(R"(
		l = demo.get1()
		print("l:", l, len(l), type(l))
		l.insert(1, 11)
		for x in l:
			print(x)
	)");
	
	for (auto x: list) std::cout << "c++ " << x << "\n";
	
	BOOST_CHECK_EQUAL(std::accumulate(list.begin(), list.end(), 0), 24);
	BOOST_CHECK_EQUAL(list.size(), 4);
	
	std::cout << "erase, access via attribute\n";
	pybind11::exec(R"(
		l = demo.list
		print("l:", l, len(l), type(l))
		l.erase(2)
		for x in l:
			print(x)
	)");
	
	for (auto x: list) std::cout << "c++ " << x << "\n";
	
	BOOST_CHECK_EQUAL(std::accumulate(list.begin(), list.end(), 0), 19);
	BOOST_CHECK_EQUAL(list.size(), 3);
	
	std::cout << "del[]\n";
	pybind11::exec(R"(
		l = demo.get1()
		print("l:", l, len(l), type(l))
		del l[1]
		for x in l:
			print(x)
	)");
	
	for (auto x: list) std::cout << "c++ " << x << "\n";
	
	BOOST_CHECK_EQUAL(std::accumulate(list.begin(), list.end(), 0), 8);
	BOOST_CHECK_EQUAL(list.size(), 2);
	
	std::cout << "in and find\n";
	pybind11::exec(R"(
		l = demo.get1()
		print("l:", l, len(l), type(l))
		if 7 in l: # true
			print("7 in l")
			l.append(7)
		if 8 in l: # false
			print("8 in l")
			l.append("100")
		if l.find(7) == 1: # true
			print("l.find(7) == 1")
			l.append(1)
		if l.find(8) == -1: # true
			print("l.find(8) == -1")
			l.append(0)
		for x in l:
			print(x)
	)");
	
	for (auto x: list) std::cout << "c++ " << x << "\n";
	
	BOOST_CHECK_EQUAL(std::accumulate(list.begin(), list.end(), 0), 16);
	BOOST_CHECK_EQUAL(list.back(), 0);
	BOOST_CHECK_EQUAL(list.size(), 5);
	
	std::cout << "get via []\n";
	pybind11::exec(R"(
		l = demo.get1()
		print("l:", l, len(l), type(l))
		if l[3] == 1 and l[2] != 5:
			l.pop_back()
		for x in l:
			print(x)
	)");
	
	for (auto x: list) std::cout << "c++ " << x << "\n";
	
	BOOST_CHECK_EQUAL(std::accumulate(list.begin(), list.end(), 0), 16);
	BOOST_CHECK_EQUAL(list.back(), 1);
	BOOST_CHECK_EQUAL(list.size(), 4);
	
	set_checksum = 0;
	std::cout << "create and passing to C++\n";
	pybind11::exec(R"(
		l = demo.IntList()
		l.append(6)
		l.push_front(1)
		l.push_back(2)
		demo.set(l)
	)");
	BOOST_CHECK_EQUAL(set_checksum, 19);
	
	set_checksum = 0;
	std::cout << "constructor from list\n";
	pybind11::exec(R"(
		x = demo.IntList([2, 5, 7])
		print(x)
		demo.set(x)
	)");
	BOOST_CHECK_EQUAL(set_checksum, 33);
	
	set_checksum = 0;
	std::cout << "implicitly_convertible\n";
	pybind11::exec(R"(
		demo.set([1, 3, 3])
	)");
	BOOST_CHECK_EQUAL(set_checksum, 16);
	
	pybind11::exec(R"(
		a = demo.A.getPtr()
		l = a.get()
		print ("a.list:", len(l), l)
		for x in l:
			print(x)
		l.pop_back()
		for x in l:
			print(x)
	)");
	
	BOOST_CHECK_EQUAL(1, 0);
	
	/// @todo TODO.4 std::list<T> when comparison of T type objects is defined and when is not defined
}
