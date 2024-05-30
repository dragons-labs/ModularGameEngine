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

#include "data/structs/BaseComponent.h"

#include "core/ScriptsSystem.h"

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief Class implements script-based component for Actor
 */
class ScriptComponent :
	public MGE::BaseComponent
{
public:
	/// return python object created/used by this component
	pybind11::object getPythonObject() {
		return *pythonObject;
	}
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// @copydoc MGE::BaseComponent::init
	virtual void init(MGE::NamedObject* parent) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	///
	/// @note For ScriptComponent this is NOT a static member. Initialized to value of @a createdForID from @ref create.
	///       We can have multiple ScriptComponent objects with different classID and different class used as @ref pythonObject.
	int scriptClassID;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == scriptClassID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return scriptClassID;
	}
	
	/// static function for register in MGE::ComponentFactory
	static MGE::BaseComponent* create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID);
	
	/**
	 * @brief static function performing registration in MGE::ComponentFactory
	 * 
	 * @param typeID    unique numeric id value of registering component
	 * @param className unique name of registering component
	 * 
	 * @note ScriptComponent will create object of @a className python class. This class:
	 *       - must be loaded before create first component of @a typeID type.
	 *       - must have constructor with two argument (pointer to "parent" actor and string with \<Component\> XML node)
	 *       - must have method @c restore with one argument (string with \<Component\> XML node)
	 *       - must have method @c store return correct XML string (to put into content of \<Component\> XML node) or empty string
	 */
	static void setup(int typeID, const std::string& className);
	
protected:
	/// python object created and used by this component
	pybind11::object* pythonObject;
	
	/// constructor
	ScriptComponent(MGE::NamedObject* parent, const pugi::xml_node& config, int createdForID);
	
	/// destructor
	virtual ~ScriptComponent();
};


/// @}

}
