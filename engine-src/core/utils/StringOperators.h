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

#include "StringTypedefs.h"

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/// Concatenate std::string_view and std::string. @{
template<typename Ch, typename T, typename A> inline std::basic_string<Ch, T, A> operator+(const std::basic_string<Ch, T, A>& a, const std::basic_string_view<Ch, T>& b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
template<typename Ch, typename T, typename A> inline std::basic_string<Ch, T, A> operator+(const std::basic_string_view<Ch, T>& a, const std::basic_string<Ch, T, A>& b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
///@}

/// Concatenate std::string_view and null-end C strings. @{
template<typename Ch, typename T, typename A = std::allocator<Ch>> inline std::basic_string<Ch, T, A> operator+(const Ch* a, const std::basic_string_view<Ch, T>& b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
template<typename Ch, typename T, typename A = std::allocator<Ch>> inline std::basic_string<Ch, T, A> operator+(const std::basic_string_view<Ch, T>& a, const Ch* b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
///@}

/// Concatenate std::string_view and MGE::x_string_view. @{
template<typename Ch, typename T, typename A = std::allocator<Ch>> inline std::basic_string<Ch, T, A> operator+(const MGE::x_string_view& a, const std::basic_string_view<Ch, T>& b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
template<typename Ch, typename T, typename A = std::allocator<Ch>> inline std::basic_string<Ch, T, A> operator+(const std::basic_string_view<Ch, T>& a, const MGE::x_string_view& b) {
	std::basic_string<Ch, T, A> x(a);
	x.append(b);
	return x;
}
///@}

/// @}
