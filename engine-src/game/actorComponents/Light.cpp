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

#include "game/actorComponents/Light.h"

#include "LogSystem.h"

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "data/structs/components/3DWorld.h"
#include "data/DotSceneLoader.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "rendering/utils/LightControllers.h"
#include "rendering/utils/RenderQueueGroups.h"

#include <OgreControllerManager.h>
#include <OgrePredefinedControllers.h>
#include <OgreSceneManager.h>
#include <OgreBillboardSet.h>
#include <OgreSceneNode.h>
#include <OgreItem.h>

#include <unordered_set>

struct MGE::Light::LightNode {
	Ogre::SceneNode*                 node;
	Ogre::Controller<Ogre::Real>*    controller;
	Ogre::ControllerFunctionRealPtr  ctrlFun;
	Ogre::ControllerValueRealPtr     ctrlDstVal;
	
	template<class CtrlFun, class CtrlVal> LightNode(
		Ogre::SceneNode*  n,
		CtrlFun           cf,
		CtrlVal           cv
	) :
		node(n), controller(NULL), ctrlFun(cf), ctrlDstVal(cv)
	{
		on();
	}
	
	void off() {
		if (controller) {
			Ogre::ControllerManager::getSingleton().destroyController(controller);
			controller = NULL;
			static_cast<MGE::LightControllerValue*>(ctrlDstVal.get())->off();
		}
	}
	
	void on() {
		if(!controller) {
			controller = Ogre::ControllerManager::getSingleton().createController(
				Ogre::ControllerManager::getSingleton().getFrameTimeSource(), ctrlDstVal, ctrlFun
			);
			static_cast<MGE::LightControllerValue*>(ctrlDstVal.get())->on();
		}
	}
};

inline Ogre::ControllerFunctionRealPtr getScaleControllerFunctions(Ogre::Real speed, std::unordered_map<Ogre::Real, Ogre::ControllerFunctionRealPtr>& list) {
	auto iter = list.find(speed);
	if (iter == list.end()) {
		Ogre::ControllerFunctionRealPtr func(
			OGRE_NEW Ogre::ScaleControllerFunction(speed, true)
		);
		list.insert({speed, func});
		return func;
	} else {
		LOG_DEBUG("use shared ScaleControllerFunction with useCount=" << iter->second.use_count());
		return iter->second;
	}
}



MGE::Light::Light(MGE::NamedObject* parent) : billboardSet(NULL), rootNode(NULL) {}

MGE::Light::~Light() {
	clear();
}

void MGE::Light::clear() {
	std::unordered_set<Ogre::SceneNode*> nodesTmp;
	for (auto& iter1 : lightNodesList) {
		for (auto& iter2 : iter1.second) {
			nodesTmp.insert(iter2.node);
			iter2.off();
		}
	}
	for (auto& iter : nodesTmp) {
		MGE::OgreUtils::recursiveDeleteSceneNode(iter);
	}
	
	if (billboardSet) {
		rootNode->detachObject(billboardSet);
		rootNode->getCreator()->destroyBillboardSet(billboardSet);
	}
	
	billboardSet = NULL;
	lightNodesList.clear();
}

MGE_ACTOR_COMPONENT_DEFAULT_CREATOR(MGE::Light, Light)


/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Light Light

