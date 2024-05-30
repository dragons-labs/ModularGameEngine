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

#include "StringUtils.h"
#include "pragma.h"

MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors")

const std::string      MGE::EMPTY_STRING("");
const std::string_view MGE::EMPTY_STRING_VIEW( MGE::EMPTY_STRING );

MGE_CLANG_WARNING_POP


std::tuple<char,char> MGE::StringUtils::getNumericBase(const std::string_view& str, char def) {
	if (str.size()>2 && str[0] == '0') {
		switch (str[1]) {
			case 'b':
			case 'B':
				return std::make_tuple(2, 2);
			case 'o':
			case 'O':
				return std::make_tuple(8, 2);
			case 'd':
			case 'D':
				return std::make_tuple(10, 2);
			case 'x':
			case 'X':
				return std::make_tuple(16, 2);
		}
	}
	return std::make_tuple(def, 0);
}
