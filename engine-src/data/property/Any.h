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

-- Based on / Inspired by: Ogre::Any class, original copyright information follows --
Copyright (c) 2000-2014 Torus Knot Software Ltd
Distributed under the MIT license terms (see ADDITIONAL_COPYRIGHT/Ogre.txt file)
	
	Ogre::Any is based on boost::any
		Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
		Distributed under the Boost Software License, Version 1.0.
		See ADDITIONAL_COPYRIGHT/Boost.txt file or http://www.boost.org/LICENSE_1_0.txt
-- End original copyright --
*/

#pragma   once

#include <OgreAny.h>

#include "force_inline.h"
#include "XmlUtils.h"

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

#if !defined(__DOCUMENTATION_GENERATOR__)  && !defined(__PYTHON_DOCUMENTATION_GENERATOR__)
template<typename ValueType> requires MGE::ListType<ValueType>
FORCE_INLINE void writeAnyValueToStream(std::ostream& s, const ValueType& val) {
	s << "std::list<...>: { ";
	for (auto& it : val)
		s << it << " ";
	s << "}";
}

template<typename ValueType> requires MGE::MapType<ValueType>
FORCE_INLINE void writeAnyValueToStream(std::ostream& s, const ValueType& val) {
	s << "std::map<...>: { ";
	for (auto& it : val)
		s << it.first << "=>" << it.second << " ";
	s << "}";
}

template<typename ValueType> requires (!MGE::ListType<ValueType> && !MGE::MapType<ValueType>)
FORCE_INLINE void writeAnyValueToStream(std::ostream& s, const ValueType& val) {
	s << val;
}

template<typename ValueType> requires (MGE::ListType<ValueType> || MGE::MapType<ValueType>)
FORCE_INLINE void writeAnyValueToXML(pugi::xml_node& xmlNode, const ValueType& value) {
	xmlNode << value;
}

template<typename ValueType> requires (!MGE::ListType<ValueType> && !MGE::MapType<ValueType>)
FORCE_INLINE void writeAnyValueToXML(pugi::xml_node& xmlNode, const ValueType& value) {
	xmlNode.append_child("value") << value;
}
#endif

/**
 * @brief Wrapper class for Ogre::Any.
 */
class Any : public Ogre::Any {
public:
	/// constructor - default
	Any() = default;
	
	/// constructor - copy from MGE::Any
	Any(const MGE::Any& other) {
		mContent = other.mContent ? other.mContent->clone() : 0;
	}
	
	/// constructor - from reference to value
	template<typename ValueType> explicit Any(const ValueType& value) {
		mContent = OGRE_NEW_T(MGE::Any::holder<ValueType>, Ogre::MEMCATEGORY_GENERAL)(value);
	}
	
	/// destructor
	virtual ~Any() = default;
	
	/**
	 * @brief return value (without checking type)
	 * 
	 * @note we must return by ResultType object (not const reference to ResultType) because temporary Ogre::Any object,
	 *       so in the case of non basic types is recommended store pointer to object in Any instead store object in Any
	 */
	template <typename ValueType> ValueType getValue() const {
		return static_cast<Ogre::Any::holder<ValueType> *>(mContent)->held;
	}
	
	/**
	 * @brief return pointer to value (without checking type)
	 */
	template <typename ValueType> ValueType* getValuePtr() const {
		if (!mContent)
			return NULL;
		else
			return &(static_cast<Ogre::Any::holder<ValueType> *>(mContent)->held);
	}
	
	/**
	 * @brief return value (without checking type), when Any is empty return @a defVal
	 */
	template <typename ValueType> ValueType getValue(const ValueType& defVal) const {
		if (!mContent)
			return defVal;
		else
			return  static_cast<Ogre::Any::holder<ValueType> *>(mContent)->held;
	}
	
	/**
	 * @brief get default (no key) Any from UserObjectBindings in @a node
	 */
	template <typename NodeType> inline static const Any& getFromBindings(const NodeType* node) {
		return static_cast<const Any&>(
			node->getUserObjectBindings().getUserAny()
		);
	}
	
	/**
	 * @brief get Any with @a key from UserObjectBindings in @a node
	 *        default case for Ogre::Utils::String, const char* , etc key
	 */
	template <typename NodeType, typename KeyType> inline static const Any& getFromBindings(const NodeType* node, const KeyType& key) {
		return static_cast<const Any&>(
			node->getUserObjectBindings().getUserAny(key)
		);
	}
	
	/**
	 * @brief set default (no key) Any from UserObjectBindings in @a node
	 */
	template <typename NodeType, typename ValueType> inline static void setToBindings(NodeType* node, const ValueType& value) {
		node->getUserObjectBindings().setUserAny( Any(value) );
	}
	
	/**
	 * @brief set Any with @a key from UserObjectBindings in @a node
	 *        default case for Ogre::Utils::String, const char* , etc key
	 */
	template <typename NodeType, typename KeyType, typename ValueType> inline static void setToBindings(NodeType* node, const KeyType& key, const ValueType& value) {
		node->getUserObjectBindings().setUserAny( key, Any(value) );
	}
	
