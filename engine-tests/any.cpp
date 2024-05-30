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

#include "data/property/Any.h"

int x_check;

struct AnyHolder {
	AnyHolder() {
		std::cout << "Constructor\n";
	}
	~AnyHolder() {
		std::cout << "Destructor\n";
		x_check += 1;
	}
	friend inline std::ostream& operator<<(std::ostream& s, const AnyHolder& obj) { return s; }
};

BOOST_AUTO_TEST_CASE( anyDestruction ) {
	x_check = 0;
	std::list<Ogre::Any> anys;
	
	anys.push_back(Ogre::Any(AnyHolder()));
	
	std::cout << "-\n";
	
	anys.emplace_back(AnyHolder());
	
	std::cout << "do clear ...\n";
	
	x_check = 0;
	anys.clear();
	BOOST_CHECK_EQUAL(x_check, 2);
	
	/// @todo TODO.4: write (Any and PropertySet) to XML (multiple variants - simple variable, list, map, ...)
	/// @todo TODO.4: write (Any and PropertySet) to string stream
	/// @todo TODO.4: read (Any and PropertySet) from  XML
}
