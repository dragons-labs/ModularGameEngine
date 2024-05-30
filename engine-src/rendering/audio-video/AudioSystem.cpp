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

#include "rendering/audio-video/AudioSystem.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "Engine.h"
#include "SceneLoader.h"
#include "rendering/RenderingSystem.h"
#include "data/utils/OgreSceneObjectInfo.h"

#ifdef USE_OGGSOUND
#include "OgreOggSoundRoot.h"
#endif

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(AudioSystem) {
	return new MGE::AudioSystem(xmlNode, context->scnMgr);
}

/**
@page XMLSyntax_MainConfig

@subsection AudioConfig \<AudioSystem\>

@c \<AudioSystem\> configuration XML node for <b>OgreOggSound Audio System</b> use next child nodes:
  - @c \<AudioDevice\>
    - string
    - name of audio device
    - empty (default value) for use default device
  - @c \<MaxSources\>
    - integer number
    - maximum number of sound sources in OgreOggSound module
  - @c \<QueueListSize\>
    - integer number
    - maximum queue size for multithreading
  - @c \<DistanceModel\>
    - the model by which sources attenuate with distance, it can be one of the following string:
      - @c AL_NONE
        - no distance attenuation.
      - @c AL_INVERSE_DISTANCE
        - doubling the distance halves the source gain.
      - @c AL_INVERSE_DISTANCE_CLAMPED
        - doubling the distance halves the source gain.
        - distance calculated is clamped between the reference and max distances
      - @c AL_LINEAR_DISTANCE
        - linear gain scaling between the reference and max distances.
        - default value.
      - @c AL_LINEAR_DISTANCE_CLAMPED
        - linear gain scaling between the reference and max distances.
        - distance calculated is clamped between the reference and max distances
      - @c AL_EXPONENT_DISTANCE
        - exponential gain dropoff.
      - @c AL_EXPONENT_DISTANCE_CLAMPED
        - exponential gain dropoff.
        - distance calculated is clamped between the reference and max distances
*/

MGE::AudioSystem::AudioSystem(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr) {
#ifdef USE_OGGSOUND
	LOG_HEADER("Create OgreOggSound (OpenAL) audio system");
	
	audioPlugin = new OgreOggSound::Root();
	audioPlugin->initialise();
	soundManager = OgreOggSound::OgreOggSoundManager::getSingletonPtr();
	
	audioDevice   = xmlNode.child("AudioDevice").text().as_string();
	maxSources    = xmlNode.child("MaxSources").text().as_int(100);
	queueListSize = xmlNode.child("QueueListSize").text().as_int(100);
	
	std::string/*_view*/ tmpDistanceModel = xmlNode.child("DistanceModel").text().as_string("AL_LINEAR_DISTANCE");
	if (tmpDistanceModel == "AL_NONE")
		distanceModel = AL_NONE;
	else if (tmpDistanceModel == "AL_INVERSE_DISTANCE")
		distanceModel = AL_INVERSE_DISTANCE;
	else if (tmpDistanceModel == "AL_INVERSE_DISTANCE_CLAMPED")
		distanceModel = AL_INVERSE_DISTANCE_CLAMPED;
	else if (tmpDistanceModel == "AL_LINEAR_DISTANCE")
		distanceModel = AL_LINEAR_DISTANCE;
	else if (tmpDistanceModel == "AL_LINEAR_DISTANCE_CLAMPED")
		distanceModel = AL_LINEAR_DISTANCE_CLAMPED;
	else if (tmpDistanceModel == "AL_EXPONENT_DISTANCE")
		distanceModel = AL_EXPONENT_DISTANCE;
	else if (tmpDistanceModel == "AL_EXPONENT_DISTANCE_CLAMPED")
		distanceModel = AL_EXPONENT_DISTANCE_CLAMPED;
	else {
		LOG_WARNING("Invalid value of DistanceModel (" << tmpDistanceModel << ") in XML config. Using default.");
		distanceModel = AL_LINEAR_DISTANCE;
	}
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, POST_RENDER);
	
	if (!scnMgr) {
		throw std::logic_error("We must have SceneManager before init audio system, initialise graphic system first");
	}
	
	soundManager->init(audioDevice, maxSources, queueListSize, scnMgr);
	soundManager->setDistanceModel(distanceModel);
	soundManager->pauseAllSounds();
	
	// register dot scene nodes elements
	MGE::SceneLoader::getPtr()->addSceneNodesCreateListener(
		"sound", reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::AudioSystem::processSoundXMLNode)
	);
