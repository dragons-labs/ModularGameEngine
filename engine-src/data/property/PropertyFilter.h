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

#include "data/property/PropertySetInterface.h"
#include "data/property/LogicFilter.h"

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

/**
 * @brief Interface base class for any comparison object,
 *        derived class determinate value type and compare operation.
 */
struct CompareAnyInterface {
	virtual bool compare(const MGE::Any& any) const = 0;
	virtual ~CompareAnyInterface() = default;
	
	/// helper funtion for creating CompareAny object
	template <typename ValueType, typename InitValueType> static CompareAnyInterface* createCompareAny(int conditionID, const InitValueType& value);
};

/**
 * @brief Class template for derived from CompareAnyInterface classes.
 * 
 * @note  This is only forward declaration, for using it include "PropertyFilter.inl" file.
 */
template <typename ValueType, int OperationType> struct CompareAny;

/**
 * @brief Class to store and do property-based filtering.
 */
template <typename FilteredObjectType> class PropertyFilterTemplate : public MGE::LogicFilter<FilteredObjectType> {
public:
	/// default constructor
	PropertyFilterTemplate();
	
	/// move constructor
	PropertyFilterTemplate(PropertyFilterTemplate&&);
	
	/// constructor from xml config node
	PropertyFilterTemplate(const pugi::xml_node& xmlNode);
	
	/// constructor
	template <typename ValueType> PropertyFilterTemplate(
		const std::string& _propertyName, const ValueType& _value, int _operationType
	);
	
	/// destructor
	~PropertyFilterTemplate();
	
	
	/// deleted copy constructor
	PropertyFilterTemplate(const PropertyFilterTemplate&)            = delete;
	
	/// deleted copy assignment operator
	PropertyFilterTemplate& operator=(const PropertyFilterTemplate&) = delete;
	
	/// deleted move assignment operator
	PropertyFilterTemplate& operator=(PropertyFilterTemplate&&)      = delete;
	
	
	/// static function to create PropertyFilterTemplate object
	static MGE::LogicFilter<FilteredObjectType>* create(const pugi::xml_node& xmlNode);
	
	/// load from xml config node
	void loadFromXML(const pugi::xml_node& xmlNode);
	
	/// get property from @a obj, compare with stored value and return results
	virtual bool check(FilteredObjectType obj) const override;
	
protected:
	/// name of property to get for comparations
	std::string propertyName;
	
	/// comparations value and operation
	CompareAnyInterface* value;
};

typedef PropertyFilterTemplate<const PropertySetInterface*> PropertyFilter;

/// @}

}
