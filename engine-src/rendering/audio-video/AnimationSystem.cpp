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

#include "rendering/audio-video/AnimationSystem.h"

#include "LogSystem.h"
#include "XmlUtils.h"
#include "Engine.h"
#include "SceneLoader.h"
#include "ConfigParser.h"
#include "rendering/utils/RenderQueueGroups.h"
#include "data/utils/NamedSceneNodes.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <Animation/OgreSkeletonInstance.h>
#include <OgreParticleSystem.h>


/*--------------------- main loop update ---------------------*/

bool MGE::AnimationSystem::update(float gameTimeStep, float realTimeStep) {
	if (gameTimeStep == 0.0f) // game is paused
		return true;
	
	for (auto iter = v2Animations.begin(); iter != v2Animations.end(); ) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
		auto curr = iter++;
		curr->first->addTime(gameTimeStep * curr->second.speedFactor);
		
		float currTime = curr->first->getCurrentTime();
		if (
		  (curr->second.speedFactor > 0 && currTime >= curr->second.endTime) ||
		  (curr->second.speedFactor < 0 && currTime <= curr->second.endTime)
		) {
			if (curr->second.loopMode == 1) {
				curr->first->setTime(curr->second.initTime);
			} else if (curr->second.loopMode == 2) {
				float tmp = curr->second.endTime;
				curr->second.endTime = curr->second.initTime;
				curr->second.initTime = tmp;
				curr->second.speedFactor = -1 * curr->second.speedFactor;
				curr->first->setTime(curr->second.initTime);
			} else {
				curr->first->setTime(curr->second.endTime);
				//curr->first->setEnabled(false);
				savedAnimations[reinterpret_cast<void*>(curr->first)] = curr->second;
				v2Animations.erase(curr);
			}
		}
	}

	for (auto iter = v1Animations.begin(); iter != v1Animations.end(); ) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
		auto curr = iter++;
		curr->first->addTime(gameTimeStep * curr->second.speedFactor);
		
		float currTime = curr->first->getTimePosition();
		if (
		  (curr->second.speedFactor > 0 && currTime >= curr->second.endTime) ||
		  (curr->second.speedFactor < 0 && currTime <= curr->second.endTime)
		) {
			if (curr->second.loopMode == 1) {
				curr->first->setTimePosition(curr->second.initTime);
			} else if (curr->second.loopMode == 2) {
				float tmp = curr->second.endTime;
				curr->second.endTime = curr->second.initTime;
				curr->second.initTime = tmp;
				curr->second.speedFactor = -1 * curr->second.speedFactor;
				curr->first->setTimePosition(curr->second.initTime);
			} else {
				curr->first->setTimePosition(curr->second.endTime);
				//curr->first->setEnabled(false);
				savedAnimations[reinterpret_cast<void*>(curr->first)] = curr->second;
				v1Animations.erase(curr);
			}
		}
	}
	return true;
}


/*--------------------- constructor/destructor, unload ---------------------*/

MGE::AnimationSystem::AnimationSystem() : MGE::SaveableToXML<AnimationSystem>(302, 402) {
	LOG_HEADER("Create AnimationSystem");
	
	// register "update" listener
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PRE_RENDER);
	
	// register MGE:: dot scene nodes elements
	MGE::SceneLoader::getPtr()->addSceneNodesCreateListener(
		"animations", reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::AnimationSystem::processAnimationXMLNode)
	);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_AnimationSystem \<AnimationSystem\>

@c \<AnimationSystem\> is used for setup <b>Animation System</b>. This node do not contain any subnodes nor attributes.

(for create/add animation use @ref XMLNode_Animation)
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(AnimationSystem) {
	return new MGE::AnimationSystem();
}


MGE::AnimationSystem::~AnimationSystem() {
	LOG_INFO("Destroy AnimationSystem");
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	MGE::SceneLoader::getPtr()->remSceneNodesCreateListener(
		reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::AnimationSystem::processAnimationXMLNode)
	);
}

bool MGE::AnimationSystem::unload() {
	LOG_INFO("unload animations info");
	v1Animations.clear();
	v2Animations.clear();
	savedAnimations.clear();
	return true;
}

/*--------------------- save store/restore ---------------------*/

