/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/LoadingSystem.h"

#include "LogSystem.h"
#include "XmlUtils.h"
#include "with.h"

#include "data/utils/OgreResources.h"
#include "data/LoadingScreen.h"

// core loading related system - call it from here
#include "ConfigParser.h"
#include "SceneLoader.h"
#include "StoreRestoreSystem.h"

// some engine modules are created here, but register inside engine class
#include "Engine.h"

// systems to (re)init - scene manager, pause management, run scripts, ...
#include "rendering/RenderingSystem.h"
#include "rendering/audio-video/AudioSystem.h"
#include "physics/TimeSystem.h"
#include "ScriptsSystem.h"

// factories to create in constructor
#include "data/structs/factories/ActorFactory.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/PrototypeFactory.h"


#ifdef USE_OGGVIDEO
#include <OgreVideoManager.h>
#endif

#include <Compositor/OgreCompositorManager2.h>
#include <OgreRectangle2D.h>
#include <OgreTextureGpuManager.h>

/*--------------------- parse mission / map config ---------------------*/

/**
@page XMLSyntax_MapConfig

@subsection XMLNode_SceneFile \<SceneFile\>

@c \<SceneFile\> is used for set @ref SceneConfigFiles filepath for load scene elements (including terrain) and configure enviroment (eg. colourAmbient, sky). Can be used multiple time and have next attributes:
	- @c path for load .scene file from filesystem path
	.
	or
	- @c name and @c group for load .scene file from Ogre resource system (indicate respectively filename and resource group to search for this file)
	.
	can have also attributes:
	- @c defaultGroup for setting default group for searching elements from .scene file in resource system


@subsection XMLNode_StateFile \<StateFile\>

@c \<StateFile\> is used for set filepath to state file (aka "fake save file" - save file without a specified map config file to restore).
This save file will be apply to current scene (clearScene and loading listeners are not called) and allows use full save syntax in map configuration.
Typically should be used after @ref XMLNode_SceneFile. Can be used multiple time and have next attributes:
	- @c path for load .scene file from filesystem path
	.
	or
	- @c name and @c group for load .scene file from Ogre resource system (indicate respectively filename and resouce group to search for this file)
*/

