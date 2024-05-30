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

#include "rendering/audio-video/VideoSystem.h"

#include "LogSystem.h"
#include "ConfigParser.h"

#ifdef USE_OGGVIDEO
#include <OgreVideoManager.h>
#include <TheoraVideoManager.h>
#include <TheoraVideoClip.h>
#endif

#include <OgreItem.h>
#include <OgreControllerManager.h>
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>

MGE::VideoSystem::VideoSystem() {
	#ifdef USE_OGGVIDEO
	LOG_HEADER("Create video system");
	videoPlugin = new Ogre::OgreVideoPlugin();
	Ogre::Root::getSingletonPtr()->installPlugin(videoPlugin, nullptr);
	#ifdef USE_OGGSOUND
	Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
	ovmgr->setAudioInterfaceFactory( MGE::VideoSoundInterfaceFactory::getSingletonPtr() );
	#else
	LOG_WARNING("Audio support for video is disabled -- build without OGGSOUND support.");
	#endif
	#else
	LOG_WARNING("Create **fake** video system -- build without OGGVIDEO support.");
	#endif
}

MGE::VideoSystem::~VideoSystem() {
	#ifdef USE_OGGVIDEO
	#ifdef USE_OGGSOUND
	delete Ogre::OgreVideoManager::getSingletonPtr();
	#endif
	Ogre::Root::getSingletonPtr()->uninstallPlugin(videoPlugin);
	delete videoPlugin;
	#endif
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_VideoSystem \<VideoSystem\>

@c \<VideoSystem\> is used for setup <b>Video and Animated Textures System</b>. This node do not contain any subnodes nor attributes.

(for create video clip/animated texture use `setVideoTexture`/`setAnimatedTexture` via script system)
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(VideoSystem) {
	return new MGE::VideoSystem();
}


TheoraVideoClip* MGE::VideoSystem::setVideoTexture(
	const Ogre::String& fileName,
	const Ogre::String& materialName,
	Ogre::SceneNode*    sceneNode,
	bool                loopClip,
	float               maxVolume,
	float               minVolume,
	float               rolloffFactor,
	float               referenceDistance,
	float               maxDistance,
	const Ogre::String& fileGroup,
	const Ogre::String& materialGroup
) {
	#ifdef USE_OGGVIDEO
	LOG_DEBUG("Create video from file: " << fileName << " on material: " << materialName);
	Ogre::OgreVideoManager* ovmgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
	
	TheoraVideoClip* clip = ovmgr->createVideoTexture(fileName, materialName, fileGroup, materialGroup);
	
	#ifdef USE_OGGSOUND
	if (clip->getAudioInterface() && sceneNode) {
		LOG_DEBUG(" - configure audio");
		MGE::AudioSystem::setSoundAs3D(
			static_cast<MGE::VideoSoundInterface*>(clip->getAudioInterface())->ogreOggSoundObj,
			sceneNode, rolloffFactor, referenceDistance, maxDistance, maxVolume, minVolume
		);
	}
	#endif
	
	clip->setAutoRestart(loopClip);
	clip->play();
	
	return clip;
	#endif
}

MGE::VideoSystem::AnimatedTextureController* MGE::VideoSystem::setAnimatedTexture(Ogre::HlmsUnlitDatablock* datablock, Ogre::Real speed, bool clamp) {
	Ogre::HlmsSamplerblock samplerblock;
	samplerblock.setAddressingMode(Ogre::TAM_WRAP);
	datablock->setSamplerblock(0, samplerblock);
	datablock->setEnableAnimationMatrix(0, true);
	
	Ogre::SharedPtr< Ogre::ControllerValue<Ogre::Real> > val(
		OGRE_NEW Ogre::TextureAnimationControllerValue(datablock, 0)
	);
	
	Ogre::SharedPtr< Ogre::ControllerFunction<Ogre::Real> > func(
		OGRE_NEW AnimationSpeedFunctionController(-speed, clamp)
	);
	
	return static_cast<AnimatedTextureController*>(
		Ogre::ControllerManager::getSingleton().createController(
			Ogre::ControllerManager::getSingleton().getFrameTimeSource(), val, func
		)
	);
}

MGE::VideoSystem::AnimatedTextureController* MGE::VideoSystem::setAnimatedTexture(Ogre::MovableObject* movable, Ogre::Real speed, bool clamp) {
	Ogre::HlmsUnlitDatablock* datablock = NULL;
	if (movable->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
		auto item = static_cast<Ogre::Item*>(movable);
		if (item->getNumSubItems() > 0) {
			datablock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(
				item->getSubItem(0)->getDatablock()
			);
		}
	}
	
	if (datablock)
		return MGE::VideoSystem::setAnimatedTexture(datablock, speed, clamp);
	
	LOG_WARNING("can't get HlmsUnlitDatablock from movable, so can't set texture animation");
	return NULL;
}
