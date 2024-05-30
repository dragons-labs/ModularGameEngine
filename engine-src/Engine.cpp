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

#include "Engine.h"
#include "MessagesSystem.h"
#include "ScriptsSystem.h"

#include "config.h"
#include "with.h"

/* ***********     init and start engine     ********** */

#include "utils/FormatTime.h"
#include "CmdLineArgs.h"
#include "ConfigParser.h"
#include "SceneLoader.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include "data/LoadingSystem.h"
#include "physics/TimeSystem.h"
#include "gui/modules/MainMenu.h"

#ifdef MGE_USE_GAMECONTROLER
void createGameControler();
#endif

#include "input/InputSystem.h"
#include "rendering/RenderingSystem.h"
#include <OgreWindowEventUtilities.h>

#if defined( TARGET_SYSTEM_IS_UNIX )
#include <X11/Xlib.h> // for some reasons should be after CEGUI
#endif

#include <filesystem>

int MGE::Engine::start(int argc, char* argv[]) {
	// create log system
	defaultLog = new MGE::Log("", true, true, true);
	
	MGE_LOG << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << std::endl;
	MGE_LOG << "+                       Modular Games Engine                        +" << std::endl;
	MGE_LOG << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+" << std::endl;
	MGE_LOG << "  Engine git revision:  " << ENGINE_GIT_VERSION << std::endl;
	MGE_LOG << "  Engine build time:    " << ENGINE_BUILD_TIME << std::endl;
	MGE_LOG << "  Game initialising on: " << MGE::FormatTime::getTime(MGE::FormatTime::ISO_DATE_AND_TIME) << std::endl << std::endl;
	
	// set crash handlers
	handleCrash();
	
	// parse cmd line args
	int retCode;
	MGE::CmdLineArgs cmdLineArgs;
	try {
		if (cmdLineArgs.parse(argc, argv))
			retCode = -1;
		else
			retCode = 0;
	} catch(std::exception& e) {
		LOG_ERROR("Cmdline args error: " << e.what())
		LOG_INFO(" .. ", "Use --help to see full options description.");
		retCode = 2;
	}
	
	// init main config parser
	MGE::ConfigParser::getPtr()->initMainConfig( cmdLineArgs.mainConfigFilePath.value_or(MGE_MAIN_CONFIG_FILE_DEFAULT_PATH).c_str(), "MGEConfig" );
	
	// update log system
	defaultLog->setFile( MGE::ConfigParser::getPtr()->getMainConfig("LogSystem").child("LogFile").text().as_string(MGE_LOG_FILE_DEFAULT_PATH) );
	
	// exit on request from parseCmdLineArgs or cmd line arg parsing error
	if (retCode >=0) {
		return retCode;
	}
	
	#if defined( TARGET_SYSTEM_IS_UNIX )
	XInitThreads(); // need for X11 clipboard support ... must be called before any other X-system calls ... so add here :-/
	#endif
	
	// create engine (and essential modules - scripts and message systems)
	new Engine(argv[0]);
	
	// continue starting engine
	getPtr()->init();
	
	// set isRun to true
	getPtr()->isRun = true;
	
	// load save, map or show menu
	LOG_HEADER("Prepare for run (dependent on cmd line args)");
	if (cmdLineArgs.loadingMode == MGE::CmdLineArgs::LOAD_SAVE) {
		MGE::LoadingSystem::getPtr()->loadSave(cmdLineArgs.loadingFilePath);
	} else if (cmdLineArgs.loadingMode == MGE::CmdLineArgs::LOAD_MAP) {
		MGE::LoadingSystem::getPtr()->loadMapConfig(cmdLineArgs.loadingFilePath);
	} else if (cmdLineArgs.loadingMode == MGE::CmdLineArgs::EDIT_SCENE) {
		MGE::LoadingSystem::getPtr()->loadEditor(cmdLineArgs.loadingFilePath);
	} else if (cmdLineArgs.loadingMode == MGE::CmdLineArgs::RUN_SCRIPT) {
		MGE::ScriptsSystem::getPtr()->runFileWithVoid(cmdLineArgs.loadingFilePath.c_str());
	} else if (MGE::MainMenu::getPtr()) {
		MGE::MainMenu::getPtr()->show();
	} else {
		LOG_WARNING("No mode set in cmdline args nor loaded main menu module - exiting.");
		return 1;
	}
	
	#ifdef MGE_USE_GAMECONTROLER
	createGameControler();
	#endif
	
	if (!cmdLineArgs.startPaused.value_or(true)) {
		MGE::TimeSystem::getPtr()->unpause();
	}
	
	// start main loop
	getPtr()->run();
	
	return 0;
}