Use subnodes:
  - @c \<sfx\> for configure SFX billboard for flare effects, this node can have attributes:
    - @c material material name for billboard
    - @c size     billboard size
    - @c num      number of initial created billboard in this set
  - @ref XMLNode_Light repeated for each light, with following additional subnodes:
    - @c \<position\>  for set light (child scene node) position (@ref XML_Vector3), each light is always create on own scene node
    - @c \<animation\> for configure light animation, can be used multiple time, have attributes:
      - @c type           type of light animation, can be "rotating", "flashing" or "flashingRandom"
	  - @c group          numeric value of light group ID used to switch on/off lights (default 0)
      - @c speed          speed of light animation
      - @c switchOn       value from range [0,1] when light will be switch on (only for type == "flashing" or "type == flashingRandom")
      - @c switchOff      value from range [@a switchOn,1] when light will be switch off (only for type == "flashing" or "type == flashingRandom", default 1.0)
      - @c randomLimit    in cycles when random number (from range [0,1]) will be greater than this value input value counting to turn on light start from @a randomSetVal instead of 0 (only for type == "flashingRandom")
      - @c randomSetVal   value used instead of 0 for counter init value in cycles when random \> @a randomLimit (only for type == "flashingRandom")
      - @c randomSetLimit maximum number of following one by one using @a randomSetVal as counter init (only for type == "flashingRandom")
    - @ref XMLNode_Item   for creating light object item, with extra attributes:
      - @c newNode        when true (default) creating new node for item (light will be on child node of node with item)
      - Info: If used mesh have pivot inside yourself, light is put inside it too.
              For best effect mesh for "glass" faces should have normal vectors to inside and semi-transparent (with suitable color for light), two side material with cull_mode="none".
    - @c \<scale\> scale for @a item
    - @c \<sfxColour\> to set SFX billboard for flare color and size (this is @ref XML_ColourValue node with one additional and optional attribute @c size to override default billboard size set in @c \<sfx\>)

