/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include <map>

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief struct for "object owner" actor component
 */
struct ObjectOwner :
	public MGE::BaseComponent
{
	/// struct for info about owned object
	struct Info {
		/// current quantity - used for doing action
		int currentQuantity;
		
		/// planned quantity for end of actions queue - used for checking on add action to queue
		/// (if some action from queue faild final @ref currentQuantity can be diffrent from @ref plannedQuantity)
		int plannedQuantity;
	};
	
	
	/**
	 * @brief add or modify object - set quantity (or remove object, if both values \<= 0)
	 * 
	 * @param obj      pointer to object (actor or prototype) to update quantity
	 * @param current  currently number of objects
	 * @param planned  predicted number of objects at end action queue
	 */
	void set(MGE::NamedObject* obj, short current, short planned);
	
	/**
	 * @brief update currentQuantity or plannedQuantity instantions of object
	 * 
	 * @param obj      pointer to object (actor or prototype) to update quantity
	 * @param current  number of instantion to add (when > 0) or remove (when < 0) to/from currentQuantity
	 * @param future   number of instantion to add (when > 0) or remove (when < 0) to/from plannedQuantity
	 */
	void update(MGE::NamedObject* obj, int current, int future);
	
	/**
	 * @brief set @ref Info::plannedQuantity to @ref Info::currentQuantity for all objects
	 */
	void resetPlanned();
	
	/**
	 * @brief constant iterator type for owned object collection
	 */
	typedef std::map<MGE::NamedObject*, MGE::ObjectOwner::Info>::const_iterator iterator;
	
	/**
	 * @brief return begin const iterator
	 */
	inline iterator begin() const {
		return ownedObjects.cbegin();
	}
	
	/**
	 * @brief return end const iterator
	 */
	inline iterator end() const {
		return ownedObjects.cend();
	}
	
	/**
	 * @brief return const iterator to specyfic object
	 * 
	 * @param obj              pointer to related object (e.g. scene object, scene object prototype) to find
	 */
	inline iterator find(MGE::NamedObject* obj) const {
		return ownedObjects.find(obj);
	}
	
	/**
	 * @brief return value of lastUpdateTime
	 */
	std::chrono::time_point<std::chrono::steady_clock> getLastUpdateTime() { return lastUpdateTime; }
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 0x03;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// constructor
	ObjectOwner();
	
protected:
	/// destructor
	virtual ~ObjectOwner() { }
	
	/// map of objects and status info
	std::map<MGE::NamedObject*, MGE::ObjectOwner::Info>  ownedObjects;
	
	/// last time of set modification (see @ref MGE::Engine::getMainLoopTime)
	/// used to determine the necessary of updating queue information in other class
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
};


/// @}

}
