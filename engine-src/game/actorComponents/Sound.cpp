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

#include "game/actorComponents/Sound.h"

#include "LogSystem.h"

#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/factories/ComponentFactoryRegistrar.h"
#include "data/utils/OgreSceneObjectInfo.h"
#include "game/actorComponents/World3DMovable.h"
#include "rendering/audio-video/AudioSystem.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreStringConverter.h>

#ifdef USE_OGGSOUND
#include <OgreOggISound.h>
#endif


MGE::Sound::Sound(MGE::NamedObject* parent) :
	owner( static_cast<MGE::BaseActor*>(parent) )
{
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::ActorMovingEventMsg::MsgType,
		std::bind(&MGE::Sound::Sound::updateOnEvent, this, std::placeholders::_1),
		this,
		owner
	);
}

MGE::Sound::~Sound() {
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::ActorMovingEventMsg::MsgType,
		std::bind(&MGE::Sound::Sound::updateOnEvent, this, std::placeholders::_1),
		this,
		owner
	);
	clear();
}

void MGE::Sound::clear() {
#ifdef USE_OGGSOUND
	for (auto& iter : sounds) {
		iter.second->getParentSceneNode()->detachObject(iter.second);
		MGE::AudioSystem::getPtr()->destroySound(iter.second);
	}
	sounds.clear();
#endif
}

MGE_ACTOR_COMPONENT_CREATOR(MGE::Sound, Sound) {
	if (MGE::AudioSystem::getPtr() != nullptr) {
		typeIDs->insert(MGE::Sound::classID);
		return new MGE::Sound(parent);
	} else {
		LOG_WARNING("Skip \"Sound\" component registration due to lack of AudioSystem");
		return nullptr;
	}
}


/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_Sound Sound

Use subnodes:
  - @ref XMLNode_Sound for defining sounds added to actor, support additional, optional attributes:
    - @c playOnMoving    when true sound auto play when start moving and auto stop when stop moving (@ref XML_Bool)
    - @c playOnNotMoving when true sound auto play when stop moving and auto stop when start moving (@ref XML_Bool)
*/
bool MGE::Sound::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	std::string prefix( Ogre::StringConverter::toString(reinterpret_cast<size_t>(parent)) + "_" );
	LOG_DEBUG("SoundComponent: create sounds for " << prefix);
	
	clear();
	
	for (auto xmlSubNode : xmlNode.children("sound")) {
		OgreOggSound::OgreOggISound* sound = MGE::AudioSystem::processSoundXMLNodeWithPrefix(
			xmlSubNode, nullptr, {sceneNode, nullptr}, prefix
		);
		sounds[ xmlSubNode.attribute("name").as_string() ] = sound;
		
		if (xmlSubNode.attribute("playOnMoving").as_bool(false)) {
			onWhenMove.insert( sound );
		}
		if (xmlSubNode.attribute("playOnNotMoving").as_bool(false)) {
			offWhenMove.insert( sound );
		}
	}
	return true;
}

void MGE::Sound::updateOnEvent(const MGE::EventMsg* msg) {
#ifdef USE_OGGSOUND
	auto movingMsg = static_cast<const MGE::ActorMovingEventMsg*>(msg);
	
	LOG_DEBUG("SoundComponent: actor move change: " << movingMsg->actor << " move is " << movingMsg->isMove);
	
	isMoving = movingMsg->isMove;
	if (isMoving) {
		for (auto& iter : onWhenMove)
			iter->play();
		for (auto& iter : offWhenMove)
			iter->stop();
	} else {
		for (auto& iter : onWhenMove)
			iter->stop();
		for (auto& iter : offWhenMove)
			iter->play();
	}
#endif
}

void MGE::Sound::play(const std::string_view& name) {
#ifdef USE_OGGSOUND
	auto s = sounds.find(name);
	if (s != sounds.end())
		s->second->play();
#endif
}

void MGE::Sound::stop(const std::string_view& name) {
#ifdef USE_OGGSOUND
	auto s = sounds.find(name);
	if (s != sounds.end())
		s->second->stop();
#endif
}

void MGE::Sound::playOnMoving(const std::string_view& name, bool set) {
#ifdef USE_OGGSOUND
	LOG_DEBUG("set playOnMoving for " << name << " to " << set << " isMoving=" << isMoving);
	
	auto s = sounds.find(name);
	if (s == sounds.end())
		return;
	auto sound = s->second;
	
	if (set) {
		onWhenMove.insert( sound );
		if (isMoving)
			sound->play();
	} else {
		onWhenMove.erase( sound );
		sound->stop();
	}
#endif
}

void MGE::Sound::playOnNotMoving(const std::string_view& name, bool set) {
#ifdef USE_OGGSOUND
	LOG_DEBUG("set playOnNotMoving for " << name << " to " << set << " isMoving=" << isMoving);
	
	auto s = sounds.find(name);
	if (s == sounds.end())
		return;
	auto sound = s->second;
	
	if (set) {
		offWhenMove.insert( sound );
		if (!isMoving)
			sound->play();
	} else {
		offWhenMove.erase( sound );
		sound->stop();
	}
#endif
}
