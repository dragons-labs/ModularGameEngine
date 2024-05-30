/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/BaseObject.h"

namespace MGE { struct ComponentFactory; }

namespace Ogre { class SceneNode; }

#include <set> /* don't need in this header but need in any derived class header */

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief base (abstract - interface only) class for game objects Component
 */
struct BaseComponent :
	public MGE::BaseObject
{
public:
	/**
	 * @brief Check if this Component class provide specific type.
	 *
	 * @param id numeric id of type to check
	 * 
	 * @remark 
	 *       - <i>TypeID</i> is used to determinate feature set provided by class
	 *          - classes can support multiple typeID via MGE::BaseComponent::provideTypeID()
	 *          - but every class have primary typeID value returned by MGE::BaseComponent::getClassID
	 *          - various classes can provide (used) this same typeID value
	 *          - but object can have attached only one component implemented specific typeID
	 *            and component should be attached with all supported by it typeID
	 *       - <i>ClassID</i> (aka primary TypeID) must be unique (no allowed two classes with this same classID value)
	 *         and must be greater than zero (zero means removed component).
	 * 
	 * @warning
	 *       - change existing values of <i>ClassID</i> / <i>TypeID</i> may break existing save and config files
	 *       - change existing values of <i>ClassID</i> / <i>TypeID</i> may break some python scripts
	 *       - do not use zero or negative value for <i>ClassID</i> / <i>TypeID</i>
	 */
	virtual bool provideTypeID(int id) const = 0;
	
	/**
	 * @brief Return primary type id of this Component class (unique class identifier).
	 * 
	 * @remark 
	 *       See additional info in @ref provideTypeID documentation.
	 */
	virtual int getClassID() const = 0;
	
	/**
	 * @brief restore from xml serialization archive
	 * 
	 * @note DO NOT use other components from parent in this call, do this in @ref init
	 * 
	 * @note this function will NOT be called when restored XML node do not contain any child nodes
	 * 
	 * @param[in]  xmlNode      xml node that will be using for load state of this object
	 * @param[in]  parent       pointer to owner of component (actor / prototype)
	 * @param[in]  sceneNode    pointer to main scene node of @a parent (can be NULL)
	 */
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) {return false;}
	
	/**
	 * @brief init component
	 * 
	 * @param[in]  parent       pointer to owner of component (actor / prototype)
	 */
	virtual void init(MGE::NamedObject* parent) {}
	
protected:
	friend struct ComponentFactory;
	
	/// destructor
	virtual ~BaseComponent() { }
};


/// @}

}
