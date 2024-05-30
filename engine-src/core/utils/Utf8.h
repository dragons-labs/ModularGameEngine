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

#include <string>
#include <string_view>

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/**
 * @brief Helper functions for UTF-8 support in standard one-byte strings.
 * 
 * @attention
 *    They function care about 'code points', but not (multi code points) 'grapheme' (based on combining character, etc).
 *    For example @ref UTF8::getSubStr :
 *        - Care about not breaking bytes sequence inside single code points, so don't break inside "\xe2\x88\x9e" (∞)
 *        - But don't care about breaking code points sequence inside single grapheme, so can break "\x61\xcc\x8b" (a̋) into "\x61" (a) and "\xcc\x8b" (combing version of ˝).
 * 
 * `#include <utf8.h>`
 */
namespace UTF8 {
	/**
	 * @brief Return number of characters (code points, not bytes/code units) in utf8 (sub)string.
	 * 
	 * @param  utf8_str      String to calculate length.
	 * @param  byte_start    Start position (bytes/code units, not characters/code points) of this string for counting utf8 characters.
	 * @param  byte_end      End position (bytes/code units, not characters/code points) in this string for counting utf8 characters.
	 *
	 * @return  Number of Unicode code points beginning in selected utf8_str substring (from ``utf8_str[byte_start]`` to ``utf8_str[byte_end-1]`` inclusive).
	 * 
	 * @remark
	 *     - If at beginning of input string (``utf8_str[byte_start]``) are utf8 continue byte/code unit, they will be ignored (don't count into returned value).
	 *     - If at end of input string start utf8 multibyte character, but the continue byte are missed, it will be count into returned value (without error).
	 */
	size_t getCharsLen(const std::string_view& utf8_str, size_t byte_start = 0, size_t byte_end = std::string::npos);
	
	/**
	 * @brief Return number of bytes/code units (not characters/code points) in utf8 (sub)string.
	 * 
	 * @param  utf8_str      String to calculate length.
	 * @param  characters    Number of characters (code points, not bytes/code units) to count.
	 * @param  byte_start    Start position (bytes, not characters/code points) of this string for counting utf8 characters.
	 * @param  byte_end      End position (bytes, not characters/code points) in this string for counting utf8 characters.
	 *
	 * @return  Number of bytes/code units used by utf8 (sub)string
	 * 
	 * @remark
	 *     - If at beginning of input string (``utf8_str[byte_start]``) are utf8 continue byte/code unit, they will be counted as byte but not as character.
	 *     - If input string (from ``utf8_str[byte_start]`` to ``utf8_str[byte_end-1]`` inclusive) contains less that requested characters, will be returned len of input string (byte_end-byte_start).
	 */
	size_t getByteLen(const std::string_view& utf8_str, size_t characters = 1, size_t byte_start = 0, size_t byte_end = std::string::npos);
	
	/**
	 * @brief  Return utf8 substring.
	 * 
	 * @param  utf8_str      Source string to get substring.
	 * @param  chars_offset  Number of characters (code points, not bytes/code units) to skip.
	 * @param  characters    Number of characters (code points, not bytes/code units) to return.
	 * @param  byte_offset   Number of bytes/code units to skip in input string, before get first utf8 character.
	 * @param  byte_end      End position (bytes/code units, not characters/code points) in this string for processing.
	 *                       This is end of string position, so it's stronger than any calculation from previous arguments.
	 *
	 * @return  requested substring as (NOT null end!) string view on input buffer @a utf8_str
	 * 
	 * @remark
	 *     Internal use @ref getByteLen, so see notes for this function.
	 */
	const std::string_view getSubStr(const std::string_view& utf8_str, size_t chars_offset, size_t characters = std::string::npos, size_t byte_offset = 0, size_t byte_end = std::string::npos);
	 
	/**
	 * @brief Return Unicode character (code point) from utf8 string staring at @a start,
	 *        update @a start to point next character.
	 * 
	 * @param[in,out] start   Pointing first byte of character to get.
	 *                        After execution will point first byte after obtained character.
	 * @param[in]     end     Pointing first byte after string (for null-end string: ``*end == '\0'``).
	 *
	 * @return  Unicode code point value.
	 * 
	 * \par Example
		\code{.cpp}
			std::string str("ą¢«𐍈»œł");
			const char* s = str.data();
			const char* e = s + str.length();
			while (s<e)
				std::cout << std::hex << std::showbase << UTF8::toUCS4(s, e) << '\n';
		\endcode
	 *
	 * @warning
	 *     - This function convert illegal UTF-8 sequence too. For example ``"\xc1\x81"`` will return 0x41 ('A').
	 *     - This function don't check ``(byte & 0xC0) == 0x80`` on continuation bytes.
	 *       Simply treats the number of bytes, specified by first byte, as continue bytes.
	 *
	 * @remark
	 *     If you need checking for illegal UTF-8 sequence you can use (deprecated in C++17) std::wstring_convert:
		\code{.cpp}
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
			std::u32string str_utf32 = utf32conv.from_bytes(str);
			for (char32_t c : str_utf32)
				std::cout << std::hex << std::showbase << c << '\n';
		\endcode
	 */
	uint32_t toUCS4(const char*& start, const char* end);
	
	/**
	 * @brief Write single character utf8 (multi-byte) string from UCS4 code value (@a c) to @a buf.
	 * 
	 * @param[out]    buf   Pointing place to put first byte of utf8 encoded character.
	 *                      Buffer must be preallocated with sufficient size.
	 * @param[in]     c     Unicode code point value of character to convert to utf8.
	 *
	 * @return  Number of bytes/code units write to @a buf,
	 *          so buf+returned_value can be used to put next character via fromUCS4.
	 * 
	 * \par Example
		\code{.cpp}
			char buf[6], len;
			len = UTF8::fromUCS4(c, buf);
			std::string_view(buf, len);
		\endcode
	 *
	 * @attention Function do not add '\0' (null) to buffer after last byte of character.
	 */
	uint8_t fromUCS4(char* buf, uint32_t c);
	
	/**
	 * @brief Append single Unicode character (multi-byte utf8) to @a dst.
	 * 
	 * @param[in,out] dst   String object to append character.
	 * @param[in]     c     Unicode code point value of character to convert to utf8 and add to @a dst.
	 */
	void appendFromUCS4(std::string& dst, uint32_t c);
};
/// @}
}
