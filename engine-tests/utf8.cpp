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
#define BOOST_TEST_MODULE Utf8
#include <boost/test/unit_test.hpp>

#include "Utf8.h"

#include <codecvt>
#include <locale>

BOOST_AUTO_TEST_CASE( toUCS4_vs_std_codecvt ) {
	std::string utf8 = "\u0065\u6c34\U0001d10b ¢→𐍈";
	
	const char* s = utf8.data();
	const char* e = s + utf8.length();
	
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
	std::u32string utf32 = utf32conv.from_bytes(utf8);
	
	int i=0;
	while (s<e) {
		BOOST_CHECK_EQUAL( MGE::UTF8::toUCS4(s, e), static_cast<uint32_t>(utf32[i++]) );
	}
	BOOST_CHECK_EQUAL(i, 7);
}

BOOST_AUTO_TEST_CASE( toUCS4_invalid ) {
	std::string utf8 = "\xc1\x81"; // invalid (two byte) encoded 'A'
	
	const char* s = utf8.data();
	const char* e = s + utf8.length();
	
	int i=0;
	while (s<e) {
		BOOST_CHECK_EQUAL( MGE::UTF8::toUCS4(s, e), 'A' ); i++;
	}
	BOOST_CHECK_EQUAL(i, 1);
}

BOOST_AUTO_TEST_CASE( fromUCS4 ) {
	char buf[8];
	size_t ret;
	
	ret = MGE::UTF8::fromUCS4(buf, 0xa2);
	buf[ret] = '\0';
	BOOST_CHECK_EQUAL( buf, "¢" );
	BOOST_CHECK_EQUAL( ret, 2 );
	
	ret = MGE::UTF8::fromUCS4(buf, 0x2192);
	buf[ret] = '\0';
	BOOST_CHECK_EQUAL( buf, "→" );
	BOOST_CHECK_EQUAL( ret, 3 );
	
	ret = MGE::UTF8::fromUCS4(buf, 0x10348);
	buf[ret] = '\0';
	BOOST_CHECK_EQUAL( buf, "𐍈" );
	BOOST_CHECK_EQUAL( ret, 4 );
}

BOOST_AUTO_TEST_CASE( getCharsLen ) {
	std::string utf8 = "«»+→5";
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8),    5 );
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 1), 4 ); // after skip 1 byte we hae 4 complete chars
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 2), 4 ); // the same after skip 2 bytes
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 0, 4), 2 );
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 0, 3), 2 ); // getCharsLen don't check completeness of last char
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 1, 4), 1 );
	BOOST_CHECK_EQUAL( MGE::UTF8::getCharsLen(utf8, 1, 3), 1 );
}

BOOST_AUTO_TEST_CASE( getByteLen ) {
	std::string utf8 = "«»+→5";
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8), 2 ); // first character («) is 2 bytes
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 1, 1), 3 ); // after skip 1 byte we have 1 byte from first character («), and two bytes from second character (»)
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 2, 1), 4 ); // 3th character (+) is one byte
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 10), 9 ); // string don't have 10 character, so return full length in bytes
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 2, 4, 6), 2 ); // in bytes range we have '+' and first byte of '→', so 2 bytes
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 2, 4, 8), 4 ); // in bytes range we have '+' and '→', so 4 bytes
	BOOST_CHECK_EQUAL( MGE::UTF8::getByteLen(utf8, 2, 4, 9), 4 ); // in bytes range we have '+', '→', and '5', but we count only 2 character, so 4 bytes
}

BOOST_AUTO_TEST_CASE( getSubStr ) {
	std::string utf8 = "«»+→5";
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 2), "+→5" ); // skip 2 first character
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 2, 2), "+→" ); // skip 2 first character and print 2 character
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 2, 2, 1), "→5" ); // after skip first byte we start on invalid byte (ignored)
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 2, 1, 1), "→" );
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 0, std::string::npos, 1, 5), "\xab»+" ); // 4 bytes: 1 invalid, 2 for '»', 1 for '+'
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 0, std::string::npos, 1, 3), "\xab\xc2" ); // 2 bytes: 1 invalid, invalid 
}

BOOST_AUTO_TEST_CASE( getSubStr_grapheme ) {
	std::string utf8 = "\x61\xcc\x8b";
	
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 0, 2), "\x61\xcc\x8b" );
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 0, 1), "\x61" );
	BOOST_CHECK_EQUAL( MGE::UTF8::getSubStr(utf8, 1, 1), "\xcc\x8b" );
}

/*
	auto x =  MGE::UTF8::getSubStr(utf8, 2, 4);
	for (int i=0; i<x.size();++i)
		std::cout << std::hex << std::showbase << (int)x[i] << '\n';
	std::cout << '\n';
*/