void MGE::LoadingSystem::loadMapConfig( const std::string& mapConfigFilePath, bool preloadOnly, std::string mainDotSceneFilePath, SceneLoadStates loadType ) {
	LOG_HEADER("Prepare for loading scene from: " + mapConfigFilePath);
	
	// set "in progress" scene status ...
	sceneLoadState = IN_PROGRESS;
	
	pugi::xml_document xmlFile;
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(xmlFile, mapConfigFilePath.c_str(), "Mission");
	
	// show loading screen
	{
		auto xmlNode = xmlRootNode.child("LoadScreen");
		std::string groupName = xmlNode.attribute("group").as_string("LoadingScreen");

		if (xmlNode)
			MGE::OgreResources::processResourcesEntriesXMLNode(groupName, xmlNode);
		
		if (loadingScreen) {
			loadingScreen->setLoadingScreenImage(
				xmlNode.attribute("file").as_string("LoadingScreen"),
				groupName
			);
			loadingScreen->showLoadingScreen();
		}
	}
	
	// clear scene
	if (loadingScreen)
		loadingScreen->setLoadingScreenProgress(0.1, "Cleaning ...");
	clearScene();
	
	LOG_HEADER("Loading scene from: " + mapConfigFilePath);
	MGE::ConfigParser::getPtr()->listListeners();
	MGE::SceneLoader::getPtr()->listListeners();
	
	// set "in progress" scene status ... yes, again, due to clearScene
	sceneLoadState = IN_PROGRESS;
	
	if (loadType == EDITOR)
		configFile = mainDotSceneFilePath; // loadDotSceneFile() on this file (via getLoadingFilePath()) will be called in Editor constructor
	else
		configFile = mapConfigFilePath;
	loadingContext.preLoad = preloadOnly;
	loadingContext.linkToXML = false;
	
	if (mainDotSceneFilePath.empty())
		mainDotSceneFilePath = MGE::OgreResources::getResourcePath(xmlRootNode.child("SceneFile"), "MapsConfigs");
	
	LOG_INFO("Configure scene manager and resources using file: " << mainDotSceneFilePath);
	
	// create SceneManager and resources based on dot scene XML file
	{
		pugi::xml_document xmlDotSceneFile;
		auto xmlDotSceneRootNode = MGE::XMLUtils::openXMLFile(xmlDotSceneFile, mainDotSceneFilePath.c_str(), "scene");
		
		// init and configure (shadows, etc) SceneManager
		loadingContext.scnMgr = MGE::RenderingSystem::getPtr()->initSceneManager( xmlDotSceneRootNode.child("sceneManager") );
		
		// reinit audio system, (re)create listener after SceneManager::clearScene() or destroy and create new SceneManager
		WITH_NOT_NULL(MGE::AudioSystem::getPtr())->setSceneManager(loadingContext.scnMgr);
		
		// creating loading camera (need for updating loading screen)
		// it will be destroyed before creating cameras in MGE::CameraSystem::restore()
		MGE::RenderingSystem::getPtr()->createLoadingCamera(loadingContext.scnMgr);
		
		// read resources config from map config and load / initialise resource groups for this map
		auto xmlNode = xmlDotSceneRootNode.child("resources");
		if (xmlNode) {
			if (loadingScreen)
				loadingScreen->setLoadingScreenProgress(0.2, "Loading Scene Resources ...");
			MGE::OgreResources::processResourcesXMLNode(xmlNode);
		} else {
			LOG_ERROR("Not found <resources> node in main " << mainDotSceneFilePath << " file");
		}
	}
	
	LOG_INFO("Create scene elements based on map file: " << mapConfigFilePath);
	
	if (loadingScreen)
		loadingScreen->setLoadingScreenProgress(0.5, "Creating Scene ...");
	
	for (auto xmlNode : xmlRootNode) {
		std::string_view xmlNodeName = xmlNode.name();
		
		if (xmlNodeName == "Name") {
			sceneName = xmlNode.text().as_string();
		} else if (xmlNodeName == "SceneFile") {
			loadingContext.defaultResourceGroup = xmlNode.attribute("defaultGroup").as_string("Map_Scene");
			loadDotSceneFile(
				MGE::OgreResources::getResourcePath(xmlNode, "MapsConfigs"),
				&loadingContext
			);
		} else if (xmlNodeName == "StateFile" && !preloadOnly) {
			loadSave(
				MGE::OgreResources::getResourcePath(xmlNode, "MapsConfigs"),
				false
			);
		} else if (xmlNodeName == "ConfigScripts") {
			loadScripts(xmlNode);
		} else if (!xmlNodeName.empty() && xmlNodeName != "Description" && xmlNodeName != "LoadScreen" && xmlNodeName != "SceneScripts") {
			MGE::ConfigParser::getPtr()->createAndConfigureModules(
				MGE::Engine::getPtr()->loadedModulesSet, xmlNodeName, xmlNode, &loadingContext, MGE::Engine::Runlevel::SceneLoad
			);
		}
	}
	
	if (!preloadOnly) {
		for (auto xmlSubNode : xmlRootNode.children("SceneScripts")) {
			loadScripts(xmlSubNode);
		}
		LOG_HEADER("Successfully loaded game from config file: " + mapConfigFilePath);
		finishLoading(loadType);
	}
}


/*--------------------- parse save file ---------------------*/

