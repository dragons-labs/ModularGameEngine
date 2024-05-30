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

-- Based on / Inspired by: Ogre Property Component, original copyright information follows --
Copyright (c) 2000-2014 Torus Knot Software Ltd
Distributed under the MIT license terms (see ADDITIONAL_COPYRIGHT/Ogre.txt file)
-- End original copyright --
*/

#pragma   once

#include <string>
#include <string_view>

namespace MGE { class  Any; }

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

/**
 * @brief Interface for property sets objects.
 * 
 * @note Depending on use case under this interface can be single or multiple (e.g. own (read-write) and read-only from parent) MGE::PropertySet objects.
 */
template <typename AnyClass = MGE::Any> struct PropertySetInterfaceTmpl {
	/**
	 * @brief check if property identifying by key is set and return reference to Any value
	 *        when property is not set return AnyClass::EMPTY
	 * 
	 *        directly call of AnyClass::getValue(const ValueType& defVal) on returned object is safety,
	 *        but directly call AnyClass::getValue() is not safety (need check AnyClass::isEmpty() first)
	 * 
	 * @param[in]  key     property name
	 */
	virtual const AnyClass& getProperty(const std::string_view& key) const = 0;
	
	/**
	 * @brief return value from property identifying by @a key, when not found, return @a defVal
	 */
	template<typename ValueType> inline ValueType getPropertyValue(const std::string_view& key, ValueType defVal) const {
		return getProperty(key).getValue(defVal);
	}
	
	/**
	 * @brief return true when property identifying by key is set, otherwise return false
	 *
	 * @param[in]  key     property name
	 */
	virtual bool hasProperty(const std::string_view& key) const {
		return !getProperty(key).isEmpty();
	}
	
	/**
	 * @brief remove property identifying by key
	 *
	 * @param[in]  key     property name
	 *
	 * @return number of remove properties (0 or 1)
	 */
	virtual size_t remProperty(const std::string_view& key) = 0;
	
	/**
	 * @brief add property identifying by key, with Any value
	 *
	 * @param[in]  key     property name
	 * @param[in]  val     property value (will be copy)
	 * @param[in]  replace when true replace value in existed property
	 *
	 * @return true when property is set or update, false when property existed and not update (becasuse @a replace == false)
	 */
	virtual bool addProperty(const std::string_view& key, const AnyClass& val, bool replace = false) = 0;
	
	/**
	 * @brief add property identifying by key, with @a ValueType value
	 *
	 * @param[in]  key     property name
	 * @param[in]  val     property value (will be copy and put in Any)
	 * @param[in]  replace when true replace value in existed property
	 *
	 * @return true when property is set or update, false when property existed and not update (becasuse @a replace == false)
	 */
	template<typename ValueType> requires (!std::same_as<ValueType, AnyClass>) bool addProperty(const std::string_view& key, const ValueType& val, bool replace = false) {
		return addProperty(key, AnyClass(val), replace);
	}
	
	/**
	 * @brief set property identifying by key, with Any value
	 *
	 * @param[in]  key     property name 
	 * @param[in]  val     property value (will be copy) 
	 *
	 * @return true when property is found and update, false when property not existed
	 */
	virtual bool setProperty(const std::string_view& key, const AnyClass& val) = 0;
	
	/**
	 * @brief set property identifying by key, with @a ValueType value
	 *
	 * @param[in]  key     property name
	 * @param[in]  val     property value (will be copy and put in Any)
	 *
	 * @return true when property is found and update, false when property not existed
	 */
	template<typename ValueType> bool setProperty(const std::string_view& key, const ValueType& val) {
		return setProperty(key, AnyClass(val));
	}
	
	/// destructor
	virtual ~PropertySetInterfaceTmpl() = default;
};

using PropertySetInterface = PropertySetInterfaceTmpl<>;

/// @}

}
