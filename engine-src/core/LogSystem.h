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

#pragma   once

#include <fstream>
#include <sstream>
#include <chrono>

#include "StringTypedefs.h"

namespace MGE {

/// @addtogroup LogSystem
/// @{
/// @file

/**
 * @brief File and/xor std::err logger.
 * 
 * Log object from this class can be used output stream target: ``logObject << "message" << std::flush``.
 * When do that remember to add @c std::endl or @c std::flush to real write to log
 * (log system don't add new line by itself, but macros LOG_*() add new line at end of log message).
 * 
 * @see
 *   - @ref LOG and other <b>LOG_*</b> macros (defined in @ref logSystem.h) documentation.
 */
class Log final : public std::ostream {
public:
	/**
	 * @brief Log importance level.
	 */
	enum LogLevel {
		/// Error.
		Error,
		/// Warning.
		Warning,
		/// Info (standard, default level).
		Info,
		/// Verbose.
		Verbose,
		/// Debug.
		Debug
	};
	
	/**
	 * @brief Constructor - create log system (open log file for writing, etc).
	 * 
	 * @param filename      Name / path of log file to write (existed file will be overwrite), when empty wait for file to log.
	 * @param useFile       When true log will be write to file.
	 * @param useStdErr     When true log will be print to stderr.
	 * @param addTimeStamp  When true log will be prefixed timestamp (seconds from start log).
	 * 
	 * @remark
	 *       - when both @a useFile and @a useStdErr are true log to file and stderr
	 *       - when @a useFile is true, but @a filename is empty wait
	 */
	Log(const std::string_view& filename = MGE::EMPTY_STRING_VIEW, bool useFile = false, bool useStdErr = true, bool addTimeStamp = true);
	
	/**
	 * @brief Destructor - close log.
	 */
	virtual ~Log();
	
	/**
	 * @brief Generate log verbosity level and module name prefix and log message from stream.
	 * 
	 * @param level       Level to use in prefix.
	 * @param moduleName  Module name to use in prefix.
	 */
	Log& logLevel(LogLevel level = Info, const std::string_view& moduleName = MGE::EMPTY_STRING_VIEW);
	
	/**
	 * @brief Write @a text to log as header.
	 * 
	 * @param text   Header text.
	 */
	void logHeader(const std::string_view& text);
	
	/**
	 * @brief Write multiline @a text to log, prefixed each line.
	 * 
	 * @param text        Text to log.
	 * @param level       Level to use in prefix.
	 * @param moduleName  Module name to use in prefix.
	 */
	void logMultiLine(const std::string_view& text, LogLevel level = Info, const std::string_view& moduleName = MGE::EMPTY_STRING_VIEW);
	
	/**
	 * @brief Set log file path (and enable log to file).
	 * 
	 * @param filename  Name / path of log file to write (existed file will be overwrite), when empty wait for file to log.
	 */
	void setFile(const std::string_view& filename);
	
	/**
	 * @brief Change (set) use file logger option.
	 * 
	 * @param val       When true log to file and stderr.
	 */
	void setUseFile(bool val) {logStreamBuffer.logToFile = val;}
	
	/**
	 * @brief Change (set) use stderr logger option
	 * 
	 * @param val       When true log to file and stderr.
	 */
	void setUseStdErr(bool val) {logStreamBuffer.logToStdErr = val;}
	
	/**
	 * @brief Change (set) time prefixed option.
	 * 
	 * @param val       When true log will be prefixed timestamp (seconds from start log).
	 */
	void setAddTimeStamp(bool val) {logStreamBuffer.addTimeStamp = val;}
	
	/**
	 * @brief Return path to file used by this log
	 */
	const std::string& getLogFilePath() {return logStreamBuffer.logFilePath;}
	
private:
	/**
	 * @brief String stream implementation for log object, derived from std::stringbuf.
	 * 
	 * @remarks we need this to use << operators for write to log
	 */
	struct LogStreamBuf : public std::stringbuf {
		/// function used to write buffer data to file or stderr
		virtual int sync() override;
		
		/// time of creation log system
		std::chrono::time_point<std::chrono::steady_clock> initTime;
		
		/// path to file for writing log
		std::string logFilePath;
		
		/// file output stream for writing log
		std::ofstream logFileStream;
		
		/// temporary buffer to store log when wait to file
		std::ostringstream tmpBuf;
		
		/// true when write log to file
		bool logToFile;
		
		/// true when write log to stderr
		bool logToStdErr;
		
		/// when true write time info to log (seconds from start log)
		bool addTimeStamp;
		
		/// when true we are on beginning of new line
		bool onLineBegin;
	};
	
	/// log stream buffer and std::ostream used to receive stream data
	LogStreamBuf logStreamBuffer;
};
/// @}
}

