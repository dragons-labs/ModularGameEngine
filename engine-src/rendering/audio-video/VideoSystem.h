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

#pragma   once

#include "config.h"

#if defined(USE_OGGVIDEO) && defined(USE_OGGSOUND)
#include "rendering/audio-video/AudioSystem.h"
#include "rendering/audio-video/utils/VideoSoundInterface.h"
#endif

#include "BaseClasses.h"

#include <OgreResourceGroupManager.h>
#include <OgrePredefinedControllers.h>
#include <Hlms/Unlit/OgreTextureAnimationController.h>

namespace Ogre {
	class MovableObject;
	class HlmsUnlitDatablock;
	class OgreVideoPlugin;
}

class TheoraVideoClip;


namespace MGE {

/// @addtogroup AudioVideo
/// @{
/// @file

/**
 * @brief Video and animated texture system
 */
class VideoSystem :
	public MGE::Singleton<VideoSystem>,
	public MGE::Module
{
public:
	/**
	 * @brief set video texture for material
	 * 
	 * @param fileName           name of Theora video file
	 * @param materialName       name of material to set video as texture
	 * @param sceneNode          scene node to attach sound emitter
	 * 
	 * @param loopClip           when true loop clip (set "auto restart" on video clip)
	 * 
	 * @param maxVolume          sound maximum attenuation volume (sets the maximum volume level of the sound when closest to the listener)
	 * @param minVolume          sound minimum attenuation volume (sets the minimum volume level of the sound when furthest away from the listener)
	 * @param rolloffFactor      sound rolloff factor (sets the rolloff factor applied to the attenuation of the volume over distance)
	 * @param referenceDistance  sound reference distance (sets the half-volume distance)
	 * @param maxDistance        sound maximum distance
	 * 
	 * @param fileGroup          resource group for search video file
	 * @param materialGroup      resource group for search material
	 */
	static TheoraVideoClip* setVideoTexture(
		const Ogre::String& fileName,
		const Ogre::String& materialName,
		Ogre::SceneNode*    sceneNode,
		bool                loopClip = true,
		float               maxVolume = 0.8,
		float               minVolume = 0,
		float               rolloffFactor = 2.0,
		float               referenceDistance = 80,
		float               maxDistance = 100,
		const Ogre::String& fileGroup     = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		const Ogre::String& materialGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
	);
	
	
	class AnimationSpeedFunctionController : public Ogre::ControllerFunction<Ogre::Real> {
	public:
		AnimationSpeedFunctionController(Ogre::Real _speed = 1.0, bool _clamp = true):
			ControllerFunction(true),
			val(0),
			speed(_speed),
			clamp(_clamp)
		{}
		
		Ogre::Real calculate (Ogre::Real in) override {
			val += in*speed;
			if (clamp && (val > 1.0 || val < -1.0))
				val = 0;
			return val;
		}
		
		void configure(Ogre::Real _speed, bool _clamp) {
			speed = _speed;
			clamp = _clamp;
		}
		
		void reset(Ogre::Real _val) {
			val = _val;
		}
		
	protected:
		Ogre::Real val;
		Ogre::Real speed;
		bool       clamp;
	};
	
	class AnimatedTextureController : public Ogre::Controller<Ogre::Real> {
	public:
		/**
		 * @brief set animation speed for animated texture controller
		 * 
		 * @param speed       speed value
		 * @param clamp       when true work in [-1,1] values range
		 */
		inline void configure(Ogre::Real speed, bool clamp) {
			return static_cast<AnimationSpeedFunctionController*>(getFunction().get())->configure(speed, clamp);
		}
		
		/**
		 * @brief reset animation time to new value
		 * 
		 * @param value       new "time" value
		 */
		inline void reset(Ogre::Real value) {
			return static_cast<AnimationSpeedFunctionController*>(getFunction().get())->reset(value);
		}
		
		/**
		 * @brief return Ogre::TextureAnimationControllerValue for animated texture controller
		 * 
		 * @param controller  animated texture controller
		 */
		inline Ogre::TextureAnimationControllerValue* getAnimationController() {
			return static_cast<Ogre::TextureAnimationControllerValue*>( getDestination().get() );
		}
		
		/// enable/disable rotation, see Ogre::TextureAnimationControllerValue::rotationAnimation
		inline void rotationAnimation(bool rotate) {
			return getAnimationController()->rotationAnimation(rotate);
		}
		
		/// enable/disable scaling, see Ogre::TextureAnimationControllerValue::scaleAnimation
		inline void scaleAnimation(bool scaleU, bool scaleV) {
			return getAnimationController()->scaleAnimation(scaleU, scaleV);
		}
		
		/// enable/disable scrolling, see Ogre::TextureAnimationControllerValue::scrollAnimation
		inline void scrollAnimation(bool translateU, bool translateV) {
			return getAnimationController()->scrollAnimation(translateU, translateV);
		}
		
		/// set tiled animation, see Ogre::TextureAnimationControllerValue::tiledAnimation
		inline void tiledAnimation(Ogre::uint16 numFramesHorizontal, Ogre::uint16 numFramesVertical) {
			return getAnimationController()->tiledAnimation(numFramesHorizontal, numFramesVertical);
		}
	};
	
	/**
	 * @brief prepare animation of texture, create and return animated texture controller
	 * 
	 * @param datablock  HlmsUnlitDatablock datablock to animae texture
	 * @param speed      animation speed value
	 */
	static AnimatedTextureController* setAnimatedTexture(Ogre::HlmsUnlitDatablock* datablock, Ogre::Real speed, bool clamp);
	
	/**
	 * @brief prepare animation of texture, create and return animated texture controller
	 * 
	 * @param movable    movable object to set animation on first datablock (must be HlmsUnlitDatablock)
	 * @param speed      animation speed value
	 */
	static AnimatedTextureController* setAnimatedTexture(Ogre::MovableObject* movable, Ogre::Real speed, bool clamp);
	
	/// constructor
	VideoSystem();
	
protected:
	Ogre::OgreVideoPlugin* videoPlugin;
	
	/// destructor
	~VideoSystem();
};

/// @}

}
