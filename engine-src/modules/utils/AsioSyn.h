/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
#include "config.h"

#ifdef TARGET_SYSTEM_IS_WINDOWS
	#include <mingw.future.h>
	#define BOOST_ASIO_HAS_STD_FUTURE
#else
	#include <future>
#endif
#include <utility> // need for asio on g++
#include <boost/asio.hpp>

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Synchronous calls for Boost.Asio with timeout
 */
class AsioSyn {
public:
	/// constructor
	AsioSyn();
	
	/// destructor
	~AsioSyn();
	
	/// init (resolve host, connect, ...) for @a host and @a service with @a timeout
	void asioInit(const std::string& host, const std::string& service, int timeout = 2);
	
	/// send data buffer with timeout
	std::size_t sendData(const char* buffer, std::size_t length, int timeout = 2, bool doPool = true);
	/// read data buffer with timeout
	std::size_t readData(char* buffer, std::size_t length, int timeout = 2, bool doPool = true);
	/// drop input data with timeout
	std::size_t dropData(std::size_t length, int timeout = 2, bool doPool = true);

protected:
	/// boost asio io service object
	boost::asio::io_context       asioIO;
	/// pointer to boost asio socket
	boost::asio::ip::tcp::socket* asioSocket;
	
	/// wait with timeout
	std::size_t timeoutWait(std::future<std::size_t>& future, int timeout, bool doPool, const char* info = "timeout");
	
	/// internal use in asioInit
	void doConnect(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpointIter, boost::asio::deadline_timer* timer);
	/// internal use for doConnect
	void finish(const boost::system::error_code& ec, std::size_t transferredBytes, boost::asio::deadline_timer* timer);
	/// internal use for asioInit
	void throwTimeout(const boost::system::error_code& ec);
	
	/// internal use for futureTimeoutWait
	void cancelOnSocket(const boost::system::error_code& ec);
};

/// @}

}