#ifndef MGE_LOG
	namespace MGE {
	/**
	* @brief Access to default (main/engine) log.
	* 
	* @note
	*         Default log object must be created somewhere else.
	*         Log class nor this header don't create it, even don't allocate space for defaultLog variable.
	* 
	* @ingroup LogSystem
	*/
	extern MGE::Log* defaultLog;
	// MGE::Log& getDefaultLog();
	}

	/**
	* @brief Macro to get default (main/engine) log as object.
	* 
	* @remark
	*         - Allow direct use as object (``LOG << something`` or ``MGE_LOG.setsomeoptions()``).
	*         - Can be override for use LOG_* macros with non default (other or local) log objects.
	* 
	* @ingroup LogSystem
	*/
	#define MGE_LOG  (*MGE::defaultLog)
	// #define MGE_LOG  MGE::getDefaultLog()
#endif

/// @addtogroup LogSystem
/// @{

#include <boost/preprocessor/facilities/overload.hpp>
#ifndef LOG_MODULE_NAME
/// Macro wit default module name, used in LOG_*() macros.
#define LOG_MODULE_NAME ""
#endif

/// Write "header" (framed) text to log.
#define LOG_HEADER(txt)        MGE_LOG.logHeader(txt)

/// Write error message (@a msg) to log from module (@a mod). @{
#define LOG_ERROR(...)           BOOST_PP_OVERLOAD(LOG_ERROR_, __VA_ARGS__)(__VA_ARGS__)
#define LOG_ERROR_2(mod, msg)    MGE_LOG.logLevel( MGE::Log::Error, mod ) << msg << std::endl;
#define LOG_ERROR_1(msg)         LOG_ERROR_2(LOG_MODULE_NAME, msg)
/// @}

/// Write warning message (@a msg) to log from module (@a mod). @{
#define LOG_WARNING(...)         BOOST_PP_OVERLOAD(LOG_WARNING_, __VA_ARGS__)(__VA_ARGS__)
#define LOG_WARNING_2(mod, msg)  MGE_LOG.logLevel( MGE::Log::Warning, mod ) << msg << std::endl;
#define LOG_WARNING_1(msg)       LOG_WARNING_2(LOG_MODULE_NAME, msg)
/// @}

/// Write normal message (@a msg) to log from module (@a mod). @{
#define LOG_INFO(...)            BOOST_PP_OVERLOAD(LOG_INFO_, __VA_ARGS__)(__VA_ARGS__)
#define LOG_INFO_2(mod, msg)     MGE_LOG.logLevel( MGE::Log::Info, mod ) << msg << std::endl;
#define LOG_INFO_1(msg)          LOG_INFO_2(LOG_MODULE_NAME, msg)
/// @}

/// Write verbose message (@a msg) to log from module (@a mod). @{
#define LOG_VERBOSE(...)         BOOST_PP_OVERLOAD(LOG_VERBOSE_, __VA_ARGS__)(__VA_ARGS__)
#define LOG_VERBOSE_2(mod, msg)  MGE_LOG.logLevel( MGE::Log::Verbose, mod ) << msg << std::endl;
#define LOG_VERBOSE_1(msg)       LOG_VERBOSE_2(LOG_MODULE_NAME, msg)
/// @}


#ifndef MGE_DEBUG_LEVEL
	#pragma message "MGE_DEBUG_LEVEL macro not set. Assume full debug log. Set MGE_DEBUG_LEVEL 0 to disable debug log, or 1 to suppress this warning (without disable debug log)."
#endif

/// Write debug level 1 message (@a msg) to log from module (@a mod). @{
#if not defined MGE_DEBUG_LEVEL or MGE_DEBUG_LEVEL > 0
	#define LOG_DEBUG(...)        BOOST_PP_OVERLOAD(LOG_DEBUG_, __VA_ARGS__)(__VA_ARGS__)
	#define LOG_DEBUG_2(mod, msg) MGE_LOG.logLevel( MGE::Log::Debug, mod ) << msg << std::endl;
	#define LOG_DEBUG_1(msg)      LOG_DEBUG_2(LOG_MODULE_NAME, msg)
#else
	#define LOG_DEBUG(...)
#endif
/// @}

/// Write debug level 1 message (@a msg) to log, adding file and line of use at end of the message.
#if not defined MGE_DEBUG_LEVEL or MGE_DEBUG_LEVEL > 0
	#define LOG_XDEBUG(msg)    MGE_LOG.logLevel( MGE::Log::Debug, "" ) << msg << std::noshowbase << std::dec << " at " <<  __FILE__ << ":" << __LINE__ << std::endl;
#else
	#define LOG_XDEBUG(msg)    
#endif

/// @}
