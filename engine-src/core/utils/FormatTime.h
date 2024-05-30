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

namespace MGE {

/// @addtogroup StringAndXMLUtils
/// @{
/// @file

/**
 * @brief Functions to getting formatted time strings.
 * 
 * `#include <formatTime.h>`
 */
namespace FormatTime {
	/**
	 * @brief Formatting predefined modes.
	 */
	enum Format {
		/// Unix timestamp.
		UNIX_TIMESTAMP,
		/// Full iso date and time.
		ISO_DATE_AND_TIME,
		/// Full iso time.
		ISO_TIME,
		/// Full iso date and time (like @ref ISO_DATE_AND_TIME), but without colons, dashes and spaces.
		ISO_DATE_AND_TIME_COMPACT,
		/// Full iso time (like @ref ISO_TIME), but without colons, dashes and spaces.
		ISO_TIME_COMPACT,
	};
	
	/**
	 * @brief Return formatted time info.
	 * 
	 * @param mode Determines the type of information (see @ref Format).
	 */
	std::string getTime(Format mode = UNIX_TIMESTAMP);
	
	/**
	 * @brief Return formatted time info.
	 * 
	 * @param format  Format string for time2str.
	 * @param bufSize Size of internal buffer (max length + 1 of results string).
	 */
	std::string getTime(const char* format, size_t bufSize = 128);
	
	/**
	 * @brief Return formated time info.
	 * 
	 * @param time      Pointer to time_t (i.e. results of time()).
	 * @param format    C-string representing format for strftime().
	 * @param timeZone  C-string representing time zone for getting time (NULL when use current time zone).
	 * @param buf       Pointer to buffer to put results string (NULL when don't get string).
	 * @param bufLen    Size of @a buf.
	 * @param tmPtr     Pointer to tm to put time info (NULL when don't get tm struct).
	 * 
	 * @return Length of string in buf (including NULL end char).
	 */
	size_t time2str(
		const time_t* time, const char* format, const char* timeZone,
		char* buf, size_t bufLen, struct tm* tmPtr
	);
};
/// @}

}
