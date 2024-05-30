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

#include "BaseClasses.h"
#include "force_inline.h"
#include "StringTypedefs.h"
#include "ListenerSet.h"

#include "LogSystem.h"
#include "ConfigParser.h" // for LoadedModulesSet

namespace MGE { class MessagesSystem; class ScriptsSystem; class StoreRestoreSystem; class LoadingSystem; struct MainLoopListener; }

#include <chrono>

/**
 * <b>Modules Game %Engine</b> main namespace.
 */
namespace MGE {

/// @addtogroup EngineMain
/// @{
/// @file

/**
 * @brief 
 * 
 */
class Engine: public MGE::Singleton<Engine> {
public:
	/**
	 * @brief %Engine runlevels (module creation levels, etc).
	 */
	struct Runlevel { enum { // scoped weak enum
		EngineInit,
		SceneLoad,
	};};
	
	/**
	 * @brief Create and start game engine.
	 * 
	 * @param argc  number of main() arguments
	 * @param argv  main() arguments array
	 */
	static int start(int argc, char* argv[]);
	
	// Destructor.
	~Engine();
	
	/**
	 * @brief Return pointer to log system.
	 */
	FORCE_INLINE MGE::Log* getLogSystem() const {
		return defaultLog;
	}
	
	/**
	 * @brief Return pointer to script system.
	 */
	FORCE_INLINE MGE::ScriptsSystem* getScriptsSystem() const {
		return scriptsSystem;
	}
	
	/**
	 * @brief Return pointer to message system.
	 */
	FORCE_INLINE MGE::MessagesSystem* getMessagesSystem() const {
		return messagesSystem;
	}
	
	/**
	 * @brief Return pointer to config parser.
	 */
	FORCE_INLINE MGE::ConfigParser*  getConfigParser() const {
		return configParser;
	}
	
	/**
	 * @brief Return pointer to config parser.
	 */
	FORCE_INLINE MGE::StoreRestoreSystem*  getStoreRestoreSystem() const {
		return storeRestoreSystem;
	}
	
	/**
	 * @brief Return pointer to (config base) loaded engine module, based on module name.
	 * 
	 * @param name   %Engine module name (name register in ConfigParser::configParserListeners).
	 * 
	 * @return Pointer to module object or NULL when module with @a name not loaded.
	 * 
	 * @note When loaded multiple modules with the same name return pointer to one of them.
	 */
	MGE::Module* getModule(const std::string_view& name);
	
	/**
	 * @brief Return pointer to (config base) loaded engine module, based on module name.
	 * 
	 * @param name   %Engine module name (name register in ConfigParser::configParserListeners).
	 * 
	 * @return Pointer to module object or throw exception when module with @a name not loaded.
	 * 
	 * @note When loaded multiple modules with the same name return pointer to one of them.
	 */
	MGE::Module* getModuleThrow(const std::string_view& name);
	
	/**
	 * @brief Show crash message, write on-crash save and exit.
	 * 
	 * @param errType   String with error type.
	 * @param errMsg    String with error message.
	 */
	[[ noreturn ]] static int crash(const std::string_view& errType, const std::string_view& errMsg);
	
	/**
	 * @brief init engine shutdown
	 * 
	 * @note this function return normally (do not break execution of current code);
	 *       stopping engine is done at the shutdown point in main loop
	 */
	void shutDown();
	
	/**
	 * @brief Set of main loop listeners.
	 * 
	 * Listener should derived from @ref MGE::MainLoopListener.
	 * Key values are not unique and determinate order of listeners execution,
	 * see @ref MGE::MainLoopListener::StandardLevels.
	 */
	ClassPtrListenerSet<MGE::MainLoopListener, int>   mainLoopListeners;
	
	/**
	 * @brief Return time of start last (current for main loop listeners) execution of main loop.
	 */
	std::chrono::time_point<std::chrono::steady_clock> getMainLoopTime() {
		return mainLoopTime;
	}
	
	/**
	 * @brief Return path to directory with executable file.
	 */
	const std::string& getExecutableDir() {
		return executableDir;
	}
	
	/**
	 * @brief Return path to current directory on engine start.
	 */
	const std::string& getWorkingDir() {
		return workingDir;
	}
	
	/**
	 * @brief Register error/crash handlers.
	 * 
	 * @remark Public because useful in sub processes.
	 */
	static void handleCrash();
	
protected:
	friend class LoadingSystem;
	
	/// map of (dynamically / config based) loaded engine modules,
	/// used for get module pointer via @ref getModule and @ref getModuleThrow.
	MGE::ConfigParser::LoadedModulesSet loadedModulesSet;
	
	/// time of last main loop begin, used to calculate time information for @ref MGE::MainLoopListener::update
	std::chrono::time_point<std::chrono::steady_clock> mainLoopTime;
	
	/// path to directory with executable file
	std::string  executableDir;
	
	/// path to current directory on engine start
	std::string  workingDir;
	
private:
	/// Constructor (used by @ref start).
	Engine(const char* argv0);
	
	/// Continue engine initialising (starting) process after create Engine Singleton.
	void init();
	
	/// Executing main loop.
	void run();
	
	/// Core modules pointers. @{
	MGE::ScriptsSystem* scriptsSystem;
	MGE::MessagesSystem* messagesSystem;
	MGE::ConfigParser* configParser;
	MGE::StoreRestoreSystem* storeRestoreSystem;
	/// Also ``MGE::Log* defaultLog;`` -- this is global `extern` variable, declare and used by logSystem.h, defined and allocated by engine.cpp.
	/// @}
	
	/// path to on-crash save file
	std::string onCrashSaveFile;
	
	/// when set to false break main loop execution
	bool isRun;
};

/// @}

}
