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

#include "ConfigParser.h"

#include "rendering/RenderingSystem.h"
#include "rendering/WindowEventMessage.h"
#include "rendering/utils/RenderQueueGroups.h"
#include "rendering/utils/Decals.h"
#include "rendering/utils/OgreHLMS.h"
#include "rendering/CameraNode.h"

#include "Engine.h"
#include "data/utils/OgreResources.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <OgreRenderQueue.h>
#include <pybind11/gil.h>

//////////////   init and destroy RenderingSystem   //////////////

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_RenderingSystemSyntax \<RenderingSystem\>

@c \<RenderingSystem\> configuration XML node for <b>Ogre 3D graphics engine</b> use next child nodes:
    - `<WindowName>` - name of rendering window
    - `<OgreConfigFile>` - path to .ini style config file for Ogre with graphics settings
    - `<PluginsConfigFile>` - path to .ini style config file for Ogre plugins loading
    - `<HLMS>` - path to Ogre HLMS main directory (passed to MGE::OgreResources::initHLMS)
    - `<LoadingScreen>` with set of @ref XMLNode_ResourcesConfigEntry nodes (as in `<Group>` node from @ref XMLNode_ResourcesConfig), for creating resources group "LoadingScreen" in MGE::AudioVideo::Graphics::createLoadingCamera
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(RenderingSystem) {
	auto ogreModule = new MGE::RenderingSystem(
		xmlNode.child("WindowName").text().as_string(),
		xmlNode.child("PluginsConfigFile").text().as_string(),
		xmlNode.child("OgreConfigFile").text().as_string()
	);
	ogreModule->createLoadingScreen( xmlNode.child("LoadingScreen") );
	
	const_cast<MGE::LoadingContext*>(context)->scnMgr = ogreModule->getLoadingSceneManager();
	
	MGE::OgreHLMS::initHLMS( 
		xmlNode.child("HLMS").text().as_string("resources/Ogre/HLMS")
	);
	MGE::OgreHLMS::loadHLMSCache();
	
	return ogreModule;
}

MGE::RenderingSystem::RenderingSystem(
	const std::string& window_name,
	const std::string& plugin_cfg,
	const std::string& ogre_cfg
) : loadingScreenCamera(0) {
	LOG_HEADER("Initialise Rendering System (Ogre 3D)");
	
	ogreLogger = new MyOgreLogger();
	
	LOG_INFO("RenderingSystem", "Create Ogre root, plugin_cfg=" << plugin_cfg << " ogre_cfg=" << ogre_cfg);
	ogreRoot = new Ogre::Root(0, plugin_cfg, ogre_cfg, "");
	
	if(ogreRoot->restoreConfig() || ogreRoot->showConfigDialog()) {
		renderWindow = ogreRoot->initialise(true, window_name);
	} else {
		throw std::logic_error("Unable load graphics config file ...");
	}
	
	Ogre::WindowEventUtilities::addWindowEventListener(renderWindow, this);
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, MGE::MainLoopListener::GRAPHICS_RENDER);
}

#undef LOG_MODULE_NAME
#define LOG_MODULE_NAME "Loading Screen"

void MGE::RenderingSystem::createLoadingScreen(const pugi::xml_node& xmlNode) {
	LOG_INFO("", "Preparing Loading Screen");
	
	LOG_INFO("Create SceneManager");
	loadingSceneManager = createSceneManager(Ogre::ST_GENERIC, "loadingSceneManager", 1);
	
	if (xmlNode) {
		LOG_INFO("Setup resources");
		MGE::OgreResources::processResourcesEntriesXMLNode("LoadingScreen", xmlNode);
		
		LOG_INFO("Initialise resources");
		Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("LoadingScreen", true);
	} else {
		LOG_WARNING("No <Resources> config for <LoadingScreenGroup> in main config file ... no loading screen");
	}
	
	LOG_INFO("Create camera");
	createLoadingCamera(loadingSceneManager);
}

#undef LOG_MODULE_NAME
#define LOG_MODULE_NAME ""

MGE::RenderingSystem::~RenderingSystem(void) {
	LOG_INFO("Destroy Rendering System");
	
	destroyLoadingSceneManager();
	
	if (renderWindow) {
		LOG_INFO("Destroy Graphics", "free render window");
		Ogre::WindowEventUtilities::removeWindowEventListener(renderWindow, this);
		windowClosed(renderWindow);
	}

	LOG_INFO("Destroy Graphics", "shutdown resource managers");
	Ogre::ResourceGroupManager::getSingleton().shutdownAll();
	
	LOG_INFO("Destroy Graphics", "shutdown ogre");
	ogreRoot->shutdown();
	
	LOG_INFO("Destroy Graphics", "shutdown render system");
	ogreRoot->getRenderSystem()->shutdown();
	
	delete ogreLogger;
}