#else
	LOG_WARNING("Create **fake** OgreOggSound (OpenAL) audio system -- build without OGGSOUND support.");
	soundManager = NULL;
#endif
}

void MGE::AudioSystem::pauseAllSounds() {
#ifdef USE_OGGSOUND
	LOG_VERBOSE("pauseAllSounds");
	soundManager->pauseAllSounds();
#endif
}

void MGE::AudioSystem::resumeAllPausedSounds() {
#ifdef USE_OGGSOUND
	LOG_VERBOSE("resumeAllPausedSounds");
	soundManager->resumeAllPausedSounds();
#endif
}

bool MGE::AudioSystem::update(float gameTimeStep, float realTimeStep) {
#ifdef USE_OGGSOUND
	soundManager->update(realTimeStep);
	return true;
#endif
}

void MGE::AudioSystem::unsetSceneManager() {/*
#ifdef USE_OGGSOUND
	LOG_INFO("Audio::unsetSceneManager");
	
	OgreOggSound::OgreOggListener* listener = soundManager->getListener();
	
	if (listener) {
		Ogre::SceneManager* scnMgr = listener->getSceneManager();
		LOG_INFO("Destroy listener " << listener << " using scene manager " << scnMgr);
		listener->detachFromParent();
		try {
			listener->getSceneManager()->destroyMovableObject(listener);
		} catch(...) {
			OGRE_DELETE_T(listener, OgreOggListener, Ogre::MEMCATEGORY_GENERAL);
		}
		if (soundManager->getListener()) {
			LOG_ERROR("Fail to destroy OgreOggListener");
		}
	}
#endif
*/}

OgreOggSound::OgreOggListener* MGE::AudioSystem::setSceneManager(Ogre::SceneManager* scnMgr) {
#ifdef USE_OGGSOUND
	LOG_INFO("Audio::setSceneManager");
	
	soundManager->setSceneManager(scnMgr);
	soundManager->createListener();
	OgreOggSound::OgreOggListener* listener = soundManager->getListener();
	if (! listener) {
		LOG_ERROR("Fail to create OgreOggListener");
	}
	
	return listener;
#else
	return NULL;
#endif
}

OgreOggSound::OgreOggISound* MGE::AudioSystem::createSound(
	const Ogre::String& name, const Ogre::String& fileName,
	bool loop, bool temporary, bool stream, bool preBuffer, bool immediately, Ogre::SceneManager* scnMgr
) {
#ifdef USE_OGGSOUND
	OgreOggSound::OgreOggISound* sound = soundManager->createSound(
		name, fileName,
		stream, loop, preBuffer, scnMgr, immediately
	);
	if (temporary) {
		sound->markTemporary();
	}
	return sound;
#else
	return NULL;
#endif
}

void MGE::AudioSystem::destroySound(OgreOggSound::OgreOggISound* sound) {
#ifdef USE_OGGSOUND
	if (sound)
		soundManager->destroySound(sound);
#endif
}

void MGE::AudioSystem::setSoundAsBackground(OgreOggSound::OgreOggISound* sound, float volume) {
#ifdef USE_OGGSOUND
	sound->disable3D(true);
	sound->setVolume(volume);
#endif
}

void MGE::AudioSystem::setSoundAs3D(
	OgreOggSound::OgreOggISound* sound, Ogre::SceneNode* parentNode,
	float rolloffFactor, float referenceDistance, float maxDistance, float maxVolume, float minVolume
) {
#ifdef USE_OGGSOUND
	sound->disable3D(false);
	sound->setVolume(maxVolume);
	
	parentNode->attachObject(sound);
	
	sound->setRolloffFactor(rolloffFactor);
	sound->setReferenceDistance(referenceDistance);
	sound->setMaxDistance(maxDistance);
	sound->setMaxVolume(maxVolume);
	sound->setMinVolume(minVolume);
#endif
}

void MGE::AudioSystem::set3DSoundAsDirectional(
	OgreOggSound::OgreOggISound* sound,
	float coneInsideAngle, float coneOutsideAngle, float outerConeVolume
) {
#ifdef USE_OGGSOUND
	sound->setConeAngles(coneInsideAngle, coneOutsideAngle);
	sound->setOuterConeVolume(outerConeVolume);
#endif
}

