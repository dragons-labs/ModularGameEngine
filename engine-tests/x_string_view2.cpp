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
#define BOOST_TEST_MODULE XStringView
#include <boost/test/unit_test.hpp>

#include "StringUtils.h"

// must be separated from x_string_view1.cpp test because we want true std::string

BOOST_AUTO_TEST_CASE( x_string_view_null_end ) {
	std::string b0("1234");
	MGE::x_string_view a("ABCD");
	MGE::x_string_view b(b0);
	std::cout << a << b << "\n";
	
	BOOST_CHECK_EQUAL( a.null_end(), true);
	BOOST_CHECK_EQUAL( b.null_end(), true);
	
	MGE::x_string_view c("XYZQ", 2);
	MGE::x_string_view d("0987"sv);
	std::cout << c << d << "\n";
	
	BOOST_CHECK_EQUAL( c.null_end(), false);
	BOOST_CHECK_EQUAL( d.null_end(), false);
}

BOOST_AUTO_TEST_CASE( string_view_add ) {
	const char* c_str = "ABC";
	std::string cpp_str = "123";
	std::string_view cpp_str_v = "XYZ";
	MGE::x_string_view x_str("QWE");
	
	// std::string / std::string_view
	BOOST_CHECK_EQUAL( cpp_str + cpp_str_v, "123XYZ");
	BOOST_CHECK_EQUAL( cpp_str_v + cpp_str, "XYZ123");
	
	// c string / std::string_view
	BOOST_CHECK_EQUAL( c_str + cpp_str_v, "ABCXYZ");
	BOOST_CHECK_EQUAL( cpp_str_v + c_str, "XYZABC");
	
	// std::string / x_string_view
	BOOST_CHECK_EQUAL( cpp_str + x_str, "123QWE");
	BOOST_CHECK_EQUAL( x_str + cpp_str, "QWE123");
	
	// c string / x_string_view
	BOOST_CHECK_EQUAL( c_str + x_str, "ABCQWE");
	BOOST_CHECK_EQUAL( x_str + c_str, "QWEABC");
	
	// x_string_view / std::string_view
	BOOST_CHECK_EQUAL( x_str + cpp_str_v, "QWEXYZ");
	BOOST_CHECK_EQUAL( cpp_str_v + x_str, "XYZQWE");
}
