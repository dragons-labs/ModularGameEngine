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

#include "BaseClasses.h"
#include "force_inline.h"
#include "data/property/Any.h"
#include "data/property/PropertySetInterface.h"

namespace MGE { struct LoadingContext; }
namespace MGE { struct BaseComponent; }

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief Base class for game objects (including actors, prototypes, and components) interfaces (with save/store interface).
 * 
 * @remark
 *   NoCopyableNoMovable because:
 *     - Class derived from BaseObject are created (and registered) by factories and should not be copied.
 *     - Pointers to BaseObject and class derived from BaseObject are store in multiple places,
 *       so address of these objects should not be changed by copy/move operation.
 */
struct BaseObject : MGE::NoCopyableNoMovable {
	/**
	 * @name XML save/store interface for objects
	 * 
	 * @{
	 */
		/// @copydoc MGE::SaveableToXMLInterface::storeToXML
		virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const {return false;}
		
		FORCE_INLINE bool storeToXML(pugi::xml_node&& xmlNode, bool onlyRef = false) const {
			return storeToXML(xmlNode, onlyRef);
		}
		
		friend FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const BaseObject& val) {
			val.storeToXML(xmlNode, false);
			return xmlNode;
		}
	/**
	 * @}
	 */
	
protected:
	/**
	 * @name Prevent direct creating objects of BaseObject class
	 * 
	 * - It's allowed crating only derived classes of BaseObject.
	 * 
	 * @{
	 */
		/// protected default constructor
		BaseObject()          = default;
	/**
	 * @}
	 *
	 * 
	 * @name Prevent deleting objects by BaseObject base class pointer / prevent direct deleting
	 * 
	 * - Deleting should be done by the factory class using derived class.
	 * 
	 * @{
	 */
		/// protected default virtual destructor
		virtual ~BaseObject() = default;
	/**
	 * @}
	 */
};

/**
 * @brief base struct for full (named) game objects (actors or prototypes) with basic features: name, properties and save-load interface
 */
struct NamedObject :
	public MGE::BaseObject,
	public MGE::PropertySetInterface
{
	/**
	 * @brief return object type
	 */
	virtual const std::string& getType() const = 0;
	
	/**
	 * @brief return object name
	 */
	virtual const std::string& getName() const = 0;
	
	/**
	 * @brief return object specific component (or NULL when specific component do not exist in this object and classID == 0)
	 * 
	 * @param  typeID   numeric type id of component to return
	 * @param  classID  numeric class id registred in ComponentFactory to create component
	 *                  if component with provided @a typeID not exist in this object (and if classID != 0)
	 */
	virtual MGE::BaseComponent* getComponent(int typeID, int classID = 0) = 0;
	
	/**
	 * @brief return object specific component (or NULL when specific component do not exist in this object)
	 * 
	 * @param  typeID   numeric type id of component to return
	 */
	inline virtual const MGE::BaseComponent* getComponent(int typeID) const {
		return const_cast<MGE::NamedObject*>(this)->getComponent(typeID);
	}
	
	/**
	 * @brief return object specific component with casting to ReturnType,
	 *        return NULL when specific component do not exist in this object and classID == 0
	 * 
	 * @param  typeID   numeric type id of component to return
	 * @param  classID  numeric class id registred in ComponentFactory to create component
	 *                  if component with provided @a typeID not exist in this object (and if classID != 0)
	 */
	template <typename ReturnType> ReturnType* getComponent(int typeID, int classID = 0) {
		return static_cast<ReturnType*>(getComponent(typeID, classID));
	}
	
	/**
	 * @brief return object specific component with casting to ReturnType,
	 *        return NULL when specific component do not exist in this object
	 * 
	 * @param  typeID   numeric type id of component to return
	 */
	template <typename ReturnType> const ReturnType* getComponent(int typeID) const {
		return static_cast<const ReturnType*>(getComponent(typeID));
	}
	
	//@{
	/**
	 * @brief return object specific component with casting to ReturnType,
	 *        return NULL when specific component do not exist in this object
	 *        use ReturnType::classID as getting typeID
	 */
	template <typename ReturnType> ReturnType* getComponent() {
		return static_cast<ReturnType*>(getComponent(ReturnType::classID));
	}
	template <typename ReturnType> const ReturnType* getComponent() const {
		return static_cast<const ReturnType*>(getComponent(ReturnType::classID));
	}
	//@}
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {return false;}
	
	/**
	 * @brief get actor or prototype based on XML
	 * 
	 * @param  xmlNode      xml node that contains actor (\<ActorName\>) or prototype (\<Prototype\>) refernce tag
	 * 
	 * @note  first try get actor, next try get prototype, return NULL when not found proper XML tag
	 */
	static NamedObject* get(const pugi::xml_node& xmlNode);
	
	/**
	 * @brief insert to collection actors and/or prototypes based on XML
	 * 
	 * @param  xmlNode      xml node that contains actors (\<ActorName\>) and/or prototypes (\<Prototype\>) refernce tags
	 * @param  collection   pointer to collection (eg. std::set) to store obtained NamedObject(s)
	 * 
	 * @note  use one agrument insert() member function of CollectionType with pointer to obtained NamedObject
	 */
	template <typename CollectionType> static void insertToCollection(
		const pugi::xml_node& xmlNode, CollectionType* collection
	);
};


/// @}

}

namespace pugi {
	/// @brief store as XML to MGE::Utils::XMLOutputArchive operator for MGE::BaseActor object
	/// @note  We need this operator, because we use MGE::Any with ValueType == MGE::BaseActor,
	///        although we don't use store() of this MGE::Any.
	///        This operator do real store of MGE::BaseActor, so it can be use in other cases too.
	inline pugi::xml_node& operator<<(pugi::xml_node& xmlNode, MGE::BaseObject* const& value) {
		value->storeToXML(xmlNode, true);
		return xmlNode;
	}
}
