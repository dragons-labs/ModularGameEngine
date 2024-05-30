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

#if 1
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE XStringView
#include <boost/test/unit_test.hpp>
#else
#include <iostream>
#define BOOST_CHECK_EQUAL(a, b)
#define BOOST_AUTO_TEST_CASE(a) int main()
#endif

int constr_count, destr_count;

// provide fake std::string for x_string_view.h for testing purpose
#include <string_view>
#include <string>
namespace fake_std {
	template<class CharT, class Traits = std::char_traits<CharT>> using basic_string_view = std::basic_string_view<CharT, Traits>;
	template<class CharT> class basic_string : public std::basic_string<CharT> {
	public:
		basic_string(const basic_string& s) :
			std::string(s)
		{
			std::cout << "CREATE fake_std::string(copy) " << s << "\n";
			constr_count++;
		}
		
		basic_string(const basic_string&& s) :
			std::string(s)
		{
			std::cout << "CREATE fake_std::string(move) " << s << "\n";
			constr_count++;
		}
		
		basic_string(const char* s) :
			std::string(s)
		{
			std::cout << "CREATE fake_std::string(c) " << s << "\n";
			constr_count++;
		}
		
		basic_string(const char* s, size_t l) :
			std::string(s, l)
		{
			std::cout << "CREATE fake_std::string(c,l)" << s << "\n";
			constr_count++;
		}
		
		~basic_string() {
			std::cout << "DESTROY fake_std::string " << this->data() << "  " << this << "\n";
			destr_count++;
		}
	};
}
#define std fake_std
#include "x_string_view.h"
#undef std

#include "StringUtils.h"


void f1(const char* xx) {
	std::cout << "f1: " << xx << "\n";
}

void f3(MGE::x_string_view xx) {
	std::cout << "f3: " << xx << "\n";
}

void f2(const fake_std::basic_string<char>& xx) {
	MGE::x_string_view yy(xx);
	f3(yy);
	std::cout << "f2: " << yy << "\n";
}

BOOST_AUTO_TEST_CASE( x_string_view ) {
	MGE::x_string_view a("ABCDE");
	auto b = a.substr(1,2);  // std::string_view
	MGE::x_string_view c(b); // from std::string_view
	
	constr_count = destr_count = 0;
	f1(X_STRING_C_STR(a));
	BOOST_CHECK_EQUAL( constr_count, 0 ); // no construction of basic_string
	BOOST_CHECK_EQUAL( constr_count, destr_count);
	
	constr_count = destr_count = 0;
	f1(X_STRING_C_STR(c));
	BOOST_CHECK_EQUAL( constr_count, 1 ); // single construction of basic_string
	BOOST_CHECK_EQUAL( constr_count, destr_count);
	
	std::cout << "\n";
	constr_count = destr_count = 0;
	{
	fake_std::basic_string<char> e("12345");
	f2(e);
	}
	BOOST_CHECK_EQUAL( constr_count, 1 ); // single construction of basic_string
	BOOST_CHECK_EQUAL( constr_count, destr_count);
}

BOOST_AUTO_TEST_CASE( x_string_view_copy ) {
	constr_count = destr_count = 0;
	
	MGE::x_string_view a("ABCD");
	MGE::x_string_view b("XYZQ");
	
	BOOST_CHECK( a.data() != b.data() );
	
	std::cout << a << b << "\n";
	b = a;
	std::cout << a << b << "\n";
	
	BOOST_CHECK_EQUAL( a.null_end(), b.null_end() );
	BOOST_CHECK_EQUAL( a.size(), b.size() );
	BOOST_CHECK_EQUAL( a.data(), b.data() );
	
	BOOST_CHECK_EQUAL( constr_count, 0 );
	BOOST_CHECK_EQUAL( constr_count, destr_count);
}


void fx1(fake_std::basic_string<char> xx) {
	std::cout << "f2: " << xx << "  " << (void*) &xx << "\n";
}

BOOST_AUTO_TEST_CASE( std_string ) {
	MGE::x_string_view a("XYZQ");
	
	std::cout << "\n";
	constr_count = destr_count = 0;
	{
	fx1(a.string());
	}
	BOOST_CHECK_EQUAL( constr_count, 1 );
	BOOST_CHECK_EQUAL( constr_count, destr_count);
	
	std::cout << "\n";
	constr_count = destr_count = 0;
	{
	fx1(std::move(a.string()));
	}
	BOOST_CHECK_EQUAL( constr_count, 2 );
	BOOST_CHECK_EQUAL( constr_count, destr_count);
	
	// compare to direct use std::string
	
	std::cout << "\n";
	constr_count = destr_count = 0;
	{
	fake_std::basic_string<char> b("0987");
	std::cout << "--\n";
	fx1(b);
	std::cout << "--\n";
	}
	BOOST_CHECK_EQUAL( constr_count, 2 );
	BOOST_CHECK_EQUAL( constr_count, destr_count);
	
	std::cout << "\n";
	constr_count = destr_count = 0;
	{
	std::cout << "--\n";
	fx1(fake_std::basic_string<char>("0987"));
	std::cout << "--\n";
	}
	BOOST_CHECK_EQUAL( constr_count, 1 );
	BOOST_CHECK_EQUAL( constr_count, destr_count);
}
