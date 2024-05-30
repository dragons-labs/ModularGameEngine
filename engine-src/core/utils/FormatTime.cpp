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

#include "FormatTime.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>

size_t MGE::FormatTime::time2str(
	const time_t* time, const char* format, const char* timeZone,
	char* buf, size_t bufLen, struct tm* tmPtr
) {
	struct tm tmTmp;
	if (!tmPtr) {
		tmPtr = &tmTmp;
	}
	
	#if defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE - 0) >= 200112L
	char oldTZ[64];
	if (timeZone) {
		strncpy(oldTZ, getenv("TZ"), 60);
		setenv("TZ", timeZone, 1);
		tzset();
	}
	#endif
	
	if (buf) {
		#if defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE - 0) >=1
		localtime_r(time, tmPtr);
		bufLen = strftime(buf, bufLen, format, tmPtr);
		#else
		bufLen = strftime(buf, bufLen, format, localtime(time));
		#endif
	} else {
		bufLen = 0;
	}
	
	#if defined _POSIX_C_SOURCE && (_POSIX_C_SOURCE - 0) >= 200112L
	if (timeZone) {
		setenv("TZ", oldTZ, 1);
		tzset();
	}
	#endif
	
	return bufLen;
}

std::string MGE::FormatTime::getTime(const char* format, size_t bufSize) {
	char buf[bufSize];
	
	time_t ts = time(0);
	bufSize = time2str(&ts, format, NULL, buf, bufSize, NULL);
	
	return std::string(buf, bufSize);
}

std::string MGE::FormatTime::getTime(Format mode) {
	switch(mode) {
		case ISO_DATE_AND_TIME:
			return getTime("%Y-%m-%d %H:%M:%S");
		case ISO_TIME:
			return getTime("%H:%M:%S");
		case ISO_DATE_AND_TIME_COMPACT:
			return getTime("%Y%m%d_%H%M%S");
		case ISO_TIME_COMPACT:
			return getTime("%H%M%S");
		case UNIX_TIMESTAMP:
		default:
			return getTime("%s");
	}
}