All non-random animation with this same @a speed value (in single component) will be use the same Ogre::ScaleControllerFunction, so lights will be working synchronous.
If you don't want this you can use different speed or different @a switchOn / @a switchOff (for flashing light), @a direction (for rotating light).
*/
bool MGE::Light::restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {
	// clear before (re)creating
	clear();
	
	rootNode = sceneNode;
	MGE::LoadingContext context(rootNode->getCreator(), false, false);
	
	{
		auto xmlSfxNode = xmlNode.child("sfx");
		if (xmlSfxNode) {
			int num = xmlSfxNode.attribute("num").as_int(4);
			Ogre::Real size = xmlSfxNode.attribute("size").as_float(8);
			auto material = xmlSfxNode.attribute("material").as_string();
			
			billboardSet = context.scnMgr->createBillboardSet( num );
			billboardSet->setAutoextend(true);
			billboardSet->setDefaultDimensions(size, size);
			billboardSet->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OUTLINE_V1);
			billboardSet->Ogre::Renderable::setDatablock(material);
			rootNode->attachObject(billboardSet);
		}
	}
	
	std::unordered_map<Ogre::Real, Ogre::ControllerFunctionRealPtr> scaleControllerFunctions;
	
	
	for (auto xmlLightNode : xmlNode.children("light")) {
		// create scene node for light
		Ogre::SceneNode* lightNode = rootNode->createChildSceneNode();
		Ogre::Vector3    position  = MGE::XMLUtils::getValue<Ogre::Vector3>(xmlLightNode.child("position"), Ogre::Vector3::ZERO);
		lightNode->setPosition( position );
		lightNode->setScale( MGE::XMLUtils::getValue<Ogre::Vector3>(xmlLightNode.child("scale"), Ogre::Vector3::UNIT_SCALE) );
		
		LOG_DEBUG("LightComponent: create light on node " << lightNode);
		
		// (optional) create item
		{
			auto xmlItemSubNode = xmlLightNode.child("item");
			if (xmlItemSubNode) {
				Ogre::SceneNode* itemNode = lightNode;
				if( xmlItemSubNode.attribute("newNode").as_bool(true) ) {
					lightNode = itemNode->createChildSceneNode();
				}
				auto item = MGE::DotSceneLoader::processItem(
					xmlItemSubNode, &context, {itemNode, nullptr}
				);
				item->setRenderQueueGroup(MGE::RenderQueueGroups::STENCIL_GLOW_OBJECT_V2);
			}
		}
		
		// create light
		Ogre::Light* light = MGE::DotSceneLoader::processLight(
			xmlLightNode, &context, {lightNode, nullptr}
		);
		
		// (optional) create SFX bilboard
		Ogre::ColourValue    sfxColour;
		Ogre::v1::Billboard* billboard = NULL;
		{
			auto xmlSfxNode = xmlLightNode.child("sfxColour");
			if (xmlSfxNode && billboardSet) {
				billboard = billboardSet->createBillboard(Ogre::Vector3::ZERO);
				sfxColour = MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlSfxNode);
				billboard->setColour(sfxColour);
				if (xmlSfxNode.attribute("size")) {
					Ogre::Real size = xmlSfxNode.attribute("size").as_float(8);
					billboard->setDimensions(size, size);
				}
				billboard->setPosition(position);
			}
		}
		
		// (optional) set animation of light and (if created) billboard
		for (auto xmlAnimationNode : xmlLightNode.children("animation")) {
			LOG_DEBUG("Create light animation for: " << MGE::XMLUtils::nodeAsString(xmlAnimationNode) );
			
			int grpID = xmlAnimationNode.attribute("group").as_int(0);
			lightGroupStatus[ grpID ] = true;
			auto& lightsList = lightNodesList[ grpID ];
			
			std::string_view strType = xmlAnimationNode.attribute("type").as_string();
			if (strType == "rotating") {
				Ogre::Real speed = xmlAnimationNode.attribute("speed").as_float(0.5);
				
				lightsList.emplace_back(
					lightNode,
					getScaleControllerFunctions(speed, scaleControllerFunctions),
					OGRE_NEW MGE::RotationLightControllerValue(light)
				);
			} else if (strType == "flashing") {
				Ogre::Real        speed          = xmlAnimationNode.attribute("speed").as_float(1);
				Ogre::Real        switchOn       = xmlAnimationNode.attribute("switchOn").as_float(0.8);
				Ogre::Real        switchOff      = xmlAnimationNode.attribute("switchOff").as_float(1.0);
				
				lightsList.emplace_back(
					lightNode,
					getScaleControllerFunctions(speed, scaleControllerFunctions),
					OGRE_NEW MGE::FlashingLightControllerValue(light, light->getPowerScale(), billboard, sfxColour, switchOn, switchOff)
				);
			} else if (strType == "flashingRandom") {
				Ogre::Real        speed          = xmlAnimationNode.attribute("speed").as_float(2);
				Ogre::Real        switchOn       = xmlAnimationNode.attribute("switchOn").as_float(0.8);
				Ogre::Real        switchOff      = xmlAnimationNode.attribute("switchOff").as_float(1.0);
				
				Ogre::Real        randomLimit    = xmlAnimationNode.attribute("randomLimit").as_float(0.4);
				Ogre::Real        randomSetVal   = xmlAnimationNode.attribute("randomSetVal").as_float(0.6);
				Ogre::Real        randomSetLimit = xmlAnimationNode.attribute("randomSetLimit").as_float(3);
				
				lightsList.emplace_back(
					lightNode,
					OGRE_NEW MGE::RandomThresholdScaleControllerFunction(speed, randomLimit, randomSetVal, randomSetLimit), // MGE::RandomScaleControllerFunction(2, 0.35)
					OGRE_NEW MGE::FlashingLightControllerValue(light, light->getPowerScale(), billboard, sfxColour, switchOn, switchOff)
				);
			}
		}
	}
	return true;
}


bool MGE::Light::isGroupOn(int grpID) {
	try {
		return lightGroupStatus.at(grpID);
	} catch(...) {
		return false;
	}
}

void MGE::Light::setGroupOn(int grpID) {
	auto& lightsList = lightNodesList.at(grpID);
	for (auto& iter : lightsList)
		iter.on();
	lightGroupStatus.at(grpID) = true;
}

void MGE::Light::setGroupOff(int grpID) {
	auto& lightsList = lightNodesList.at(grpID);
	for (auto& iter : lightsList)
		iter.off();
	lightGroupStatus.at(grpID) = false;
}

void MGE::Light::setAllOn() {
	for (auto& iter : lightNodesList)
		setGroupOn(iter.first);
}

void MGE::Light::setAllOff() {
	for (auto& iter : lightNodesList)
		setGroupOff(iter.first);
}
