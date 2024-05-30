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

#pragma   once

#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>

namespace MGE {

/// @addtogroup CoreUtils
/// @{
/// @file

#ifndef __PYTHON_DOCUMENTATION_GENERATOR__

/// C++ concept for template with arithmetic arguments
template <typename T> concept Arithmetic = std::is_arithmetic<T>::value;

/// C++ concept for template with list-like argument (iterated like std::list).
template <typename T> concept ListType = 
	std::same_as<T, std::list<typename T::value_type, typename T::allocator_type>> ||
	std::same_as<T, std::forward_list<typename T::value_type, typename T::allocator_type>>;

/// C++ concept for template with list-like argument (iterated like std::map).
template <typename T> concept MapType = 
	std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
	std::same_as<T, std::multimap<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
	std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>> ||
	std::same_as<T, std::unordered_multimap<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

#endif

/// @}
}