MGE::AudioSystem::~AudioSystem(void) {
#ifdef USE_OGGSOUND
	LOG_INFO("Destroy Audio");
	
	soundManager->stopAllSounds();
	soundManager->destroyAllSounds();
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	MGE::SceneLoader::getPtr()->remSceneNodesCreateListener(
		reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::AudioSystem::processSoundXMLNode)
	);
	
	audioPlugin->shutdown();
	delete audioPlugin;
#endif
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Sound \<sound\>

@c \<sound\> is used for creating sound, should be subnode of @ref XMLNode_Node and has the following attributes:
  - for all type of sounds:
    - @c name
      - name of created Ogre object (need for Ogre \<2.0)
    - @c filename
      - name of sound file to play (file path will be get via resources system)
    - @c loop
      - @ref XML_Bool
      - default true
    - @c stream
      - @ref XML_Bool
      - default false
    - @c preBuffer
      - @ref XML_Bool
      - default true
    - @c immediate
      - @ref XML_Bool
      - default false
    - @c autoPlay
      - automatically start play after finish scene loading
    - @c isBackgroundSound
      - @ref XML_Bool
      - default false
      - set to true for non-3D (background) sound
  - for non-3D (background) sound
    - @c volume
      - volume level for non-3D (background) sound
  - for 3D sound:
    - @c rolloffFactor
    - @c referenceDistance
    - @c maxDistance
    - @c maxVolume
    - @c minVolume
    - @c isDirectionalSound
      - @ref XML_Bool
      - default false
  - for directional 3D sound:
    - @c insideAngle
    - @c outsideAngle
    - @c outerConeVolume
  .
*/

OgreOggSound::OgreOggISound* MGE::AudioSystem::processSoundXMLNodeWithPrefix(
	const pugi::xml_node&       xmlNode,
	const MGE::LoadingContext*  /*context*/,
	const MGE::SceneObjectInfo& parent,
	const std::string_view&     namePrefix
) {
#ifdef USE_OGGSOUND
	// Process attributes
	std::string name = namePrefix + xmlNode.attribute("name").as_string();
	std::string filename = xmlNode.attribute("filename").as_string();
	
	if (filename.empty()) {
		LOG_WARNING("Can't create sound without filename");
		return NULL;
	}
	
	if (name.empty()) {
		name = filename;
	}
	name = namePrefix + name;
	
	// Create the sound
	OgreOggSound::OgreOggISound* pSound = getPtr()->createSound(
		name,
		filename,
		xmlNode.attribute("loop").as_bool(true),
		false,
		xmlNode.attribute("stream").as_bool(false),
		xmlNode.attribute("preBuffer").as_bool(true),
		xmlNode.attribute("immediate").as_bool(false)
	);
	
	if ( xmlNode.attribute("isBackgroundSound").as_bool(false) || !parent.node ) {
		setSoundAsBackground(
			pSound,
			xmlNode.attribute("volume").as_float(0.5)
		);
	} else {
		setSoundAs3D(
			pSound,
			parent.node,
			xmlNode.attribute("rolloffFactor").as_float(2.0),
			xmlNode.attribute("referenceDistance").as_float(80.0),
			xmlNode.attribute("maxDistance").as_float(100.0),
			xmlNode.attribute("maxVolume").as_float(0.8),
			xmlNode.attribute("minVolume").as_float(0.0)
		);
		
		if ( xmlNode.attribute("isDirectionalSound").as_bool(false) ) {
			float insideAngle  = xmlNode.attribute("insideAngle").as_float(0);
			float outsideAngle = xmlNode.attribute("outsideAngle").as_float(insideAngle);
			set3DSoundAsDirectional(
				pSound,
				insideAngle,
				outsideAngle,
				xmlNode.attribute("outerConeVolume").as_float(0.0)
			);
		}
	}
	
	if ( xmlNode.attribute("autoPlay").as_bool(false) ) {
		getPtr()->getSoundManager()->addSoundToResume(pSound);
		//pSound->play(); will be called internaly by resumeAllPausedSounds()
	}
	
	return pSound;
#else
	return NULL;
#endif
}
