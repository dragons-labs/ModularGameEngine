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

#include <unordered_map>
#include <list>

namespace Ogre {
	namespace v1 {
		class BillboardSet;
	}
	class SceneNode;
}

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief Class implements Light component for Actor
 */
class Light :
	public MGE::BaseComponent
{
public:
	/**
	 * @brief return true when lights in group are "on"
	 * 
	 * @param grpID   group ID
	 */
	bool isGroupOn(int grpID);
	
	/**
	 * @brief set "on" all lights in group
	 * 
	 * @param grpID   group ID
	 */
	void setGroupOn(int grpID);
	
	/**
	 * @brief set "on" all lights in group
	 * 
	 * @param grpID   group ID
	 */
	void setGroupOff(int grpID);
	
	/**
	 * @brief set "on" all lights in all groups
	 * 
	 * @param grpID   group ID
	 */
	void setAllOn();
	
	/**
	 * @brief set "off" all lights in all groups
	 * 
	 * @param grpID   group ID
	 */
	void setAllOff();
	
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 13;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// constructor
	Light(MGE::NamedObject* parent);
	
protected:
	/// remove lights attached to SceneNode
	void clear();
	
	/// destructor
	virtual ~Light();
	
private:
	struct LightNode;
	std::unordered_map< int, std::list<LightNode> > lightNodesList;
	std::unordered_map< int, bool > lightGroupStatus;
	Ogre::v1::BillboardSet* billboardSet;
	Ogre::SceneNode* rootNode;
};

/// @}

}
