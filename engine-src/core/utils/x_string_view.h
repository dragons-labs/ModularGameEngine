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

#include "force_inline.h"

#include <string>
#include <string_view>

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/**
 * @brief std::string_view with null-end info.
 *        Can be converted into C null end string for function argument with macro @ref X_STRING_C_STR.
 * 
 */
template<typename Ch> class basic_x_string_view : public std::basic_string_view<Ch> {
private:
	/// @brief Null end indicator / pointer to source string.
	///
	/// @li 0 → not null end
	/// @li 1 → null end
	/// @li otherwise → pinter to std::string witd source buffer (this->data() == this->source.data())
	const void* source; // due to alignment with base class fields, memory cost is the same as for bool
public:
	/// Constructor from std::basic_string (std::string).
	FORCE_INLINE basic_x_string_view(const std::basic_string<Ch>& s) :
		std::basic_string_view<Ch>(s.data(), s.length()),
		source(reinterpret_cast<const void*>(&s))
	{}
	
	/// Constructor from null-end C string.
	FORCE_INLINE basic_x_string_view(const Ch* s) :
		std::basic_string_view<Ch>(s),
		source(reinterpret_cast<const void*>(1))
	{}
	
	/// Constructor from std::basic_string_view (std::string_view).
	FORCE_INLINE basic_x_string_view(const std::basic_string_view<Ch>& s, bool is_null_terminated = false) :
		std::basic_string_view<Ch>(s.data(), s.length()),
		source(reinterpret_cast<const void*>(is_null_terminated)) // bool(true) is 1, bool(false) is 0 (C++17 §7.8/4)
	{}
	
	/// Constructor from string + length.
	FORCE_INLINE basic_x_string_view(const Ch* s, size_t l, bool is_null_terminated = false) :
		std::basic_string_view<Ch>(s, l),
		source(reinterpret_cast<const void*>(is_null_terminated)) // bool(true) is 1, bool(false) is 0 (C++17 §7.8/4)
	{}
	
	/// Return true if underlayer string (obtain by data() from base std::basic_string_view class) is null ended.
	FORCE_INLINE constexpr bool null_end() const noexcept {
		return source;
	}
	
	/// Return pointer underlayer C++ string object (if created from it) or less that 2 value
	FORCE_INLINE constexpr const std::basic_string<Ch>* get_source() const noexcept {
		return reinterpret_cast<const std::basic_string<Ch>*>(source);
	}
	
	/// Explicit convert to std::basic_string (std::string) as method.
	FORCE_INLINE constexpr std::basic_string<Ch> string() const {
		return std::basic_string<Ch>(this->data(), this->size());
	}
};

/// See @ref MGE::basic_x_string_view.
typedef basic_x_string_view<char> x_string_view;

/// @}
}

/// @addtogroup StringAndXMLUtils
/// @{

/**
 * @brief Macro to convert @ref MGE::basic_x_string_view object to C NULL-end string for function argument.
 *
 * @warning NEVER store/assign value returned by this macro. Only to use in functions arguments.
 * 
 * @remark  This must be macro due to problem with too early call destructor on tempoary ``std::string``
 *          returned by string() in construct like ``xxx(str.c_str())`` when ``str`` is not null end and:
 *          ``getCStr() { if(is_null_end) return data() else return string().c_str(); }``
 */
#define X_STRING_C_STR(s) s.null_end() ? s.data() : s.string().c_str()

/**
 * @brief Macro to convert @ref MGE::basic_x_string_view object to C++ string for function argument.
 * 
 * In contrast of @ref MGE::basic_x_string_view::string return original C++ string object (do not create new),
 * when @ref MGE::basic_x_string_view was created from C++ string.
 */
#define X_STRING_CPP_STR(s) s.get_source() > reinterpret_cast<const void*>(1) ? *(s.get_source()) : s.string()

/// @}