void MGE::LoadingSystem::loadSave( const std::string& filePath, bool _isRealSaveFile ) {
	pugi::xml_document xmlFile;
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(xmlFile, filePath.c_str(), "SavedState");
	
	if (_isRealSaveFile) {
		configFile = xmlRootNode.child("SceneConfigFile").text().as_string();
		loadMapConfig(configFile, true);
		LOG_HEADER("Loading game from " + filePath + " - load saved data");
	} else {
		LOG_INFO("Loading state from " + filePath);
	}
	
	if (_isRealSaveFile && loadingScreen)
		loadingScreen->setLoadingScreenProgress(0.8, "Restoring ...");
	
	loadingContext.preLoad = false;
	MGE::Engine::getPtr()->getStoreRestoreSystem()->restoreFromXML(xmlRootNode, &loadingContext);
	
	if (_isRealSaveFile) {
		pugi::xml_document xmlFile2;
		auto xmlRootNode2 = MGE::XMLUtils::openXMLFile(xmlFile2, configFile.c_str(), "Mission");
		for (auto xmlSubNode : xmlRootNode2.children("SceneScripts")) {
			loadScripts(xmlSubNode);
		}
		LOG_HEADER("Successfully loaded game from save file: " + filePath);
		finishLoading(GAME);
	}
}


/*--------------------- create scene from .scene file ---------------------*/

void MGE::LoadingSystem::loadDotScene(
		const pugi::xml_node& xmlNode,
		const MGE::LoadingContext* context,
		Ogre::SceneNode* parent
) {
	if (!context)
		context = &loadingContext;
	
	if (!parent)
		parent = context->scnMgr->getRootSceneNode();
	
	for (auto xmlSubNode : xmlNode) {
		std::string_view xmlSubNodeName = xmlSubNode.name();
		
		if (xmlSubNodeName == "nodes") {
			MGE::SceneLoader::getPtr()->parseSceneXMLNode(xmlSubNode, context, {parent, nullptr});
		} else {
			MGE::ConfigParser::getPtr()->createAndConfigureModules(
				MGE::Engine::getPtr()->loadedModulesSet, xmlSubNodeName, xmlSubNode, &loadingContext, MGE::Engine::Runlevel::SceneLoad
			);
		}
	}
}

void MGE::LoadingSystem::loadDotSceneXML(
		const std::string& xmlStr,
		const MGE::LoadingContext* context,
		Ogre::SceneNode* parent
) {
	pugi::xml_document xmlDoc;
	xmlDoc.load_string(xmlStr.c_str());
	loadDotScene(xmlDoc.child("scene"), context, parent);
}

void MGE::LoadingSystem::loadDotSceneFile(
		const std::string& filePath,
		const MGE::LoadingContext* context,
		Ogre::SceneNode* parent,
		pugi::xml_document** xmlDoc
) {
	LOG_INFO("Loading scene from file: " + filePath);
	MGE::ConfigParser::getPtr()->listListeners();
	MGE::SceneLoader::getPtr()->listListeners();
	
	pugi::xml_document* xmlFile = new pugi::xml_document();
	auto xmlRootNode = MGE::XMLUtils::openXMLFile(*xmlFile, filePath.c_str(), "scene");
	
	loadDotScene(xmlRootNode, context, parent);
	
	if (xmlDoc) {
		*xmlDoc = xmlFile;
		editedDotSceneXMLParser = xmlFile;
	} else {
		delete xmlFile;
	}
}


/*--------------------- loading script from mission / map file config entry ---------------------*/

void MGE::LoadingSystem::loadScriptsFromResourceGroup(const Ogre::String& group, const Ogre::String& filter) {
	LOG_INFO("Load python scripts from resource group: " << group << " with filter: " << filter);
	
	Ogre::FileInfoListPtr filesInfo = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo(group, filter); 
	for (auto& fi : *filesInfo) {
		MGE::ScriptsSystem::getPtr()->runFileWithVoid( (fi.archive->getName() + "/" + fi.filename).c_str() );
	}
}


