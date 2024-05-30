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
#include "data/utils/ResourceLocationInfo.h"
#include "data/property/PropertySet.h"

namespace MGE { struct PrototypeFactory; }

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief class for (abstract - interface only) game objects prototypes
 */
struct BasePrototype :
	public MGE::NamedObject
{
	/// return type name for comparable with value return getType() for check if this NamedObject is BasePrototype
	inline static const std::string& TypeName() {
		static std::string typeName("BasePrototype");
		return typeName;
	}
	
	/**
	 * @brief return config info
	 */
	virtual const MGE::ResourceLocationInfo* getLocationInfo() const = 0;
	
	/**
	 * @brief return pointer to prototype XML configuration node
	 * 
	 * @param config  config info identifying prototype and describing the prototype config location
	 * @param xmlDoc  pugixml document object for opening xml file specified by @a config
	 */
	static pugi::xml_node getPrototypeXML(
		const MGE::ResourceLocationInfo* config,
		pugi::xml_document& xmlDoc
	);
	
protected:
	friend struct PrototypeFactory;
	
	/// destructor
	virtual ~BasePrototype() {}
};

/**
 * @brief simple implementation of BasePrototype
 */
struct BasePrototypeImpl :
	public MGE::BasePrototype
{
	/**
	 * @name base BasePrototypeImpl basic elements
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
		
		/// @copydoc MGE::BasePrototype::getLocationInfo
		virtual const MGE::ResourceLocationInfo* getLocationInfo() const override;
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
		
		/// @warning newer use this member function on BasePrototypeImpl object (always throw exception);
		///          properties in prototype are read-only
		[[ noreturn ]] size_t remProperty(const std::string_view& key) override;
		
		/// @warning newer use this member function on BasePrototypeImpl object (always throw exception);
		///          properties in prototype are read-only
		[[ noreturn ]] bool addProperty(const std::string_view& key, const MGE::Any& val, bool replace = false) override;
		
		/// @warning newer use this member function on BasePrototypeImpl object (always throw exception);
		///          properties in prototype are read-only
		[[ noreturn ]] bool setProperty(const std::string_view& key, const MGE::Any& val) override;
	/** 
	 * @}
	 * 
	 * 
	 * @name store and restore to/from XML
	 * 
	 * @{
	 */
		/// @copydoc MGE::NamedObject::store
		/// @note always store only config source information (name, fileName, fileGroup);
		///       not store properties (are read only, and will be restored from config source) nor components
		virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
		
		/// @copydoc MGE::NamedObject::restore
		/// @note  restore (load) only properties and components;
		///        restoring config source must be done before construct BasePrototypeImpl
		virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	/** 
	 * @}
	 */
	
protected:
	friend struct PrototypeFactory;
	
	/// location of prototype config
	MGE::ResourceLocationInfo config;
	
	/// prototype properties
	MGE::PropertySet properties;
	
	/// collection of actor components
	ComponentsCollection components;
	
	/// constructor
	BasePrototypeImpl(const std::string& _name, const std::string& _fileName, const std::string& _fileGroup);
	
	/// destructor
	virtual ~BasePrototypeImpl();
};


/// @}

}
