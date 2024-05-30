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

#include <string>
#include <optional>

namespace MGE {

/// @addtogroup EngineMain
/// @{
/// @file

/**
 * @brief Struct for parse and store command line options and arguments.
 * 
 */
struct CmdLineArgs {
	CmdLineArgs();
	
	/// Parse cmd line options and arguments and fill CmdLineArgs struct. Throw on error.
	bool parse(int argc, char* argv[]);
	
	/// run modes from command line args
	enum RunMode {
		UNSET       =    0,  /// run mode is not set by cmdline, use config value
		SHOW_MENU   =    1,
		LOAD_MAP    =    2,
		LOAD_SAVE   =    3,
		EDIT_SCENE  =    4,
		RUN_SCRIPT  =    5,
	};
	
	/// Loading mode set by cmdline args, see @ref RunMode.
	int loadingMode;
	
	/// Loading path (the meaning depends on value of @ref loadingMode) set by cmdline args.
	std::string loadingFilePath;
	
	/// Initial pause value set by cmdline args.
	std::optional<bool> startPaused;
	
	/// Main config file path, when unset use default
	std::optional<std::string>  mainConfigFilePath;
};

/// @}

}
