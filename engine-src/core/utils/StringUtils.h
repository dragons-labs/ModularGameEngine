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

#include "Utf8.h"
#include "StringOperators.h"

#ifndef __DOCUMENTATION_GENERATOR__
#if defined(DEBUG_STRING_TO_MASK) && !defined(DEBUG_STRING_TO_MASK_LOG)
	#include "LogSystem.h"
	#define DEBUG_STRING_TO_MASK_LOG(a, b) LOG_DEBUG(a, b)
#else
	#define DEBUG_STRING_TO_MASK_LOG(a, b)
#endif
#endif

#include <functional>
#include <sstream>
#include <tuple>
#include <charconv>

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/**
 * @brief String utilities functions.
 */
namespace StringUtils {
	/**
	 * @brief Function doing conversion "a b d" string to numeric mask value of "a | b | d".
	 * 
	 * @param string             String to convert.
	 * @param convertionFunction Function used to convert single world from @a string (e.g. "a" in above example) to numeric value.
	 *                           When function return 0, word will be converted as numeric value (with 0x prefix support).
	 */
	template<typename ReturnType, typename StringType, typename ConvertFunction> inline ReturnType stringToNumericMask(const StringType& string, ConvertFunction convertionFunction) {
		DEBUG_STRING_TO_MASK_LOG("", "stringToNumericMask input_string: " << string);
		
		std::istringstream  valueStream(static_cast<std::string>(string));
		std::string         valueSubStr;
		ReturnType          retVal = 0;
		
		while (std::getline (valueStream, valueSubStr, ' ')) {
			ReturnType newVal = convertionFunction(valueSubStr);
			if (!newVal) {
				newVal = static_cast<ReturnType>( std::strtoll(valueSubStr.c_str(), nullptr, 0) );
			}
			retVal = retVal | newVal;
			DEBUG_STRING_TO_MASK_LOG("stringToNumericMask", "str=" << valueSubStr << " => newVal=" << std::hex << std::showbase << +newVal << " and retVal=" << +retVal);
		}
		
		return retVal;
	}
	
	/**
	 * @brief Read numeric system base from prefix in string.
	 * 
	 * @param str   String with (optionally prefixed) numeric value.
	 * @param def   Default base value to return if no prefix in @a str (or length of @a str <= 2, so if prefixed, then without value).
	 * 
	 * @remark Support prefixes:
	 *   @li 0b or 0B → binary
	 *   @li 0o or 0O → octal
	 *   @li 0d or 0D → decimal
	 *   @li 0b or 0B → hex
	 */
	std::tuple<char,char> getNumericBase(const std::string_view& str, char def = 10);
	
	/**
	 * @brief Convert string to numeric value. Support prefixes via @ref getNumericBase.
	 * 
	 * @param str   String with (optionally prefixed) numeric value.
	 * @param base  Integer base to use. 0 to detect by prefix.
	 * @param def   Default value to return if conversion fail.
	 */
	template<typename ReturnType> ReturnType toNumeric(const std::string_view& str, char base, ReturnType def) {
		// get base and offset from string begin
		char offset = 0;
		if (base == 0) {
			std::tie(base, offset) = getNumericBase(str);
		}
		
		// convert to numeric
		ReturnType result;
		if (std::from_chars(str.data() + offset, str.data() + str.size(), result, base).ec == std::errc())
			return result;
		else
			return def;
	}
	
	/**
	 * @brief Convert string to numeric value. Support prefixes via @ref getNumericBase.
	 * 
	 * @param str   String with (optionally prefixed) numeric value.
	 * @param base  Integer base to use. 0 to detect by prefix.
	 * 
	 * Throw exception if conversion fail.
	 */
	template<typename ReturnType> ReturnType toNumeric(const std::string_view& str, char base = 0) {
		// get base and offset from string begin
		char offset = 0;
		if (base == 0) {
			std::tie(base, offset) = getNumericBase(str);
		}
		
		// convert to numeric
		ReturnType result;
		if (std::from_chars(str.data() + offset, str.data() + str.size(), result, base).ec == std::errc())
			return result;
		else
			throw std::logic_error("Can't convert \"" + str + "\" to numeric value");
	}
}

/// @}
}