bool MGE::AnimationSystem::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	LOG_INFO("store animations info");
	
	for (auto& iter : v1Animations) {
		if (iter.second.node) {
			auto xmlSubNode = xmlNode.append_child("animation");
			xmlSubNode.append_child("nodeName")      << iter.second.node->getName();
			xmlSubNode.append_child("animationName") << iter.first->getAnimationName();
			xmlSubNode.append_child("currTime")      << iter.first->getTimePosition();
			xmlSubNode.append_child("loopMode")      << iter.second.loopMode;
			xmlSubNode.append_child("endTime")       << iter.second.endTime;
			xmlSubNode.append_child("speed")         << iter.second.speedFactor;
		}
	}
	for (auto& iter : v2Animations) {
		if (iter.second.node) {
			auto xmlSubNode = xmlNode.append_child("animation");
			xmlSubNode.append_child("nodeName")      << iter.second.node->getName();
			xmlSubNode.append_child("animationName") << iter.second.name;
			xmlSubNode.append_child("currTime")      << iter.first->getCurrentTime();
			xmlSubNode.append_child("loopMode")      << iter.second.loopMode;
			xmlSubNode.append_child("endTime")       << iter.second.endTime;
			xmlSubNode.append_child("speed")         << iter.second.speedFactor;
		}
	}
	for (auto& iter : savedAnimations) {
		if (iter.second.node) {
			auto xmlSubNode = xmlNode.append_child("animation");
			xmlSubNode.append_attribute("finished")  << true;
			xmlSubNode.append_child("nodeName")      << iter.second.node->getName();
			xmlSubNode.append_child("animationName") << iter.second.name;
			xmlSubNode.append_child("currTime")      << iter.second.endTime;
		}
	}
	
	return true;
}

bool MGE::AnimationSystem::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	LOG_INFO("restore animations info");
	
	for (auto xmlSubNode : xmlNode.children("animation")) {
		std::string_view nodeName      = xmlNode.child("nodeName").text().as_string();
		std::string      animationName = xmlNode.child("animationName").text().as_string();
		float            currTime      = xmlNode.child("currTime").text().as_float();
		
		if (xmlSubNode.attribute("finished").as_bool(false)) {
			setAnimation( MGE::NamedSceneNodes::getSceneNode(nodeName), animationName, SET_POSE, currTime, currTime, 0, 0, true );
		} else {
			int   loopMode = xmlNode.child("loopMode").text().as_float();
			float endTime  = xmlNode.child("endTime").text().as_float();
			float speed    = xmlNode.child("speed").text().as_float();
			
			setAnimation( MGE::NamedSceneNodes::getSceneNode(nodeName), animationName, ADD, currTime, endTime, speed, loopMode, true );
		}
	}
	
	return true;
}

/*--------------------- .scene XML node processing ---------------------*/

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Animation \<animations\>

@c \<animations\> is used for defining animations, should be subnode of @ref XMLNode_Item or @ref XMLNode_Entity and has the following subnodes:
  - @c \<animationState\> has the following attributes:
    - @c animationName
    - @c enabled
      - @ref XML_Bool
      - when false do not running this animation, only set pose (based on @c startTime and @c speed when startTime is zero)
      - default true
    - @c startTime
      - default 0 (beginning of animation, or end when animation is reversed)
      - floting point value
    - @c endTime
      - default 0 (end of animation, or beginning when animation is reversed)
      - floting point value
    - @c speed
      - default 1.0
      - when less than 0 reverse animation direction (startTime \> endTime)
      - floting point value (default 1.0)
    - @c loop
      - 0 = no looping
      - 1 = standard looping (default)
      - 2 = reverse direction at end of loop
  - @c \<addionalSkeleton\>
    - add external skeleton to current item, specify skeleton file by name and resource group via attributes: @c fileName and @c groupName
  - @c \<useExternalSkeleton\>
    - use animation (skeleton) from other item attached to this same scene node
    - use attribute @c itemName to provide name of animation source item
    - if occurs it should be the only one subnode of \<animations\> node