	/**
	 * @brief class for automatic determinate and run (or not run) dynamic_cast<>() on Ogre::any_cast<>() results
	 * 
	 * @note class used specializations of templates to avoid unnecessary dynamic_cast<>()
	 *       when typeid(AnyElementType) != typeid(ResultType) use this template and do dynamic_cast<>()
	 */
	template <typename ResultType, typename ValueType> struct Cast {
		/// return value from @a any
		inline static ResultType getValue(const Any& any) {
			return dynamic_cast<ResultType>(any.getValue<ValueType>());
		}
	};
	
	/**
	 * @brief class for automatic determinate and run (or not run) dynamic_cast<>() on Ogre::any_cast<>() results
	 * 
	 * @note class used specializations of templates to avoid unnecessary dynamic_cast<>()
	 *       when typeid(AnyElementType) == typeid(ResultType) use this template specialization and do not dynamic_cast<>()
	 */
	template <typename ValueType> struct Cast<ValueType, ValueType> {
		/// return value from @a any
		inline static ValueType getValue(const Any& any) {
			return any.getValue<ValueType>();
		}
	};
	
	/**
	 * @brief store value (without opening / close xml tag) to xml serialization archive
	 * 
	 * @note opening tag should be added externaly (eg \<Property name="..." type="..." /\> in PropertySet)
	 */
	FORCE_INLINE void storeToXML(pugi::xml_node& xmlNode) const {
		static_cast<MGE::Any::placeholder*>(mContent)->storeToXML(xmlNode);
	}
	
	/// store in xml – stream operator
	friend FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const MGE::Any& any) {
		any.storeToXML(xmlNode);
		return xmlNode;
	}
	
	/**
	 * @brief restore/load value from xml node (using only value of node, ignoring name and atributes)
	 * 
	 * @param xmlNode   xml node with value used to create any
	 */
	template<typename T> static const MGE::Any getAnyFromXML(const pugi::xml_node& xmlNode) {
		return MGE::Any(MGE::XMLUtils::getValue<T>(xmlNode));
	}
	
	/**
	 * @brief restore/load list of value from xml node (using only value of node, ignoring name and atributes)
	 * 
	 * @param xmlNode   xml node with value used to create any
	 */
	template<typename T> static const MGE::Any getAnyListFromXML(const pugi::xml_node& xmlNode) {
		std::list<T> tmp;
		auto ret = MGE::Any(tmp);
		MGE::XMLUtils::getListOfValues<T>(xmlNode, ret.getValuePtr<std::list<T>>());
		return ret;
	}
	
	/**
	 * @brief restore/load maps of key-value from xml node (using only value of node, ignoring name and atributes)
	 * 
	 * @param xmlNode   xml node with value used to create any
	 */
	template<typename K, typename V> static const MGE::Any getAnyMapFromXML(const pugi::xml_node& xmlNode) {
		std::map<K, V> tmp;
		auto ret = MGE::Any(tmp);
		MGE::XMLUtils::getMapOfValues<K, V>(xmlNode, ret.getValuePtr<std::map<K, V>>());
		return ret;
	}

#ifdef __DOCUMENTATION_GENERATOR__
	/** 
	 * @name Public Member Functions inherited from Ogre::Any
	 * 
	 * @{
	 */
		/// swap with other Any
		Any& swap(Any & rhs);
		/// set Any value
		template<typename ValueType> Any& operator=(const ValueType & rhs);
		/// set Any value from other Any
		Any& operator=(const Any & rhs);
		
		/// return true if Any is empty
		bool isEmpty() const;
		/// return type of Any value
		const std::type_info& getType() const;
		
		/// get Value from Any with checking type
		template<typename ValueType> ValueType operator()() const;
		
		/// get Value from Any with checking type
		template <typename ValueType> ValueType get(void) const;
		
		/// write to std::ostream (to string converter)
		inline friend std::ostream& operator << ( std::ostream& o, const Any& v );
		/// destroy
		void destroy();
	/**
	 * @}
	 */
#endif
	
	/// empty Any object
	static const Any EMPTY;
	
protected:
#ifndef __DOCUMENTATION_GENERATOR__
	class placeholder : public Ogre::Any::placeholder {
	public:
		virtual ~placeholder() = default;
		virtual void storeToXML(pugi::xml_node& xmlNode) const = 0;
	};
	
	template<typename ValueType> class holder : public placeholder {
	public:
		holder(const ValueType & value) : held(value) {
		}
		
		virtual const std::type_info & getType() const override {
			return typeid(ValueType);
		}
		
		virtual placeholder * clone() const override {
			return OGRE_NEW_T(holder, Ogre::MEMCATEGORY_GENERAL)(held);
		}
		
		virtual void writeToStream(std::ostream& stream) override {
			writeAnyValueToStream(stream, held);
		}
		
		virtual void storeToXML(pugi::xml_node& xmlNode) const override {
			writeAnyValueToXML(xmlNode, held);
		}
		
		ValueType held;
	};
#endif
};

/// @}

}
