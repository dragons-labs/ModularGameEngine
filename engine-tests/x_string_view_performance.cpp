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


#define count 100000

#define napis "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed erat lorem, tempor ac tincidunt sit amet, lacinia id orci. Aenean lobortis tempor nisi, ut tempus magna bibendum vitae. Fusce nec pellentesque dui. Aliquam finibus risus lorem, id condimentum ligula elementum a. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nulla facilisi. Donec egestas, eros laoreet placerat molestie, massa libero gravida quam, nec condimentum dolor tellus non justo. Interdum et malesuada fames ac ante ipsum primis in faucibus. Nulla consectetur pulvinar lorem id aliquam. Suspendisse faucibus lobortis euismod. Etiam blandit sit amet felis in vulputate. Donec vitae augue ullamcorper, rhoncus magna non, euismod risus. Nulla non lectus ornare, auctor dolor in, auctor nisi. Cras aliquet ligula tellus, quis convallis dui dictum eu. Nunc at fringilla lacus."

long long xx;

// C string

void a1(const char* x) {
	xx += x[30];
}
void a2(MGE::null_end_string x) {
	xx +=  x[30];
}
void a3(const MGE::null_end_string& x) {
	xx +=  x[30];
}

BOOST_AUTO_TEST_CASE( c_str ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		a1(napis);
}
BOOST_AUTO_TEST_CASE( c_str_typef ) { // for test error
	xx = 0;
	for (int i=0; i<count; ++i)
		a2(napis);
}
BOOST_AUTO_TEST_CASE( c_str_ref ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		a3(napis);
}

// string view

void b1(const std::string_view& x) {
	xx +=  x[30];
}
void b2(std::string_view x) {
	xx +=  x[30];
}

BOOST_AUTO_TEST_CASE( std_string_view_ref ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		b1(napis);
}
BOOST_AUTO_TEST_CASE( std_string_view ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		b2(napis);
}

BOOST_AUTO_TEST_CASE( std_string_view_ref__witout_construct ) {
	xx = 0;
	auto a = std::string_view(napis);
	for (int i=0; i<count; ++i)
		b1(a);
}
BOOST_AUTO_TEST_CASE( std_string_view__witout_construct ) {
	xx = 0;
	auto a = std::string_view(napis);
	for (int i=0; i<count; ++i)
		b2(a);
}

// x string view

void c1(const MGE::x_string_view& x) {
	xx +=  x[30];
}
void c2(MGE::x_string_view x) {
	xx +=  x[30];
}

BOOST_AUTO_TEST_CASE( x_string_view_ref ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		c1(napis);
}
BOOST_AUTO_TEST_CASE( x_string_view ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		c2(napis);
}

BOOST_AUTO_TEST_CASE( x_string_view_ref__witout_construct ) {
	xx = 0;
	auto a = MGE::x_string_view(napis);
	for (int i=0; i<count; ++i)
		c1(a);
}
BOOST_AUTO_TEST_CASE( x_string_view__witout_construct ) {
	xx = 0;
	auto a = MGE::x_string_view(napis);
	for (int i=0; i<count; ++i)
		c2(a);
}

BOOST_AUTO_TEST_CASE( x_string_view_ref__from_string ) {
	xx = 0;
	auto a = std::string(napis);
	for (int i=0; i<count; ++i)
		c1(a);
}
BOOST_AUTO_TEST_CASE( x_string_view__from_string ) {
	xx = 0;
	auto a = std::string(napis);
	for (int i=0; i<count; ++i)
		c2(a);
}

// string

void d1(const std::string& x) {
	xx +=  x[30];
}
void d2(std::string x) {
	xx +=  x[30];
}

BOOST_AUTO_TEST_CASE( std_string_ref ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		d1(napis);
}
BOOST_AUTO_TEST_CASE( std_string ) {
	xx = 0;
	for (int i=0; i<count; ++i)
		d2(napis);
}

BOOST_AUTO_TEST_CASE( std_string_ref__witout_construct ) {
	xx = 0;
	auto a = std::string(napis);
	a.append("XYZ");
	for (int i=0; i<count; ++i)
		d1(a);
}
BOOST_AUTO_TEST_CASE( std_string__witout_construct ) {
	xx = 0;
	auto a = std::string(napis);
	a.append("XYZ");
	for (int i=0; i<count; ++i)
		d2(a);
}