*/
void MGE::AnimationSystem::processAnimationXMLNode(
	const pugi::xml_node&       xmlNode,
	const MGE::LoadingContext*  context,
	const MGE::SceneObjectInfo& parent
) {
	auto externalSkeletonSource = xmlNode.child("useExternalSkeleton").attribute("itemName");
	if (externalSkeletonSource) {
		// find item
		// we find only in parent scene node, due to "nature of skeleton sharing"
		// (need this same positions/transforms of parent nodes for both items)
		Ogre::MovableObject* item = parent.node->getAttachedObject( externalSkeletonSource.as_string() );
		
		// set external skeleton
		if (item && item->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			static_cast<Ogre::Item*>(parent.movable)->useSkeletonInstanceFrom( static_cast<Ogre::Item*>(item) );
		} else {
			LOG_WARNING("can't find source for external skeleton");
		}
		
		return;
	}
	
	for (auto xmlSubNode : xmlNode.children("addionalSkeleton")) {
		static_cast<Ogre::Item*>(parent.movable)->getSkeletonInstance()->addAnimationsFromSkeleton(
			xmlSubNode.attribute("fileName").as_string(),
			xmlSubNode.attribute("groupName").as_string()
		);
	}
	
	for (auto xmlSubNode : xmlNode.children("animationState")) {
		bool          enabled   = xmlSubNode.attribute("enabled").as_bool(true);
		std::string   name      = xmlSubNode.attribute("animationName").as_string();
		int           loop      = xmlSubNode.attribute("loop").as_int(1);
		float         startTime = xmlSubNode.attribute("startTime").as_float(0.0);
		float         endTime   = xmlSubNode.attribute("endTime").as_float(0.0);
		float         speed     = xmlSubNode.attribute("speed").as_float(1.0);
		
		if (parent.movable->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			getPtr()->setAnimation(
				static_cast<Ogre::v1::Entity*>(parent.movable), name, enabled ? MGE::AnimationSystem::ADD : MGE::AnimationSystem::SET_POSE, startTime, endTime, speed, loop, false
			);
		} else if (parent.movable->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			getPtr()->setAnimation(
				static_cast<Ogre::Item*>(parent.movable), name, enabled ? MGE::AnimationSystem::ADD : MGE::AnimationSystem::SET_POSE, startTime, endTime, speed, loop, false
			);
		}
	}
}


/*--------------------- setAnimation() ---------------------*/

bool MGE::AnimationSystem::setAnimation(Ogre::SkeletonAnimation* anim, Operation mode, float initTime, float endTime, float speedFactor, int loop, const Ogre::SceneNode* node, const std::string& name) {
	LOG_INFO("setAnimation for SkeletonAnimation initTime=" << initTime << " endTime=" <<  endTime<< " speedFactor=" << speedFactor << " loop=" << loop);
	
	switch(mode) {
		case SET_POSE:
		{
			if (speedFactor < 0 && initTime == 0)
				initTime = anim->getDuration();
			anim->setEnabled(true);
			anim->setTime(initTime);
			anim->addTime(0.01);
			savedAnimations[reinterpret_cast<void*>(anim)] = { node, name, initTime, initTime, speedFactor, loop };
			return true;
		}
		case ADD:
		{
			if (speedFactor > 0 && endTime == 0)
				endTime = anim->getDuration();
			if (speedFactor < 0 && initTime == 0)
				initTime = anim->getDuration();
			anim->setEnabled(true);
			anim->setLoop(loop == 1);
			anim->setTime(initTime);
			v2Animations[anim] = { node, name, initTime, endTime, speedFactor, loop };
			return true;
		}
		case REMOVE:
		{
			anim->setEnabled(false);
			v2Animations.erase(anim);
			savedAnimations.erase(reinterpret_cast<void*>(anim));
			return true;
		}
		default:
			return false;
	}
}

bool MGE::AnimationSystem::setAnimation(const Ogre::Item* item, const std::string& name, Operation mode, float initTime, float endTime, float speedFactor, int loop, bool save) {
	LOG_INFO("setAnimation \"" + name + "\" for Item: " + item->getName());
	
	Ogre::SkeletonInstance*  skeletonInstance = item->getSkeletonInstance();
	if (!skeletonInstance) {
		LOG_WARNING("Item do not have SkeletonInstance");
		return false;
	}
	
	if (mode == REPLACE || mode == REMOVE_ALL) {
		for (auto& iter : skeletonInstance->getAnimations()) {
			if (iter.getEnabled())
				setAnimation(const_cast<Ogre::SkeletonAnimation*>(&iter), REMOVE);
		}
		switch (mode) {
			case REMOVE_ALL:
				return true;
			default:
				mode = ADD;
		}
	}
	
	try {
		Ogre::SkeletonAnimation* anim = skeletonInstance->getAnimation(name);
		return setAnimation(anim, mode, initTime, endTime, speedFactor, loop, save ? item->getParentSceneNode() : NULL, name);
	} catch(Ogre::ItemIdentityException&) {
		LOG_WARNING("Animation \"" + name + "\" not exist");
		return false;
	}
}

