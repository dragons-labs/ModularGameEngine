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

#include "data/structs/BaseActor.h"
#include "data/structs/BaseComponent.h"

#include "Engine.h"
#include "MessagesSystem.h"

namespace OgreOggSound { class OgreOggISound; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements Sound component for Actor
 */
class Sound :
	public MGE::BaseComponent
{
public:
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 12;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// callback function for events message
	void updateOnEvent(const MGE::EventMsg* msg);
	
	/// play selected sound
	void play(const std::string_view& name);
	
	/// stop selected sound
	void stop(const std::string_view& name);
	
	/// set/unset play selected sound as auto play when start moving and auto stop when stop moving
	/// @a set == true → set, @a set == false → unset
	void playOnMoving(const std::string_view& name, bool set);
	
	/// set/unset play selected sound as auto play when stop moving and auto stop when start moving
	/// @a set == true → set, @a set == false → unset
	void playOnNotMoving(const std::string_view& name, bool set);
	
	/// constructor
	Sound(MGE::NamedObject* parent);
	
protected:
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	/// prefix used in sounds names
	
	/// remove sounds attached to SceneNode
	void clear();
	
	/// destructor
	virtual ~Sound();
	
	std::map<std::string, OgreOggSound::OgreOggISound*, std::less<>> sounds;
	std::set<OgreOggSound::OgreOggISound*>  onWhenMove;
	std::set<OgreOggSound::OgreOggISound*> offWhenMove;
	
	bool isMoving;
};

/// @}

}
