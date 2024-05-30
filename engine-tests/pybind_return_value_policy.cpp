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

int constr_count, destr_count;

struct A {
	A() {
		std::cout << "CREATE " << (void*)this << "\n";
		constr_count++;
	}
	~A() {
		std::cout << "DELETE " << (void*)this << "\n";
		destr_count++;
	}
};

A* obj;

A* get() {
	return obj;
}

PYBIND11_EMBEDDED_MODULE(demo, m) {
	m.def("get1", &get);
	m.def("get2", &get, pybind11::return_value_policy::reference);
	pybind11::class_<A>(m, "A");
} 

BOOST_AUTO_TEST_CASE( return_value_policy ) {
	pybind11::scoped_interpreter guard{};
	pybind11::exec("import demo, sys, gc");
	
	std::cout << "demo\n";
	constr_count = destr_count = 0;
	obj = new A();
	pybind11::exec(R"(
		a = demo.get1()
		print("refcount a", sys.getrefcount(a)-1)
		del a
		gc.collect()
	)");
	BOOST_CHECK_EQUAL(constr_count, 1);
	BOOST_CHECK_EQUAL(destr_count, 1);
	
	std::cout << "test2\n";
	constr_count = destr_count = 0;
	obj = new A();
	pybind11::exec(R"(
		b = demo.get2()
		print("refcount b", sys.getrefcount(b)-1)
		del b
		gc.collect()
	)");
	BOOST_CHECK_EQUAL(constr_count, 1);
	BOOST_CHECK_EQUAL(destr_count, 0);
	
	std::cout << "test3\n";
	constr_count = destr_count = 0;
	obj = new A();
	pybind11::globals()["c"] = obj; // obj => pybind11::cast(obj) => pybind11::cast(obj, pybind11::return_value_policy::automatic_reference) =[obj is pointer]=> pybind11::cast(obj, pybind11::return_value_policy::reference)
	pybind11::exec(R"(
		print("refcount c", sys.getrefcount(c)-1)
		del c
		gc.collect()
	)");
	BOOST_CHECK_EQUAL(constr_count, 1);
	BOOST_CHECK_EQUAL(destr_count, 0);
	
	std::cout << "test4\n";
	constr_count = destr_count = 0;
	obj = new A();
	pybind11::globals()["d"] = pybind11::cast(obj, pybind11::return_value_policy::automatic);
	pybind11::exec(R"(
		print("refcount d", sys.getrefcount(d)-1)
		del d
		gc.collect()
	)");
	BOOST_CHECK_EQUAL(constr_count, 1);
	BOOST_CHECK_EQUAL(destr_count, 1);
	
	std::cout << "test5\n";
	constr_count = destr_count = 0;
	obj = new A();
	pybind11::globals()["e"] = obj;
	pybind11::exec(R"(
		f = demo.get2()
		print("refcount f", sys.getrefcount(f)-1)
		g = (id(f)==id(e))
	)");
	bool g = pybind11::globals()["g"].cast<bool>();
	BOOST_CHECK_EQUAL(g, true);
}