//////////////   scene manager   //////////////

/**
@page XMLSyntax_Misc

@subsection XMLNode_SceneManagerSyntax SceneManager config syntax

SceneManager configuration node have next attributes:
	- @c name  set name of scene manager
	- @c typeName name of type scene manager to create
	- @c typeMask numeric type mask of scene manager to create (use only when no @c typeName attribute, default Ogre::ST_GENERIC)
	.
	and subnodes:
	- @c \<Shadows\> for configurate shadows, with following attributes:
		- @c farDistance set value for Ogre::SceneManager::setShadowFarDistance
		- @c dirLightExtrusionDistance set value for Ogre::SceneManager::setShadowDirectionalLightExtrusionDistance
		- @c dirLightTextureOffset set value for Ogre::SceneManager::setShadowDirLightTextureOffset
		- @c textureFadeEnd set value for Ogre::SceneManager::setShadowTextureFadeEnd
		- @c textureFadeStart set value for Ogre::SceneManager::setShadowTextureFadeStart
	- @c \<Forward3D\> for configurate shadows, with following attributes:
		- @c width
		- @c height
		- @c numSlices
		- @c lightsPerCell
		- @c decalsPerCell
		- @c cubemapProbesPerCel
		- @c minDistance
		- @c maxDistance
	- @ref XMLNode_Decals for enable and configurate Decals (need @c \<Forward3D\> config too)
*/

Ogre::SceneManager* MGE::RenderingSystem::initSceneManager(const pugi::xml_node& xmlNode) {
	pugi::xml_attribute  xmlAttrib;
	pugi::xml_node       xmlSubNode;
	Ogre::SceneManager*   scnMgr;
	
	if (xmlNode.empty()) {
		LOG_ERROR("Call initSceneManager with no xml config node");
		scnMgr = createSceneManager( Ogre::ST_GENERIC, "standardSM", 0 );
		scnMgr->setForwardClustered( true, 16, 16, 8, 8, 4, 4, 1, 32 );
		return scnMgr;
	}
	
	// create Scene Manager
	
	std::string name = xmlNode.attribute("name").as_string("mainSceneManager");
	
	if ( (xmlAttrib = xmlNode.attribute("typeName")) ) {
		scnMgr = createSceneManager( xmlAttrib.as_string(), name, 0 );
	} else {
		scnMgr = createSceneManager( xmlNode.attribute("typeName").as_int(Ogre::ST_GENERIC), name, 0 );
	}
	
	// configure Scene Manager
	
	xmlSubNode = xmlNode.child("Shadows");
	if (xmlSubNode) {
		if ( (xmlAttrib = xmlSubNode.attribute("farDistance")) )
			scnMgr->setShadowFarDistance( xmlAttrib.as_float() );
		
		if ( (xmlAttrib = xmlSubNode.attribute("dirLightExtrusionDistance")) )
			scnMgr->setShadowDirectionalLightExtrusionDistance( xmlAttrib.as_float() );
		
		if ( (xmlAttrib = xmlSubNode.attribute("dirLightTextureOffset")) )
			scnMgr->setShadowDirLightTextureOffset( xmlAttrib.as_float() );
		
		if ( (xmlAttrib = xmlSubNode.attribute("textureFadeEnd")) )
			scnMgr->setShadowTextureFadeEnd( xmlAttrib.as_float() );
		
		if ( (xmlAttrib = xmlSubNode.attribute("textureFadeStart")) )
			scnMgr->setShadowTextureFadeStart( xmlAttrib.as_float() );
	}
	
	for (int i = 2; i < RenderQueueGroups::STOP_RENDER_QUEUE; i=i+2) {
		scnMgr->getRenderQueue()->setRenderQueueMode(i, Ogre::RenderQueue::FAST);
		scnMgr->getRenderQueue()->setRenderQueueMode(i + 1, Ogre::RenderQueue::V1_FAST);
	}
	
	xmlSubNode = xmlNode.child("Forward3D");
	if (xmlSubNode) {
		scnMgr->setForwardClustered(
			true, 
			xmlSubNode.attribute("width").as_int(16),
			xmlSubNode.attribute("height").as_int(16),
			xmlSubNode.attribute("numSlices").as_int(8),
			xmlSubNode.attribute("lightsPerCell").as_int(8),
			xmlSubNode.attribute("decalsPerCell").as_int(4),
			xmlSubNode.attribute("cubemapProbesPerCel").as_int(4),
			xmlSubNode.attribute("minDistance").as_float(1),
			xmlSubNode.attribute("maxDistance").as_float(32)
		);
	}
	xmlSubNode = xmlNode.child("Decals");
	if (xmlSubNode) {
		new MGE::Decals( xmlSubNode, scnMgr );
	}
	
	return scnMgr;
}