MGE::Engine::Engine(const char* argv0) :
	scriptsSystem( new MGE::ScriptsSystem() ),
	messagesSystem( new MGE::MessagesSystem() ),
	configParser( MGE::ConfigParser::getPtr() ),
	storeRestoreSystem( new MGE::StoreRestoreSystem() ) /* this is core due to unload() registration / execution */
{
	// get working and binary path
	workingDir    = std::filesystem::absolute(".").lexically_normal().generic_string();
	executableDir = std::filesystem::absolute(argv0).parent_path().lexically_normal().generic_string();
	LOG_INFO("Engine", "current working dir = " << workingDir);
	LOG_INFO("Engine", "executable dir = " << executableDir);
	
	// get path to "on-crash" save
	onCrashSaveFile = MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("OnCrashSaveFile").text().as_string("./Crash.xml");
	LOG_INFO("Engine", "on-crash savefile path = " << onCrashSaveFile);
	
	// import MGE Python module => import engine Python script API
	scriptsSystem->getGlobalsDict()["MGE"] = pybind11::module::import("MGE");
	
	// list registered XML tags for parsing config files:
	MGE::ConfigParser::getPtr()->listListeners();
	MGE::SceneLoader::getPtr()->listListeners();
}

void MGE::Engine::init() {
	LOG_HEADER("Creating \"auostart\" modules");
	MGE::LoadingContext context; /* do not put nullptr to createAndConfigureModules, because scene manager will be created and used on some modules */
	configParser->createAndConfigureModules(loadedModulesSet, configParser->getMainConfig("Autostart"), &context, Runlevel::EngineInit);
}

void MGE::Engine::run() {
	LOG_HEADER("Start Rendering via Main Loop");
	
	// reset timer, now is used as source for time from last frame
	mainLoopTime = std::chrono::steady_clock::now();
	
	auto mainMenu = MGE::MainMenu::getPtr();
	
	// main loop
	while(true) {
		// timer update
		auto nowTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> diffTime = nowTime - mainLoopTime;
		mainLoopTime = nowTime;
		
		// calculate TimeSinceLastFrame ...
		float realTimeSinceLastFrame = diffTime.count();
		float gameTimeSinceLastFrame = MGE::TimeSystem::getPtr()->getScaledTime(realTimeSinceLastFrame); // == 0.0f when paused
		
		//LOG_DEBUG(" ======= ::::: ======= ");
		
		Ogre::WindowEventUtilities::messagePump();
		
		if (!isRun || MGE::RenderingSystem::getPtr()->getRenderWindow()->isClosed()) {
			break;
		}
		
		if (! MGE::RenderingSystem::getPtr()->getRenderWindow()->isVisible()) {
			//if (mainMenu) mainMenu->show();
			Ogre::Threads::Sleep( 500 );
			continue;
		}
		
		if (mainMenu && mainMenu->isVisible()) {
			mainLoopListeners.callAll(&MGE::MainLoopListener::updateOnFullPause, realTimeSinceLastFrame);
		} else {
			/// @todo TODO.8: maybe run parallel/multi-thread listeners with the same key value
			mainLoopListeners.callAll(&MGE::MainLoopListener::update, gameTimeSinceLastFrame, realTimeSinceLastFrame);
		}
		
		//std::string FPSInfo =  Ogre::StringConverter::toString(MGE::RenderingSystem::getPtr()->getRenderWindow()->getLastFPS());
		//MGE::OnScreenInfo::getPtr()->showOnScreenText(FPSInfo.c_str(), -1, 333);
	}
	
	LOG_HEADER("End Rendering via Main Loop ... shutting down Engine");
}

void MGE::Engine::shutDown() {
	isRun = false;
}

MGE::Module* MGE::Engine::getModule(const std::string_view& name) {
	auto m = loadedModulesSet.find(name);
	if (m != loadedModulesSet.end())
		return m->ptr;
	else
		return nullptr;
}

MGE::Module* MGE::Engine::getModuleThrow(const std::string_view& name) {
	auto m = loadedModulesSet.find(name);
	if (m != loadedModulesSet.end())
		return m->ptr;
	else
		throw std::runtime_error("Engine module " + name + " is not loaded");
}

MGE::Engine::~Engine() { /// @todo TODO.4: use "destroy" listener registration based approach
	LOG_HEADER("Shutting down engine");
	
	/*  
	MGE::LoadingSystem::getPtr()->clearScene();
	
	delete MGE::UserInterface::Console::getPtr();
	delete MGE::UserInterface::EscMenu::getPtr();
	delete MGE::GUISystem::getPtr();
	delete WITH_NOT_NULL(MGE::Selection::getPtr());
	delete MGE::Physics::Physics::getPtr();
	delete MGE::Audio::getPtr();
	delete MGE::AudioVideo::AnimationSystem::getPtr();
	delete MGE::Physics::TimeSystem::getPtr();
	delete MGE::CameraSystem::getPtr();
	delete MGE::InputSystem::getPtr();
	MGE::LoadingSystem::saveHLMSCache();
	delete MGE::RenderingSystem::getPtr();
	delete MGE::ScriptsSystem::getPtr();
	delete MGE::LoadingSystem::getPtr();
	*/
	
	delete defaultLog;
	defaultLog = nullptr;
}

MGE::Log*           MGE::defaultLog = nullptr;
