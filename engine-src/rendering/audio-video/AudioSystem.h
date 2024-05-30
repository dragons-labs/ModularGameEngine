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

#include "StringUtils.h"
#include "BaseClasses.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "config.h" // for USE_OGGSOUND

namespace pugi { class xml_node; }

namespace OgreOggSound { class OgreOggSoundManager; class OgreOggListener; class OgreOggISound; class Root; }

#ifdef USE_OGGSOUND
#include <OgreOggSound.h>
#else
typedef int ALenum;
#endif

namespace MGE { struct LoadingContext; struct SceneObjectInfo; }

namespace MGE {

/// @addtogroup AudioVideo
/// @{
/// @file

/**
 * @brief Initializing OgreOggSound audio system (based on OpenAL)
 */
class AudioSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Singleton<AudioSystem>
{
public:
	/**
	 * @brief update 3D sound.
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/**
	 * @brief Pauses all currently playing sounds.
	 */
	void pauseAllSounds();
	
	/**
	 * @brief Resumes all previously playing sounds.
	 */
	void resumeAllPausedSounds();
	
	/**
	 * @brief set SceneManager and recreate listener,
	 *        return new listener (which should be attached to scene node in @a scnMgr)
	 * 
	 * @param     scnMgr   pointer to scene manager
	 */
	OgreOggSound::OgreOggListener* setSceneManager(Ogre::SceneManager* scnMgr);
	
	/**
	 * @brief unset SceneManager and destroy listener,
	 */
	void unsetSceneManager();

	
	/**
	 * @brief return pointer to SoundManager
	 */
	inline OgreOggSound::OgreOggSoundManager* getSoundManager(void) const {
		return soundManager;
	}
	
	/**
	 * @brief create and return sound object
	 * 
	 * @param name        Unique name of sound
	 * @param fileName    Audio file path string
	 * @param loop        Flag indicating if the file should loop.
	 * @param temporary   Flag indicating if the sound is temporary (auto-destroys itself after finishing playing).
	 * @param stream      Flag indicating if the sound sound be streamed.
	 * @param preBuffer   Flag indicating if a source should be attached at creation.
	 * @param immediately Optional flag to indicate creation should occur immediately and not be passed to background thread for queueing.
	 *                    Can be used to overcome the random creation time which might not be acceptable (MULTI-THREADED ONLY)
	 * @param scnMgr      Pointer to SceneManager this sound belongs - 0 defaults to first SceneManager defined.
	 */
	OgreOggSound::OgreOggISound* createSound(
		const Ogre::String& name, const Ogre::String& fileName,
		bool loop = false, bool temporary = false, bool stream = false,
		bool preBuffer = false, bool immediately = false,
		Ogre::SceneManager* scnMgr = NULL
	);
	
	/**
	 * @brief destroy sound object
	 */
	void destroySound(OgreOggSound::OgreOggISound* sound);
	
	/**
	 * @brief configure sound as non 3D (background)
	 * 
	 * @param sound       sound to configure as background
	 * @param volume      volume value
	 */
	static void setSoundAsBackground(OgreOggSound::OgreOggISound* sound, float volume);
	
	/**
	 * @brief configure sound as 3D
	 * 
	 * @param sound              sound to configure as background
	 * @param parentNode         node to attach sound
	 * 
	 * @param rolloffFactor      sound rolloff factor (sets the rolloff factor applied to the attenuation of the volume over distance)
	 * @param referenceDistance  sound reference distance (sets the half-volume distance)
	 * @param maxDistance        sound maximum distance
	 *                           (sets the maximum distance at which attenuation is stopped; beyond this distance the volume remains constant)
	 * @param maxVolume          sound maximum attenuation volume (sets the maximum volume level of the sound when closest to the listener)
	 * @param minVolume          sound minimum attenuation volume (sets the minimum volume level of the sound when furthest away from the listener)
	 */
	static void setSoundAs3D(
		OgreOggSound::OgreOggISound* sound, Ogre::SceneNode* parentNode,
		float rolloffFactor, float referenceDistance, float maxDistance, float maxVolume, float minVolume
	);
	
	/**
	 * @brief configure 3D sound as directional
	 * 
	 * @param sound              sound to configure as directional
	 *
	 * @param coneInsideAngle    sound cone angle over which the volume is at maximum
	 * @param coneOutsideAngle   sound cone angle over which the volume is at minimum 
	 * @param outerConeVolume    sound outer cone volume
	 * 
	 * @note sound direction == parent scene node direction
	 */
	static void set3DSoundAsDirectional(
		OgreOggSound::OgreOggISound* sound,
		float coneInsideAngle, float coneOutsideAngle, float outerConeVolume
	);
	
	/**
	 * @brief create sound based on XML config node
	 * 
	 * like as processSoundXMLNode, but with prefixed sound name by @a namePrefix
	 */
	static OgreOggSound::OgreOggISound* processSoundXMLNodeWithPrefix(
		const pugi::xml_node&       xmlNode,
		const MGE::LoadingContext*  context,
		const MGE::SceneObjectInfo& parent,
		const std::string_view&     namePrefix
	);
	
	/**
	 * @brief create sound based on XML config node
	 * 
	 * Implementation of @ref MGE::SceneLoader::SceneNodesCreateFunction.
	 */
	static OgreOggSound::OgreOggISound* processSoundXMLNode(
		const pugi::xml_node&       xmlNode,
		const MGE::LoadingContext*  context,
		const MGE::SceneObjectInfo& parent
	) {
		return processSoundXMLNodeWithPrefix(xmlNode, context, parent, MGE::EMPTY_STRING_VIEW);
	}
	
	/**
	 * @brief constructor for use in @ref MGE::Singleton::init()
	 * 
	 * @param xmlArchive xml archive object, with pointer to xml node with audio configuration info
	 * @param scnMgr     pointer to scene manager
	 */
	AudioSystem(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr);
	
protected:
	/// destructor
	~AudioSystem(void);
	
	/// @brief pointer to plugin
	OgreOggSound::Root* audioPlugin;
	
	/// @brief pointer to OgreOggSound SoundManager
	/// @note This is singleton, so: soundManager == OgreOggSound::OgreOggSoundManager::getSingletonPtr()
	OgreOggSound::OgreOggSoundManager* soundManager;
	
	/// name of audio device
	std::string audioDevice;
	/// max num of sound sources
	unsigned int maxSources;
	/// max queue for multithreading
	unsigned int queueListSize;
	/// distance model (see AL_DISTANCE_MODEL in al.h)
	ALenum distanceModel;
};

/// @}

}
