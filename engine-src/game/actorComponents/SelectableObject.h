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

#include "StringUtils.h"
#include "data/structs/BaseComponent.h"

namespace MGE { struct BaseActor; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements selectable states and minimap symbols for Actor
 */
class SelectableObject :
	public MGE::BaseComponent
{
public:
	/// list of all selectable objects
	static std::set<MGE::SelectableObject*> allSelectableObject;
	
	/**
	 * @brief flags using to actor selection and filtering
	 */
	enum StatusFlags {
		/// is selectable (can be action origin)
		IS_SELECTABLE      = (1 << 0),
		/// is target of actions (even actors with IS_SELECTABLE must have IS_ACTION_TARGET for be target action)
		IS_ACTION_TARGET   = (1 << 1),
		/// is hidden
		IS_HIDDEN          = (1 << 2),
		/// is (temporary) unavailable
		IS_UNAVAILABLE     = (1 << 3),
	};
	
	/// integer type for status bit mask
	typedef uint8_t status_t;
	
	/**
	 * @brief convert string notation of StatusFlags to numeric mask (single flag value)
	 * 
	 * @param[in] s  string to convert
	 */
	inline static status_t stringToStatusFlag(const std::string_view& s) {
		if (s == "IS_SELECTABLE")            return IS_SELECTABLE;
		else if (s == "IS_ACTION_TARGET")    return IS_ACTION_TARGET;
		else if (s == "IS_HIDDEN")           return IS_HIDDEN;
		else if (s == "IS_UNAVAILABLE")      return IS_UNAVAILABLE;
		return MGE::StringUtils::toNumeric<status_t>(s);
	}
	
	/**
	 * @brief convert string notation of StatusFlags to numeric mask (space delimited list of flags)
	 * 
	 * @param[in] s  string to convert
	 */
	static status_t stringToStatusMask(const std::string_view& s);
	
	/// object selectable status mask
	status_t status;
	
	/**
	 * @brief set available status of actor and switch visibility
	 * 
	 * @param isAvailable  when true remove IS_UNAVAILABLE, when false set IS_UNAVAILABLE flag
	 * @param setVisible   when true set 3D world visible to @a isAvailable
	 * 
	 * @note call actor create / destroy listeners callback functions via MGE::ActorFactory
	 */
	void setAvailable(bool isAvailable, bool setVisible = true);
	
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	/**
	 * @brief return (via reference arguments) minimap symbol (icon)
	 * 
	 * @param[out] buf    pointer to 4bit per channel ARGB buffer with symbol
	 * @param[out] width  width  of symbol (line length in buf)
	 * @param[out] height height of symbol (number of line in buf)
	 */
	void getMiniMapSymbol(const uint16_t*& buf, int& width, int& height);
	
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 6;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// constructor
	SelectableObject(MGE::NamedObject* parent);
	
protected:
	/// destructor
	virtual ~SelectableObject();
	
	/// width of mini map symbol
	int miniMapSymbolWidth;
	
	/// height of mini map symbol
	int miniMapSymbolHeight;
	
	/// mini map symbol data, ARGB 4bit per channel
	uint16_t* miniMapSymbol;
};

/// @}

}
