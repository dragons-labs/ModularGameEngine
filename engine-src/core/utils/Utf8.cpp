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

#include "Utf8.h"

#ifndef __GNUC__
	#define __attribute__()
#endif

__attribute__((pure)) size_t MGE::UTF8::getCharsLen(const std::string_view& utf8_str, size_t byte_start, size_t byte_end) {
	if (byte_end == std::string::npos) {
		byte_end = utf8_str.size();
	}
	const char*  s = utf8_str.data() + byte_start;
	const char*  e = utf8_str.data() + byte_end;
	
	size_t len = 0;
	while (s < e) {
		const char& c = *s++;
		if ((c & 0x80) == 0x00 || (c & 0xc0) == 0xc0) { // ASCII or start of non-ASCII
			++len;
		}
	}
	return len;
}

__attribute__((pure)) size_t MGE::UTF8::getByteLen(const std::string_view& utf8_str, size_t characters, size_t byte_start, size_t byte_end) {
	if (byte_end == std::string::npos) {
		byte_end = utf8_str.size();
	}
	const char*  s = utf8_str.data() + byte_start;
	const char*  e = utf8_str.data() + byte_end;
	
	size_t len = 0;
	while (s < e) {
		const char& c = *s++;
		if ((c & 0x80) == 0x00 || (c & 0xc0) == 0xc0) { // ASCII or start of non-ASCII
			if (characters == 0) {
				break;
			} else {
				--characters;
			}
		}
		len++;
	}
	return len;
}

__attribute__((pure)) const std::string_view MGE::UTF8::getSubStr(const std::string_view& utf8_str, size_t chars_offset, size_t characters, size_t byte_offset, size_t byte_end) {
	if (chars_offset)
		byte_offset = byte_offset + getByteLen(utf8_str, chars_offset, byte_offset, byte_end);
	
	if (characters != std::string::npos)
		byte_end = getByteLen(utf8_str, characters, byte_offset, byte_end);
	else if (byte_end != std::string::npos)
		byte_end -= byte_offset; // std::string::substr use (start, count) not (start, end)
	
	return utf8_str.substr(byte_offset, byte_end);
}

uint32_t MGE::UTF8::toUCS4(const char*& start, const char* end) {
	uint32_t ret = 0;
	size_t str_len = end - start; // end pointing first byte after string (for null-end string: *end == '\0')
	
	if (str_len >= 1 && (*start & 0x80) == 0x00) {
		ret = *start;
	} else if (str_len >= 2 && (*start & 0xe0) == 0xc0) {
		ret  = ((*start     & 0x1f) << 6);
		ret |= ( *(++start) & 0x3f);
	} else if (str_len >= 3 && (*start & 0xf0) == 0xe0) {
		ret  = ((*start     & 0x0f) << 12);
		ret |= ((*(++start) & 0x3f) << 6);
		ret |= ( *(++start) & 0x3f);
	} else if (str_len >= 4 && (*start & 0xf8) == 0xf0) {
		ret  = ((*start     & 0x07) << 18);
		ret |= ((*(++start) & 0x3f) << 12);
		ret |= ((*(++start) & 0x3f) << 6);
		ret |= ( *(++start) & 0x3f);
	#ifdef NO_RFC3629
	// RFC 3629 remove 5 and 6 bytes UTF-8 sequences,
	// due to UTF-16 compatibility and Unicode range (0..0x10FFFF)
	} else if (str_len >= 5 && (*start & 0xfc) == 0xf8) {
		ret  = ((*start     & 0x03) << 24);
		ret |= ((*(++start) & 0x3f) << 18);
		ret |= ((*(++start) & 0x3f) << 12);
		ret |= ((*(++start) & 0x3f) << 6);
		ret |= ( *(++start) & 0x3f);
	} else if (str_len >= 6 && (*start & 0xfe) == 0xfc) {
		ret  = ((*start     & 0x01) << 30);
		ret |= ((*(++start) & 0x3f) << 24);
		ret |= ((*(++start) & 0x3f) << 18);
		ret |= ((*(++start) & 0x3f) << 12);
		ret |= ((*(++start) & 0x3f) << 6);
		ret |= ( *(++start) & 0x3f);
	#endif
	}
	
	++start;
	return ret;
}

uint8_t MGE::UTF8::fromUCS4(char* buf, uint32_t c) {
	if (c < 0x80) {
		buf[0] = c;
		return 1;
	} else if (c < 0x800) {
		buf[0] = (c >> 6) | 0xc0;
		buf[1] = (c         & 0x3f) | 0x80;
		return 2;
	} else if (c < 0x10000) {
		buf[0] = (c >> 12) | 0xe0;
		buf[1] = ((c >>  6) & 0x3f) | 0x80;
		buf[2] = ( c        & 0x3f) | 0x80;
		return 3;
	} else if (c < 0x200000) {
		buf[0] = (c >> 18) | 0xf0;
		buf[1] = ((c >> 12) & 0x3f) | 0x80;
		buf[2] = ((c >>  6) & 0x3f) | 0x80;
		buf[3] = ( c        & 0x3f) | 0x80;
		return 4;
	#ifdef NO_RFC3629
	// RFC 3629 remove 5 and 6 bytes UTF-8 sequences,
	// due to UTF-16 compatibility and Unicode range (0..0x10FFFF)
	} else if (c < 0x4000000) {
		buf[0] = (c >> 24) | 0xf8;
		buf[1] = ((c >> 18) & 0x3f) | 0x80;
		buf[2] = ((c >> 12) & 0x3f) | 0x80;
		buf[3] = ((c >>  6) & 0x3f) | 0x80;
		buf[4] = ( c        & 0x3f) | 0x80;
		return 5;
	} else {
		buf[0] = (c >> 30) | 0xfc;
		buf[1] = ((c >> 24) & 0x3f) | 0x80;
		buf[2] = ((c >> 18) & 0x3f) | 0x80;
		buf[3] = ((c >> 12) & 0x3f) | 0x80;
		buf[4] = ((c >>  6) & 0x3f) | 0x80;
		buf[5] = ( c        & 0x3f) | 0x80;
		return 6;
	#endif
	}
	return 0;
}

void MGE::UTF8::appendFromUCS4(std::string& dst, uint32_t c) {
	char buf[6];
	size_t len;
	len = fromUCS4(buf, c);
	dst.append(buf, len);
}
