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

#include "rendering/CameraSystem.h"

#include "rendering/RenderingSystem.h"
#include "rendering/audio-video/AudioSystem.h"
#include "LogSystem.h"
#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "Engine.h"
#include "ConfigParser.h"

MGE::CameraSystem::CameraSystem() :
	MGE::SaveableToXML<CameraSystem>(101, 501),
	currentCamera(NULL), defaultCamera(NULL)
{
	LOG_HEADER("Create CameraSystem" );
	
	// register main loop / update listener
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER);
}

MGE::CameraSystem::~CameraSystem() {
	LOG_INFO("Destroy CameraSystem");
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	unload();
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_CameraSystem \<CameraSystem\>

@c \<CameraSystem\> is used for setup <b>Camera System</b>. This node do not contain any subnodes nor attributes.

(for configure camera see @ref XMLNode_Camera)
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(CameraSystem) {
	return new MGE::CameraSystem();
}


void MGE::CameraSystem::setCurrentCamera(MGE::CameraNode* newCamera, bool audio) {
	if (newCamera == NULL)
		newCamera = defaultCamera;
	
	if (newCamera == currentCamera)
		return;
	
	#ifdef USE_OGGSOUND
	OgreOggSound::OgreOggSoundManager* soundMgr = OgreOggSound::OgreOggSoundManager::getSingletonPtr();
	if (audio && soundMgr) {
		LOG_INFO("attached audio listener to (new) default camera");
		if (currentCamera) {
			currentCamera->detachObject( soundMgr->getListener() );
		}
		newCamera->attachObject( soundMgr->getListener() );
	}
	#endif
	
	currentCamera = newCamera;
}

bool MGE::CameraSystem::update(float gameTimeStep, float realTimeStep) {
	/// update all camera
	for (auto& iter : allCameraNodes) {
		iter.second->update();
	}
	
	return true;
}


bool MGE::CameraSystem::unload() {
	LOG_INFO("unload CameraSystem");
	
	defaultCamera = currentCamera = NULL;
	
	auto iter = allCameraNodes.begin();
	while (iter!=allCameraNodes.end()) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
		delete (*iter++).second;
	}
	
	allCameraNodes.clear();
	return true;
}

bool MGE::CameraSystem::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("Configure / restore CameraSystem");
	
	std::string_view defaultCameraName = xmlNode.child("default").text().as_string();
	std::string_view currentCameraName = xmlNode.child("current").text().as_string();
	
	for (auto xmlSubNode : xmlNode.child("CameraNodes")) {
		std::string cameraName = xmlSubNode.name();
		if (cameraName.empty())
			continue;
		
		MGE::CameraNode* camera;
		auto cameraIter = allCameraNodes.find(cameraName);
		if (cameraIter != allCameraNodes.end()) {
			camera = cameraIter->second;
		} else {
			camera = new MGE::CameraNode(cameraName, context->scnMgr);
		}
		camera->restoreFromXML(xmlSubNode, context);
		
		if (cameraName == defaultCameraName) {
			defaultCamera = camera;
			if (!defaultCamera->getRenderTarget()) {
				defaultCamera->setRenderTarget( MGE::RenderingSystem::getPtr()->getRenderWindow()->getTexture(), MGE::VisibilityFlags::DEFAULT_MASK, 1 );
			}
		}
		if (cameraName == currentCameraName) {
			setCurrentCamera(camera);
		}
	}
	return true;
}

bool MGE::CameraSystem::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	xmlNode.append_child("default") <<  defaultCamera->getName();
	xmlNode.append_child("current") <<  currentCamera->getName();
	
	auto xmlSubNode = xmlNode.append_child("CameraNodes");
	for (auto cam : allCameraNodes) {
		if (cam.second->getName() != "LoadingScreen")
			cam.second->storeToXML( xmlSubNode.append_child( cam.second->getName().c_str() ), onlyRef );
	}
	return true;
}

