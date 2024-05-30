/*
Copyright (c) 2017-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#pragma   once

#include "x_string_view.h"

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

	/// Null terminated string (C-string) constant reference type (alias for const char*)
	typedef const char* null_end_string;
	
	/// Empty string object (to use in reference return, etc)
	extern const std::string EMPTY_STRING;
	
	/// Empty string view object (to use in reference return, etc)
	extern const std::string_view EMPTY_STRING_VIEW;

	/// Struct using for support get element from unordered_map / unordered_set (with std::string key) by .find() with std::string_view or NULL-end string
	struct string_hash {
		/// class used to generate hash
		using hash_type = std::hash<std::string_view>;
		/// define type `is_transparent` needed to use this struct by overloaded methods of unordered cointainers
		using is_transparent = std::true_type;
		
		/// operator to calculate hash from different string types @{
		std::size_t operator()(const char* str) const             { return hash_type{}(str); }
		std::size_t operator()(const std::string_view& str) const { return hash_type{}(str); }
		std::size_t operator()(const std::string& str) const      { return hash_type{}(str); }
		/// @}
	};
	
/// @}
}

// enable string literals ("..."s for std::string)
using namespace std::literals::string_literals;

// enable string_view_literals ("..."sv for std::string_view)
using namespace std::literals::string_view_literals;
