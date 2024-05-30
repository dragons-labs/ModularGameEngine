/*
Copyright (c) 2014-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include <OgreTimer.h>
#include <OgreAnimationState.h>
#include <Animation/OgreSkeletonAnimation.h>
#include <OgreItem.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <unordered_map>

namespace pugi { class xml_node; }

namespace MGE { struct LoadingContext; struct SceneObjectInfo; }

namespace MGE {

/// @addtogroup AudioVideo
/// @{
/// @file

/**
 * @brief implement animation update functionality
 */
class AnimationSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Singleton<AnimationSystem>,
	public MGE::SaveableToXML<AnimationSystem>
{
public:
	/// animation operations 
	enum Operation {
		/// add animation
		ADD,
		/// add animation
		SET_POSE,
		/// remove single animation
		REMOVE,
		/// remove all animations on object and and new animation
		REPLACE,
		/// remove all animations on object
		REMOVE_ALL
	};
	
	/**
	 * @brief set (add or remove) animation to system
	 * 
	 * @param     anim         pointer to skeleton animation
	 * @param[in] mode         operation mode
	 * @param[in] loop         if 1 animation will be looped, if 2 animation will be looped with reversing direction after each loop
	 * @param[in] initTime     run animation in @a initTime position (default 0)
	 * @param[in] endTime      when not equal 0 end animation in @a endTime position (default 0 =\> end at final position)
	 * @param[in] speedFactor  speed factor for animation (default 1.0 =\> no speed change)
	 * @param[in] node         pointer to scene node owned this animation (if NULL animation will not be saved)
	 * 
	 * @return true on success, false otherwise
	 */
	bool setAnimation(Ogre::SkeletonAnimation* anim, Operation mode, float initTime = 0, float endTime = 0, float speedFactor = 1, int loop = 1, const Ogre::SceneNode* node = NULL, const std::string& name = MGE::EMPTY_STRING);
	
	/**
	 * @brief set (add or remove) animation to system
	 * 
	 * @param[in] item         pointer to item on which to be run animation
	 * @param[in] name         animation name
	 * @param[in] mode         operation mode
	 * @param[in] loop         if 1 animation will be looped, if 2 animation will be looped with reversing direction after each loop
	 * @param[in] initTime     run animation in @a initTime position (default 0)
	 * @param[in] endTime      when not equal 0 end animation in @a endTime position (default 0 =\> end at final position)
	 * @param[in] speedFactor  speed factor for animation (default 1.0 =\> no speed change)
	 * @param[in] save         save state this animation when savin game
	 *                         (should be true for dymaic in game initialized animations and false for scene animation)
	 * 
	 * @return true on success, false otherwise
	 */
	bool setAnimation(const Ogre::Item* item, const std::string& name, Operation mode, float initTime = 0, float endTime = 0, float speedFactor = 1, int loop = 1, bool save = true);
	
	/**
	 * @brief set (add or remove) animation to system
	 * 
	 * @param     anim         pointer to animation state
	 * @param[in] mode         operation mode
	 * @param[in] loop         if 1 animation will be looped, if 2 animation will be looped with reversing direction after each loop
	 * @param[in] initTime     run animation in @a initTime position (default 0)
	 * @param[in] endTime      when not equal 0 end animation in @a endTime position (default 0 =\> end at final position)
	 * @param[in] speedFactor  speed factor for animation (default 1.0 =\> no speed change)
	 * @param[in] node         pointer to scene node owned this animation (if NULL animation will not be saved)
	 * 
	 * @return true on success, false otherwise
	 */
	bool setAnimation(Ogre::v1::AnimationState* anim, Operation mode, float initTime = 0, float endTime = 0, float speedFactor = 1, int loop = 1, const Ogre::SceneNode* node = NULL);
	
	/**
	 * @brief set (add or remove) animation to system
	 * 
	 * @param[in] entity       pointer to entity on which to be run animation
	 * @param[in] name         animation name
	 * @param[in] mode         operation mode
	 * @param[in] loop         if 1 animation will be looped, if 2 animation will be looped with reversing direction after each loop
	 * @param[in] initTime     run animation in @a initTime position (default 0)
	 * @param[in] endTime      when not equal 0 end animation in @a endTime position (default 0 =\> end at final position)
	 * @param[in] speedFactor  speed factor for animation (default 1.0 =\> no speed change)
	 * @param[in] save         save state this animation when saving game
	 *                         (should be true for dymaic in game initialized animations and false for scene animation)
	 * 
	 * @return true on success, false otherwise
	 */
	bool setAnimation(const Ogre::v1::Entity* entity, const std::string& name, Operation mode, float initTime = 0, float endTime = 0, float speedFactor = 1, int loop = 1, bool save = true);
	
	/**
	 * @brief set (add or remove) animation to system
	 * 
	 * @param[in] node         pointer to scene node on which to be run animation
	 * @param[in] name         animation name
	 * @param[in] mode         operation mode
	 * @param[in] loop         if 1 animation will be looped, if 2 animation will be looped with reversing direction after each loop
	 * @param[in] initTime     run animation in @a initTime position (default 0)
	 * @param[in] endTime      when not equal 0 end animation in @a endTime position (default 0 =\> end at final position)
	 * @param[in] speedFactor  speed factor for animation (default 1.0 =\> no speed change)
	 * @param[in] save         save state this animation when saving game
	 *                         (should be true for dynamic in game initialized animations and false for scene animation)
	 * 
	 * @return true on success, false otherwise
	 */
	bool setAnimation(const Ogre::Node* node, const std::string& name, Operation mode, float initTime = 0, float endTime = 0, float speedFactor = 1, int loop = 1, bool save = true);
	
	/**
	 * @brief create animation based on XML config node
	 * 
	 * Implementation of @ref MGE::SceneLoader::SceneNodesCreateFunction.
	 */
	static void processAnimationXMLNode(
		const pugi::xml_node&       xmlNode,
		const MGE::LoadingContext*  context,
		const MGE::SceneObjectInfo& parent
	);
	
	/**
	 * @brief return current time position (in seconds) of animation
	 * 
	 * @param[in] node         pointer to scene node with animation
	 * @param[in] name         animation name
	 * 
	 * @note return time position of first find animation by name only
	 */
	static Ogre::Real getAnimationTime(const Ogre::SceneNode* node, const std::string& name);
	
	/**
	 * @brief create particle effect
	 * 
	 * @param templateName  name of particle template to create
	 * @param name          name for create particle system
	 * @param node          node to attach created particle system
	 */
	static void createParticle(const std::string& templateName, const std::string& name, Ogre::SceneNode* node);
	
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "Animations";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override; 
	
	/// constructor
	AnimationSystem();
	
protected:
	/// destructor
	~AnimationSystem();
	
	/// struct with AnimationSystem info about single running animation
	struct AnimationInfo {
		/// when not NULL used to save animation
		const Ogre::SceneNode* node;
		/// name of animation
		std::string name;
		/// end time of animation
		float initTime;
		/// end time of animation
		float endTime;
		/// speed of animation
		float speedFactor;
		/// loop mode
		int loopMode;
	};
	
	/// set of all running animation states for Ogre::v1 animations
	std::unordered_map<Ogre::v1::AnimationState*, AnimationInfo> v1Animations;
	
	/// set of all running animation states for Ogre2 animations
	std::unordered_map<Ogre::SkeletonAnimation*,  AnimationInfo> v2Animations;
	
	/// set with all finished animations to save
	std::unordered_map<void*, AnimationInfo> savedAnimations;
};

/// @}

}
