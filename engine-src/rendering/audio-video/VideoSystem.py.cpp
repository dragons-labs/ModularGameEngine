/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "ScriptsInterface.h"

#include "rendering/audio-video/VideoSystem.h"

#ifdef USE_OGGVIDEO
#include <OgreVideoManager.h>
#endif
#include <OgreMovableObject.h>
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	void pauseAllVideoClips() {
		Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		return ovmgr->pauseAllVideoClips();
	}
	
	void unpauseAllVideoClips() {
		Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		return ovmgr->unpauseAllVideoClips();
	}
	
	void destroyVideoTexture1(std::string name) {
		Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		return ovmgr->destroyAdvancedTexture(name);
	}
	
	void destroyVideoTexture2(std::string name, std::string group) {
		Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		return ovmgr->destroyAdvancedTexture(name, group);
	}
	
	TheoraVideoClip* getVideoClipByMaterialName(std::string name) {
		Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		return ovmgr->getVideoClipByMaterialName(name);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(VideoSystem) {
	py::class_<MGE::VideoSystem, std::unique_ptr<MGE::VideoSystem, py::nodelete>>(
		m, "VideoSystem", DOC(MGE, VideoSystem)
	)
		.def_static("pauseAllVideoClips", &pauseAllVideoClips, "pause all video")
		.def_static("unpauseAllVideoClips", &unpauseAllVideoClips, "unpause all video")
		.def_static("destroyVideoTexture", &destroyVideoTexture1, "destroy video texture")
		.def_static("destroyVideoTexture", &destroyVideoTexture2, "destroy video texture")
		.def_static("getVideoClipByMaterialName", &getVideoClipByMaterialName,
			py::return_value_policy::reference, 
			"return video clip based on material name"
		)
		.def_static("setVideoTexture", &MGE::VideoSystem::setVideoTexture,
			py::return_value_policy::reference, 
			DOC(MGE, VideoSystem, setVideoTexture),
			py::arg("fileName"),
			py::arg("materialName"),
			py::arg("sceneNode"),
			py::arg("loopClip") = true,
			py::arg("maxVolume") = 0.8,
			py::arg("minVolume") = 0,
			py::arg("rolloffFactor") = 2.0,
			py::arg("referenceDistance") = 80,
			py::arg("maxDistance") = 100,
			py::arg("fileGroup")     = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			py::arg("materialGroup") = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
		)
		.def_static("setAnimatedTexture",
			py::overload_cast<Ogre::HlmsUnlitDatablock*, Ogre::Real, bool>( MGE::VideoSystem::setAnimatedTexture ),
			py::return_value_policy::reference,
			DOC(MGE, VideoSystem, setAnimatedTexture)
		)
		.def_static("setAnimatedTexture",
			py::overload_cast<Ogre::MovableObject*, Ogre::Real, bool>( MGE::VideoSystem::setAnimatedTexture ),
			py::return_value_policy::reference,
			DOC(MGE, VideoSystem, setAnimatedTexture, 2)
		)
	;
	py::class_<MGE::VideoSystem::AnimatedTextureController>(m, "AnimatedTextureController", DOC(MGE, VideoSystem, AnimatedTextureController))
		.def("configure", &MGE::VideoSystem::AnimatedTextureController::configure,
			DOC(MGE, VideoSystem, AnimatedTextureController, configure)
		)
		.def("reset", &MGE::VideoSystem::AnimatedTextureController::reset,
			DOC(MGE, VideoSystem, AnimatedTextureController, reset)
		)
		.def("rotationAnimation", &MGE::VideoSystem::AnimatedTextureController::rotationAnimation,
			DOC(MGE, VideoSystem, AnimatedTextureController, rotationAnimation)
		)
		.def("scaleAnimation", &MGE::VideoSystem::AnimatedTextureController::scaleAnimation,
			DOC(MGE, VideoSystem, AnimatedTextureController, scaleAnimation)
		)
		.def("scrollAnimation", &MGE::VideoSystem::AnimatedTextureController::scrollAnimation,
			DOC(MGE, VideoSystem, AnimatedTextureController, scrollAnimation)
		)
		.def("tiledAnimation", &MGE::VideoSystem::AnimatedTextureController::tiledAnimation,
			DOC(MGE, VideoSystem, AnimatedTextureController, tiledAnimation)
		)
	;
}
#endif
