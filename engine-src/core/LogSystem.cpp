/*
Copyright (c) 2013-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#ifndef __DOCUMENTATION_GENERATOR__
#ifndef MGE_DEBUG_LEVEL
#define MGE_DEBUG_LEVEL 0 // we don't use LOG_* macros in this file, so suppress warning without include config
#endif
#endif

#include "LogSystem.h"
#include "Utf8.h"

#include <iostream>
#include <iomanip>

int MGE::Log::LogStreamBuf::sync() {
	const auto& str = this->str();
	const auto  len = str.size();
	
	if (! len)
		return 0;
	
	std::string time;
	if (onLineBegin && addTimeStamp) {
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(6) << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - initTime).count() / 1000000.0 << " ";
		time = ss.str();
	}
	
	if (logToFile) {
		if (logFileStream.is_open())
			logFileStream << time << str << std::flush;
		else
			tmpBuf << time << str << std::flush;
	}
	
	if (logToStdErr) {
		std::cerr << time << str << std::flush;
	}
	
	onLineBegin = (str[len-1] == '\n');
	this->str("");
	return 0;
}

MGE::Log::Log(const std::string_view& filename, bool useFile, bool useStdErr, bool addTimeStamp) :
	std::ostream(&logStreamBuffer)
{
	logStreamBuffer.initTime = std::chrono::steady_clock::now();
	logStreamBuffer.onLineBegin = true;
	
	logStreamBuffer.logToFile    = useFile;
	logStreamBuffer.logFilePath  = filename;
	logStreamBuffer.logToStdErr  = useStdErr;
	logStreamBuffer.addTimeStamp = addTimeStamp;
	
	if (! logStreamBuffer.logFilePath.empty()) {
		logStreamBuffer.logFileStream.open( logStreamBuffer.logFilePath.c_str() );
	}
}

void MGE::Log::setFile(const std::string_view& filename) {
	logStreamBuffer.logFileStream.close();
	logStreamBuffer.logToFile = true;
	logStreamBuffer.logFilePath = filename;
	logStreamBuffer.logFileStream.open( logStreamBuffer.logFilePath.c_str() );
	logStreamBuffer.logFileStream << logStreamBuffer.tmpBuf.str() << std::flush;
	logStreamBuffer.tmpBuf.str("");
}

MGE::Log::~Log() {
	logStreamBuffer.logFileStream.close();
	
}


MGE::Log& MGE::Log::logLevel(LogLevel level, const std::string_view& moduleName) {
	logStreamBuffer.sync();
	
	if (logStreamBuffer.onLineBegin) {
		switch(level) {
			case Error:
				*this << "ERROR: ";
				break;
			case Warning:
				*this << "WARNING: ";
				break;
			case Debug:
				*this << "DEBUG: ";
				break;
			default:
				break;
		}
		
		if (moduleName.size()) {
			*this << "[" << moduleName << "] ";
		}
	}
	
	return* this;
}

void MGE::Log::logHeader(const std::string_view& text) {
	int len = MGE::UTF8::getCharsLen(text) + 12;
	*this << std::endl;
	*this << std::string(len, '+') << std::endl;
	*this << "++++  " << text << "  ++++" << std::endl;
	*this << std::string(len, '+') << std::endl;
}

void MGE::Log::logMultiLine(const std::string_view& text, LogLevel level, const std::string_view& moduleName) {
	size_t pos = 0;
	const size_t size = text.size();
	while (pos < size) {
		const size_t begin = pos;
		pos = text.find_first_of( '\n', begin );
		if (pos == std::string_view::npos) {
			logLevel(level, moduleName) << text.substr(begin) << std::endl << std::flush;
		} else {
			logLevel(level, moduleName) << text.substr(begin, (++pos)-begin) << std::flush;
		}
	}
}