void MGE::RenderingSystem::destroySceneManager(Ogre::SceneManager*& scnMgr) {
	LOG_INFO("Destroy SceneManager: name=" << scnMgr->getName() << " type=" << scnMgr->getTypeName() << " (" << scnMgr << ")" );
	
	LOG_INFO("Destroy SceneManager", "destroy all cameras");
	scnMgr->destroyAllCameras();
	LOG_INFO("Destroy SceneManager", "destroy all light");
	scnMgr->destroyAllLights();
	LOG_INFO("Destroy SceneManager", "destroy all entities");
	scnMgr->destroyAllEntities();
	LOG_INFO("Destroy SceneManager", "destroy all movable objects");
	scnMgr->destroyAllMovableObjects();
	LOG_INFO("Destroy SceneManager", "clear scene");
	scnMgr->clearScene(true);
	LOG_INFO("Destroy SceneManager", "remove decals");
	delete MGE::Decals::getPtr();
	LOG_INFO("Destroy SceneManager", "destroy scene manager");
	ogreRoot->destroySceneManager(scnMgr);
	
	scnMgr = NULL;
}

//////////////   loading time camera and scene manager   //////////////

void MGE::RenderingSystem::createLoadingCamera(Ogre::SceneManager* scnMgr) {
	if (loadingScreenCamera)
		return;
	
	if (!scnMgr)
		scnMgr = loadingSceneManager;
	
	loadingScreenCamera = new MGE::CameraNode("LoadingScreen", scnMgr);
	loadingScreenCamera->setRenderTarget( renderWindow->getTexture(), MGE::VisibilityFlags::DEFAULT_MASK );
	renderOneFrame();
}

void MGE::RenderingSystem::destroyLoadingCamera() {
	delete loadingScreenCamera;
	loadingScreenCamera = NULL;
}

void MGE::RenderingSystem::destroyLoadingSceneManager() {
	destroyLoadingCamera();
	
	if (loadingSceneManager)
		destroySceneManager(loadingSceneManager);
	loadingSceneManager = NULL;
}

//////////////   utils   //////////////

bool MGE::RenderingSystem::update(float gameTimeStep, float realTimeStep) {
	pybind11::gil_scoped_release();
	if (!renderOneFrame())
		MGE::Engine::getPtr()->shutDown();
	return true;
}

void MGE::RenderingSystem::windowClosed(Ogre::Window* rw) {
	if (rw == renderWindow)
		MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::WindowEventMsg(WindowEventMsg::Closed) );
}

void MGE::RenderingSystem::windowResized(Ogre::Window* rw) {
	if (rw == renderWindow)
		MGE::Engine::getPtr()->getMessagesSystem()->sendMessage( MGE::WindowEventMsg(WindowEventMsg::Resized) );
}

//////////////   logger for Ogre   //////////////

#include <OgreLogManager.h>

void MGE::RenderingSystem::MyOgreLogger::messageLogged (
	const Ogre::String& message, Ogre::LogMessageLevel lml,
	bool maskDebug, const Ogre::String& logName, bool& skipThisMessage
) {
	MGE::Log::LogLevel level = MGE::Log::Verbose;
	switch(lml) {
		case Ogre::LML_CRITICAL:
			level = MGE::Log::Error;
			break;
		case Ogre::LML_NORMAL:
			level = MGE::Log::Info;
			break;
		case Ogre::LML_TRIVIAL:
			level = MGE::Log::Debug;
			break;
	}
	MGE_LOG.logMultiLine(message, level, "Ogre3D");
	skipThisMessage = true;
}

MGE::RenderingSystem::MyOgreLogger::MyOgreLogger() {
	LOG_INFO("RenderingSystem", "Create Ogre logger");
	ogreLogManager = new Ogre::LogManager();
	ogreLogManager->createLog("", true, false);
	ogreLogManager->getDefaultLog()->addListener(this);
};

MGE::RenderingSystem::MyOgreLogger::~MyOgreLogger() {
	delete ogreLogManager;
}
