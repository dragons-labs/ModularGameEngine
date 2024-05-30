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

#include "CmdLineArgs.h"

#include "config.h"

#include "LogSystem.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>

MGE::CmdLineArgs::CmdLineArgs() :
	loadingMode(UNSET),
	startPaused(std::nullopt),
	mainConfigFilePath(std::nullopt)
{}

bool MGE::CmdLineArgs::parse(int argc, char* argv[]) {
	// declare command line options
	boost::program_options::options_description desc("Supported options");
	desc.add_options()
		("help",        "show help message")
		("config-file", boost::program_options::value<std::string>(), "read main config from \"arg\", default is: " MGE_MAIN_CONFIG_FILE_DEFAULT_PATH)
		("load-save",   boost::program_options::value<std::string>(), "load saved game from \"arg\" file")
		("load-map",    boost::program_options::value<std::string>(), "load new game from \"arg\" map config file")
		("editor",      boost::program_options::value<std::string>(), "load dot scene file from \"arg\" to editor")
		("exec-script", boost::program_options::value<std::string>(), "execute script file from \"arg\"")
		("pause",       "pause game after load")
		("no-pause",    "no pause game after load")
	;
	
	// parse command line options
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	
	if (vm.count("help")) {
		LOG_INFO("print help message");
		std::cout << "\n" << desc << "\n";
		return false;
	}
	
	// get path to config file
	if (vm.count("config-file")) {
		mainConfigFilePath = vm["config-file"].as<std::string>();
		if (! std::filesystem::is_regular_file(std::filesystem::path( mainConfigFilePath.value() ))) {
			mainConfigFilePath = std::nullopt;
			throw std::logic_error("File \"" + mainConfigFilePath.value() + "\" not exist");
		}
	}
	
	// analyze command line options
	if (vm.count("load-save")) {
		loadingMode = LOAD_SAVE;
		loadingFilePath = vm["load-save"].as<std::string>();
	} else if (vm.count("load-map")) {
		loadingMode = LOAD_MAP;
		loadingFilePath = vm["load-map"].as<std::string>();
	} else if (vm.count("editor")) {
		loadingMode = EDIT_SCENE;
		loadingFilePath = vm["editor"].as<std::string>();
	} else if (vm.count("exec-script")) {
		loadingMode = RUN_SCRIPT;
		loadingFilePath = vm["exec-script"].as<std::string>();
	} else {
		loadingMode = SHOW_MENU;
	}
	
	if (!vm.count("pause")) {
		startPaused = true;
	} else if (!vm.count("no-pause")) {
		startPaused = false;
	}
	
	// check exists of file indicated by cmd args
	if (loadingMode == LOAD_SAVE || loadingMode == LOAD_MAP || loadingMode == EDIT_SCENE || loadingMode == RUN_SCRIPT) {
		if (! std::filesystem::is_regular_file(std::filesystem::path( loadingFilePath ))) {
			throw std::logic_error("File \"" + loadingFilePath + "\" not exist");
		}
	}
	
	return true;
}