/**
@page XMLSyntax_MapConfig

@subsection XMLNode_Scripts \<ConfigScripts\> and \<SceneScripts\>

@c \<ConfigScripts\> and @c \<SceneScripts\> is used for load and execute scripts (see @ref MGE::ScriptsSystem).
	- Scripts files defined with @c \<ConfigScripts\> will be load (and executed) while reading @ref MapConfig.
	- Scripts files defined with @c \<SceneScripts\> will be load (and executed) **after** load scene and save files.
	.
It have next subnodes (can be used repeatedly):
	- @c \<Group\> for load all scripts from indicated Ogre resource group via @ref loadScriptsFromResourceGroup. It have following attributes:
		- @c name name of resource group to load
		- @c filter optional filter for filenames in this group (default @c *.py for load only files with .py extention)
	- @c \<File\> for run script file via @ref MGE::ScriptsSystem::runFile. It have following attributes:
		- @c name  filename of file to load from Ogre resource system
		- @c group group name to search for this file (default @c Map_Scripts)
*/

void MGE::LoadingSystem::loadScripts(const pugi::xml_node& xmlNode) {
	LOG_INFO("Loading scripts [" << xmlNode.name() << "]");
	for (auto xmlSubNode : xmlNode.children("Group")) {
		std::string groupName       = xmlSubNode.attribute("name").as_string("Scripts");
		std::string fileNameFilter  = xmlSubNode.attribute("filter").as_string("*.py");
		loadScriptsFromResourceGroup(groupName, fileNameFilter);
	}
	for (auto xmlSubNode : xmlNode.children("File")) {
		std::string fileName  = xmlSubNode.attribute("name").as_string();
		std::string fileGroup = xmlSubNode.attribute("group").as_string("Map_Scripts");
		LOG_INFO("run " << fileName << " from " << fileGroup);
		MGE::ScriptsSystem::getPtr()->runFileWithVoid( MGE::OgreResources::getResourcePath(fileName, fileGroup).c_str() );
	}
	LOG_INFO("Scripts executed");
}


/*--------------------- finish loading ---------------------*/

void MGE::LoadingSystem::finishLoading(SceneLoadStates loadType) {
	// call pause() on TimeSystem ... game is paused after load/restore, but TimeSystem may need to show "on screen info"
	if (loadType != EDITOR)
		MGE::TimeSystem::getPtr()->pause(); 
	
	// set scene load state
	sceneLoadState = loadType;
	
	// wait for load resources
	if (loadingScreen)
		loadingScreen->setLoadingScreenProgress(0.9, "Prearing rendering ...");
	Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager()->waitForStreamingCompletion();
	
	// hide loading screen
	if (loadingScreen)
		loadingScreen->hideLoadingScreen();
	
	// render first frame
	MGE::RenderingSystem::getPtr()->renderOneFrame();
	Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager()->waitForStreamingCompletion();
	
	// unpause audio and realtimeTimer (after rendering first frame!)
	WITH_NOT_NULL(MGE::AudioSystem::getPtr())->resumeAllPausedSounds();
	MGE::TimeSystem::getPtr()->realtimeTimer->unpause();
	
	// reset main loop timer (avoid big value of "time from last frame" on first frame)
	MGE::Engine::getPtr()->mainLoopTime = std::chrono::steady_clock::now();
}


/*--------------------- write save ---------------------*/

bool MGE::LoadingSystem::writeSave( const std::string& filePath ) {
	if (sceneLoadState != GAME) {
		LOG_INFO("Not saving game to " + filePath + ". Game is NOT loaded");
		return false;
	}
	
	LOG_INFO("Saving game to " + filePath);
	
	pugi::xml_document xmlDoc;
	auto xmlNode = xmlDoc.append_child("SavedState");
	xmlNode.append_child("SceneConfigFile") << configFile;
	MGE::Engine::getPtr()->getStoreRestoreSystem()->storeToXML(xmlNode);
	bool saveResult = xmlDoc.save_file(filePath.c_str());
	
	LOG_INFO("Saving game result: " << saveResult);
	return saveResult;
}