bool MGE::AnimationSystem::setAnimation(Ogre::v1::AnimationState* anim, Operation mode, float initTime, float endTime, float speedFactor, int loop, const Ogre::SceneNode* node) {
	LOG_INFO("setAnimation for AnimationState initTime=" << initTime << " endTime=" <<  endTime<< " speedFactor=" << speedFactor << " loop=" << loop);
	
	switch(mode) {
		case SET_POSE:
		{
			if (speedFactor < 0 && initTime == 0)
				initTime = anim->getLength();
			anim->setEnabled(true);
			anim->setTimePosition(initTime);
			anim->addTime(0.01);
			savedAnimations[reinterpret_cast<void*>(anim)] = { node, anim->getAnimationName(), initTime, initTime, speedFactor, loop };
			return true;
		}
		case ADD:
		{
			if (speedFactor > 0 && endTime == 0)
				endTime = anim->getLength();
			if (speedFactor < 0 && initTime == 0)
				initTime = anim->getLength();
			anim->setEnabled(true);
			anim->setLoop(loop == 1);
			anim->setTimePosition(initTime);
			v1Animations[anim] = { node, anim->getAnimationName(), initTime, endTime, speedFactor, loop };
			return true;
		}
		case REMOVE:
		{
			anim->setEnabled(false);
			v1Animations.erase(anim);
			savedAnimations.erase(reinterpret_cast<void*>(anim));
			return true;
		}
		default:
			return false;
	}
}

bool MGE::AnimationSystem::setAnimation(const Ogre::v1::Entity* entity, const std::string& name, Operation mode, float initTime, float endTime, float speedFactor, int loop, bool save) {
	LOG_INFO("setAnimation \"" + name + "\" for Entity: " + entity->getName());
	
	if ((mode == REPLACE || mode == REMOVE_ALL) && entity->getAllAnimationStates()) {
		auto iter = entity->getAllAnimationStates()->getEnabledAnimationStateIterator();
		while(iter.hasMoreElements()) {
			setAnimation(iter.getNext(), REMOVE);
		}
		switch (mode) {
			case REMOVE_ALL:
				return true;
			default:
				mode = ADD;
		}
	}
	
	try {
		Ogre::v1::AnimationState* state = entity->getAnimationState(name);
		return setAnimation(state, mode, initTime, endTime, speedFactor, loop, save ? entity->getParentSceneNode() : NULL);
	} catch(Ogre::ItemIdentityException&) {
		LOG_WARNING("Animation \"" + name + "\" not exist");
		return false;
	}
}

bool MGE::AnimationSystem::setAnimation(const Ogre::Node* node, const std::string& name, Operation mode, float initTime, float endTime, float speedFactor, int loop, bool save) {
	LOG_INFO("setAnimation \"" + name + "\" for Node: " + node->getName());
	
	auto objIter   = static_cast<const Ogre::SceneNode*>(node)->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	bool retVal    = false;
	
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		
		if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME)
			retVal = retVal || setAnimation(static_cast<Ogre::Item*>(m), name, mode, initTime, endTime, speedFactor, loop, save);
		else if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME)
			retVal = retVal || setAnimation(static_cast<Ogre::v1::Entity*>(m), name, mode, initTime, endTime, speedFactor, loop, save);
	}
	
	while(childIter.hasMoreElements()) {
		retVal = retVal || setAnimation(childIter.getNext(), name, mode, initTime, endTime, speedFactor, loop, save);
	}
	
	return retVal;
}

Ogre::Real MGE::AnimationSystem::getAnimationTime(const Ogre::SceneNode* node, const std::string& name) {
	auto objIter   = static_cast<const Ogre::SceneNode*>(node)->getAttachedObjectIterator();
	auto childIter = node->getChildIterator();
	while(objIter.hasMoreElements()) {
		Ogre::MovableObject* m = objIter.getNext();
		
		if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
			Ogre::SkeletonInstance*  si = static_cast<Ogre::Item*>(m)->getSkeletonInstance();
			if (si) {
				try {
					return si->getAnimation(name)->getCurrentTime();
				} catch(Ogre::ItemIdentityException&) {}
			}
		} else if (m->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
			try {
				return static_cast<Ogre::v1::Entity*>(m)->getAnimationState(name)->getTimePosition();
			} catch(Ogre::ItemIdentityException&) {}
		}
	}
	
	while(childIter.hasMoreElements()) {
		return getAnimationTime(static_cast<Ogre::SceneNode*>(childIter.getNext()), name);
	}
	
	return 0.0;
}

/*--------------------- createParticle() ---------------------*/

void MGE::AnimationSystem::createParticle(const std::string& templateName, const std::string& name, Ogre::SceneNode* node) {
	Ogre::ParticleSystem* particleSystem = node->getCreator()->createParticleSystem(
		templateName
	);
	particleSystem->setName(name);
	particleSystem->setRenderQueueGroup(MGE::RenderQueueGroups::DEFAULT_OBJECTS_V1);
	node->attachObject(particleSystem);
}
