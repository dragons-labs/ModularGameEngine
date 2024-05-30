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

#include "modules/utils/AsioSyn.h"

#include <boost/bind.hpp>
#include <thread>

MGE::AsioSyn::AsioSyn() {
	asioSocket = new boost::asio::ip::tcp::socket(asioIO);
}

MGE::AsioSyn::~AsioSyn() {
	delete asioSocket;
}

void MGE::AsioSyn::asioInit(const std::string& host, const std::string& service, int timeout) {
	boost::asio::deadline_timer timer(asioIO);
	timer.expires_from_now(boost::posix_time::seconds(timeout));
	timer.async_wait(boost::bind(&AsioSyn::throwTimeout, this, _1));
	
	boost::asio::ip::tcp::resolver resolver(asioIO);
	resolver.async_resolve(
		boost::asio::ip::tcp::resolver::query(host, service),
		boost::bind(&AsioSyn::doConnect, this, _1, _2, &timer)
	);
	
	asioIO.run();
	asioIO.restart();
}

void MGE::AsioSyn::doConnect(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator endpointIter, boost::asio::deadline_timer* timer) {
	if (ec)
		throw std::logic_error("resolver error: " + ec.message());
	
	boost::asio::async_connect(
		*asioSocket, endpointIter,
		boost::bind(&AsioSyn::finish, this, _1, 0, timer)
	);
}

void MGE::AsioSyn::finish(const boost::system::error_code& ec, std::size_t transferredBytes, boost::asio::deadline_timer* timer) {
	if (ec)
		throw std::logic_error("error: " + ec.message());
	
	timer->cancel();
}

void MGE::AsioSyn::throwTimeout(const boost::system::error_code& ec) {
	if (ec != boost::asio::error::operation_aborted)
		throw std::logic_error("timeout");
}


std::size_t MGE::AsioSyn::sendData(const char* buffer, std::size_t length, int timeout, bool doPool) {
	std::future<std::size_t> future = boost::asio::async_write(
		*asioSocket, boost::asio::buffer(buffer, length),
		boost::asio::transfer_exactly(length),
		boost::asio::use_future
	);
	
	return timeoutWait(future, timeout, doPool, "timeout in sendData()");
}

std::size_t MGE::AsioSyn::readData(char* buffer, std::size_t length, int timeout, bool doPool) {
	std::future<std::size_t> future = boost::asio::async_read(
		*asioSocket, boost::asio::buffer(buffer, length),
		boost::asio::transfer_exactly(length),
		boost::asio::use_future
	);
	
	return timeoutWait(future, timeout, doPool, "timeout in readData()");
}

std::size_t MGE::AsioSyn::dropData(std::size_t length, int timeout, bool doPool) {
	char* tmpBuf = reinterpret_cast<char*>(malloc(length));
	auto res = readData(tmpBuf, length, timeout, doPool);
	free(tmpBuf);
	return res;
}

std::size_t MGE::AsioSyn::timeoutWait(std::future<std::size_t>& future, int timeout, bool doPool, const char* info) {
	boost::asio::deadline_timer timer(asioIO);
	timer.expires_from_now(boost::posix_time::seconds(timeout));
	timer.async_wait(boost::bind(&AsioSyn::cancelOnSocket, this, _1));
	
	do {
		if (!doPool || !asioIO.poll())
			std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	} while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready);
	
	if (timer.expires_from_now().total_seconds() > 0) {
		timer.cancel();
		return future.get();
	}
	
	throw std::logic_error(info);
}

void MGE::AsioSyn::cancelOnSocket(const boost::system::error_code& ec) {
	if (ec != boost::asio::error::operation_aborted)
		asioSocket->cancel();
}
