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

#include "data/structs/BaseObject.h"
#include "data/structs/ComponentsCollection.h"
#include "data/property/PropertySet.h"

namespace MGE { struct BasePrototype; }
namespace MGE { struct ActorFactory; }

namespace Ogre {
	class SceneNode;
	class MovableObject;
}

#include <map>

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief class (abstract - interface only) for game objects (actors)
 */
struct BaseActor :
	public MGE::NamedObject
{
	/// return type name for comparable with value return getType() for check if this NamedObject is BaseActor
	inline static const std::string& TypeName() {
		static std::string typeName("BaseActor");
		return typeName;
	}
	
	/**
	 * @name getting BaseActor from Ogre scene objects
	 * 
	 * @{
	 */
		/**
		 * @brief get pointer to BaseActor from Ogre::SceneNode
		 */
		static MGE::BaseActor* get(const Ogre::SceneNode* node);
		
		/**
		 * @brief get pointer to BaseActor from Ogre::MovableObject
		 */
		static MGE::BaseActor* get(const Ogre::MovableObject* movable);
	/**
	 * @}
	 */
	
	/**
	 * @brief get prototype of this actor
	 */
	virtual const MGE::BasePrototype* getPrototype() const = 0;
	
protected:
	friend struct ActorFactory;
	
	/// destructor
	virtual ~BaseActor() { }
};

/**
 * @brief simple implementation of BaseActor
 */
struct BaseActorImpl :
	public MGE::BaseActor
{
	/**
	 * @name base BaseActorImpl basic elements
	 * 
	 * @{
	 */
		/// @copydoc MGE::NamedObject::getType
		virtual const std::string& getType() const override;
		
		/// @copydoc MGE::NamedObject::getName
		virtual const std::string& getName() const override;
		
		/// @copydoc MGE::NamedObject::getComponent(int, int)
		virtual MGE::BaseComponent* getComponent(int typeID, int classID = 0) override;
		
		/// @copydoc MGE::NamedObject::getComponent(int)
		virtual const MGE::BaseComponent* getComponent(int typeID) const override;
		
		/// @copydoc MGE::BaseActor::getPrototype
		virtual const MGE::BasePrototype* getPrototype() const override;
	/**
	 * @}
	 * 
	 * 
	 * @name MGE::PropertySet interface
	 * 
	 * @{
	 */
		/// @copydoc MGE::PropertySetInterface::getProperty
		virtual const MGE::Any& getProperty(const std::string_view& key) const override;
		
		/// @copydoc MGE::PropertySetInterface::remProperty
		/// return -1 when adding masking (MGE::Any::EMPTY) property to set,
		/// because property with key is set in (read only) prototype property set
		virtual size_t remProperty(const std::string_view& key) override;
		
		/// @copydoc MGE::PropertySetInterface::addProperty(const std::string&, const MGE::Any&, bool replace)
		virtual bool addProperty(const std::string_view& key, const MGE::Any& val, bool replace = false) override;
		
		/// @copydoc MGE::PropertySetInterface::setProperty(const std::string&, const MGE::Any&)
		virtual bool setProperty(const std::string_view& key, const MGE::Any& val) override;
	/** 
	 * @}
	 * 
	 * 
	 * @name store and restore to/from XML
	 * 
	 * @{
	 */
		/// @copydoc MGE::NamedObject::store
		virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
		
		/// @copydoc MGE::NamedObject::restore
		/// @note (in real restore, not read config mode) must be called after complete list of Actor (@ref MGE::ActorFactory::allActors)
		virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	/** 
	 * @}
	 */
	
protected:
	friend struct ActorFactory;
	
	/// unique name of object
	std::string                name;
	
	/// pointer to prototype
	const MGE::BasePrototype*  prototype;
	
	/// properties set
	MGE::PropertySet           properties;
	
	/// collection of actor components
	ComponentsCollection       components;
	
	/// constructor
	BaseActorImpl(
		const std::string&        _name,
		const MGE::BasePrototype* _prototype
	);
	
	/// destructor
	/// @note destructor is protected
	///       for delete Actor should be use MGE::ActorFactory::destroyActor
	virtual ~BaseActorImpl();
};


/// @}

}
