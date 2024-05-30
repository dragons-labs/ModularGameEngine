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

#include "Engine.h"
#include "config.h"
#include "data/LoadingSystem.h"

#include <signal.h>
#include <exception>
#include <OgreException.h>

#include <iostream>

#include <cxxabi.h>
#include <boost/core/demangle.hpp>

#if defined USE_BOOST_STACKTRACE
	#include <boost/stacktrace.hpp>
#elif defined USE_BACKTRACE
	#include <execinfo.h>
#endif

bool onCrash = false;

/// handler for uncaught exception
[[ noreturn ]] void terminate() {
	if (onCrash) abort(); else onCrash = true;
	
	std::string errType, errMsg;
	try {
		throw;
	} catch(Ogre::Exception& e) {
		errType = "Ogre exception";
		errMsg = e.getFullDescription().c_str();
	} catch(std::exception& e) {
		errType = "Std exception";
		errMsg = e.what();
	} catch(...) {
		errType = "Unknown exception";
		errMsg = boost::core::demangle(abi::__cxa_current_exception_type()->name());
	}
	
	MGE::Engine::crash(errType, errMsg);
};

/// handler for SIGSEGV signal
[[ noreturn ]] void segmentation_handle(int sig) {
	if (onCrash) abort(); else onCrash = true;
	
	MGE::Engine::crash("Segmentation fault", "");
}

/// handler for SIGABRT signal
[[ noreturn ]] void abort_handle(int sig) {
	if (onCrash) abort(); else onCrash = true;
	
	MGE::Engine::crash("Aborted", "");
}

void MGE::Engine::handleCrash() {
	onCrash = false;
	signal(SIGSEGV, &segmentation_handle);
	signal(SIGABRT, &abort_handle);
	std::set_terminate(&terminate);
}

[[ noreturn ]] /* call abort(); */ int MGE::Engine::crash(const std::string_view& errType, const std::string_view& errMsg) {
	signal(SIGSEGV, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	
	LOG_HEADER("MGE CRASH:  "s + errType + " " + errMsg);
	
	// make crash save
	auto engine = MGE::Engine::getPtr();
	auto loadingSystem = MGE::LoadingSystem::getPtr();
	bool saveCreated = false;
	if ( engine && loadingSystem && !(engine->onCrashSaveFile.empty()) ) {
		MGE_LOG << "Trying write on-crash save" << std::endl;
		saveCreated = loadingSystem->writeSave(engine->onCrashSaveFile);
		MGE_LOG << std::endl;
	}
	
	MGE_LOG.setAddTimeStamp(false);
	
	// write crash info
	std::cerr << "\033[0m\033[1;7;5;33m";
	MGE_LOG << std::endl;
	MGE_LOG << "#########################" << std::endl;
	MGE_LOG << "####    MGE CRASH    ####" << std::endl;
	MGE_LOG << "#########################" << std::endl;
	MGE_LOG << std::endl;
	std::cerr << "\033[0m\033[1;31m";
	MGE_LOG << "  ERROR TYPE:     " << errType << std::endl;
	MGE_LOG << "  ERROR MESSAGE:  " << errMsg << std::endl;
	std::cerr << "\033[0m";
	MGE_LOG << std::endl;
	
	if (errMsg == "Intentional crash from keyboard (via F9).")
		abort();
	
	// write backtrace
	#if defined USE_BOOST_STACKTRACE
		MGE_LOG << "Backtrace:" << std::endl;
		MGE_LOG << boost::stacktrace::stacktrace() << std::endl;
	#elif defined USE_BACKTRACE
		void* array[20];
		int backtraceSize = backtrace(array, 20);
		char **backtraceStr = backtrace_symbols(array, backtraceSize);
		MGE_LOG << "Backtrace (to decode function names use `c++filt` utility):" << std::endl;
		for (int i = 0; i < backtraceSize; i++)
			MGE_LOG << backtraceStr[i] << std::endl;
		MGE_LOG << std::endl;
	#endif
	
	// write addional info
	std::cerr << "\033[0m\033[1;33m";
	MGE_LOG << "To help improve MGE, please report this problem by send:" << std::endl;
	if (saveCreated) {
		MGE_LOG << " - \"" << MGE::Engine::getPtr()->onCrashSaveFile << "\"" << std::endl;
	}
	MGE_LOG << " - \"" << MGE_LOG.getLogFilePath() << "\"" << std::endl;
	MGE_LOG << " - description of recent activities before the problem occurred" << std::endl;
	MGE_LOG << "to dge-bugs@opcode.eu.org" << std::endl;
	std::cerr << "\033[0m";
	
	abort();
}
