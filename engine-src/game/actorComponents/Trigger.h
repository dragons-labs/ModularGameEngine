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
#include "data/structs/BaseComponent.h"

namespace MGE { struct BaseActor; }

#include <map>

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements trigger interface for (trigger) Actor
 */
class Trigger :
	public MGE::BaseComponent
{
public:
	enum TrigerTypes {
		/// trigger object is disabled, do not trigger anythings
		DISABLED = 0,
		/// script based trigger for actions
		RUN_ACTION_SCRIPT = 1,
		/// script based trigger for access
		RUN_SPEED_SCRIPT = 2,
		/// script based trigger for actions and access
		RUN_SCRIPT = 3,
		/// use map of speeds modifier for access check
		CHECK_SPEED_MAP = 4,
		/// no access to trigger area
		NO_ACCESS  = 5
	};
	
	/**
	 * @brief convert string notation of TrigerTypes to numeric value
	 * 
	 * @param[in] s  string to convert
	 */
	inline static int stringToTrigerType(const std::string_view& s) {
		if (s == "DISABLED")               return DISABLED;
		else if (s == "RUN_ACTION_SCRIPT") return RUN_ACTION_SCRIPT;
		else if (s == "RUN_SPEED_SCRIPT")  return RUN_SPEED_SCRIPT;
		else if (s == "RUN_SCRIPT")        return RUN_SCRIPT;
		else if (s == "CHECK_SPEED_MAP")   return CHECK_SPEED_MAP;
		else if (s == "NO_ACCESS")         return NO_ACCESS;
		return MGE::StringUtils::toNumeric<int>(s);
	}
	
	/**
	 * @brief run trigger script
	 * 
	 * @param actor - pointer to actor who launch trigger
	 */
	void runTrigger(MGE::BaseActor* actor) const;
	
	/**
	 * @brief return speed modifier (usually \<= 1.0), when return 0 actor can't cross this trigger
	 * 
	 * @param actor - pointer to actor who launch trigger
	 */
	float getSpeedModifier(MGE::BaseActor* actor) const;
	
	/// prefix for scripts names to execute when trigger is hit
	///  - scriptName + "_check" will be executed on @ref getSpeedModifier
	///  - scriptName + "_run" will be executed on @ref runTrigger
	std::string scriptName;
	
	/// trigger type for identification trigger (e.g. run script or for make inaccessible trigger area),
	/// see @ref TrigerTypes
	int triggerType;
	
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// @copydoc MGE::BaseComponent::init
	virtual void init(MGE::NamedObject* parent) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 7;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// constructor
	Trigger(MGE::NamedObject* parent);
	
protected:
	/// destructor
	virtual ~Trigger();
	
	/// map movable subtype -> speed modifier for this trigger
	std::map<int,float> speedModifiers;
};

/// @}

}