/*--------------------- clear / unload scene ---------------------*/

void MGE::LoadingSystem::clearScene() {
	LOG_HEADER("Clear Scene");
	
	sceneLoadState = IN_PROGRESS;
	MGE::Engine::getPtr()->getStoreRestoreSystem()->unload();
	#ifdef USE_OGGVIDEO
	static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr())->destroyAllVideoTextures();
	#endif
	WITH_NOT_NULL(MGE::AudioSystem::getPtr())->unsetSceneManager();
	
	if (loadingContext.scnMgr) {
		// loadingContext.scnMgr->clearScene();
		MGE::RenderingSystem::getPtr()->destroySceneManager(loadingContext.scnMgr);
		loadingContext.scnMgr = NULL;
	}
	
	MGE::RenderingSystem::getPtr()->destroyLoadingSceneManager();
	
	// this fix "Renderable wasn't being tracked by this datablock" ogre exception
	// after manipulate "SkyPostprocess" material in MGE::DotSceneLoader::processEnvironment
	Ogre::Renderable* tmp = 0;
	tmp = Ogre::Root::getSingletonPtr()->getCompositorManager2()->getSharedFullscreenQuad();
	if (tmp)
		tmp->_setNullDatablock();
	tmp = Ogre::Root::getSingletonPtr()->getCompositorManager2()->getSharedFullscreenTriangle();
	if (tmp)
		tmp->_setNullDatablock();
	
	/// @todo TODO.5: remove unused resources by call removeUnreferencedResources() on all ResourceManager ... by iteration on Ogre::ResourceGroupManager::getSingleton().getResourceManagerIterator()
	
	sceneLoadState = NO_SCENE;
}


/*--------------------- other LoadingSystem stuff ---------------------*/

/* fake listener to suppress warning from MGE::ConfigParser – node "sceneManager" and "resources" is internal handling in MGE::LoadingSystem::loadMapConfig */
MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(sceneManager) { return reinterpret_cast<MGE::Module*>(1); }
MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(resources) { return reinterpret_cast<MGE::Module*>(1); }

MGE::LoadingSystem::LoadingSystem() :
	sceneLoadState(NO_SCENE), loadingScreen(nullptr)
{
	LOG_HEADER("Create LoadingSystem");
	new MGE::PrototypeFactory();
	new MGE::ActorFactory();
	new MGE::ComponentFactory();
}

MGE::LoadingSystem::~LoadingSystem() {
	LOG_INFO("Destroy LoadingSystem");
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_LoadingSystem \<LoadingSystem\>

@c \<LoadingSystem\> is used for setup <b>Loading System</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(LoadingSystem) {
	return new MGE::LoadingSystem();
}

std::string MGE::LoadingSystem::getSaveName() {
	return sceneName + MGE::TimeSystem::getPtr()->gameTimer->getCounterStr(0, "/%02d.%02d.%02d.xml");
}

/*--------------------- editor related stuff ---------------------*/

void MGE::LoadingSystem::loadEditor( const std::string& _mapFile ) {
	LOG_INFO("Open " << _mapFile << " in editor");
	loadMapConfig(
		MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("EditorPsedoMapConfigFile").text().as_string("./conf/editor.xml"),
		false, _mapFile, EDITOR
	);
}

bool MGE::LoadingSystem::writeScene( const std::string& filePath ) {
	if (sceneLoadState != EDITOR) {
		LOG_ERROR("Not writing scene to " + filePath + ". Scene is NOT loaded in \"editor\" mode");
		return false;
	}
	
	LOG_INFO("Writing scene to: " + filePath);
	bool saveResult = editedDotSceneXMLParser->save_file(filePath.c_str());
	LOG_INFO("Writing game result: " << saveResult);
	return saveResult;
}
