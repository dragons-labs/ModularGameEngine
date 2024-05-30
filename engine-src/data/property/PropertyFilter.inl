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

/// @addtogroup PropertySystem
/// @file

#include "data/property/PropertyFilter.h"
#include "data/property/LogicFilter.inl"

#include "LogSystem.h"

namespace MGE {

/*--------------------- CompareAny variants for various ValueType ---------------------*/

template <typename ValueType, int OperationType> struct CompareAny : CompareAnyInterface {
	ValueType value;
	
	CompareAny(const ValueType& v) : value(v) {}
	
	virtual bool compare(const MGE::Any& any) const override final {
		return Compare::Functor<ValueType, ValueType, OperationType>::compare(
			any.getValue<ValueType>(), value
		);
	}
};

template <int OperationType> struct CompareAny<std::string, OperationType> : CompareAnyInterface {
	std::string value;
	
	CompareAny(const std::string& v)      : value(v) {}
	CompareAny(const std::string_view& v) : value(v) {}
	
	virtual bool compare(const MGE::Any& any) const override final {
		return Compare::Functor<std::string, std::string, OperationType>::compare(
			any.getValue<std::string>(), value
		);
	}
};

template <int OperationType> struct CompareAny<std::regex, OperationType> : CompareAnyInterface {
	std::regex propertyRegex;
	
	CompareAny(const std::string& v)      : propertyRegex(v) {}
	CompareAny(const std::string_view& v) : propertyRegex(static_cast<std::string>(v)) {}
	
	virtual bool compare(const MGE::Any& any) const override final {
		return Compare::Functor<std::string, std::regex, OperationType>::compare(
			any.getValue<std::string>(), propertyRegex
		);
	}
};


/*--------------------- createCompareAny() variants for various ValueType ---------------------*/

template <typename ValueType, typename InitValueType> CompareAnyInterface* CompareAnyInterface::createCompareAny(int conditionID, const InitValueType& value) {
	switch (conditionID) {
		case Compare::EQUAL:             return new CompareAny<ValueType, Compare::EQUAL>(value);
		case Compare::NOT_EQUAL:         return new CompareAny<ValueType, Compare::NOT_EQUAL>(value);
		case Compare::LESS:              return new CompareAny<ValueType, Compare::LESS>(value);
		case Compare::GREATER:           return new CompareAny<ValueType, Compare::GREATER>(value);
		case Compare::LESS_EQUAL:        return new CompareAny<ValueType, Compare::LESS_EQUAL>(value);
		case Compare::GREATER_EQUAL:     return new CompareAny<ValueType, Compare::GREATER_EQUAL>(value);
		default:                         throw std::logic_error("Unsupported condition in PropertyFilter XML");
	}
}

template <> CompareAnyInterface* CompareAnyInterface::createCompareAny<std::string>(int conditionID, const std::string_view& value) {
	switch (conditionID) {
		case Compare::EQUAL:             return new CompareAny<std::string, Compare::EQUAL>(value);
		case Compare::NOT_EQUAL:         return new CompareAny<std::string, Compare::NOT_EQUAL>(value);
		case Compare::CONTAINS_WORD:     return new CompareAny<std::string, Compare::CONTAINS_WORD>(value);
		case Compare::NOT_CONTAINS_WORD: return new CompareAny<std::string, Compare::NOT_CONTAINS_WORD>(value);
		default:                         throw std::logic_error("Unsupported condition in string PropertyFilter XML");
	}
}

template <> CompareAnyInterface* CompareAnyInterface::createCompareAny<std::regex>(int conditionID, const std::string_view& value) {
	switch (conditionID) {
		case Compare::MATCH:             return new CompareAny<std::regex, Compare::MATCH>(value);
		case Compare::NOT_MATCH:         return new CompareAny<std::regex, Compare::NOT_MATCH>(value);
		default:                         throw std::logic_error("Unsupported condition in regex PropertyFilter XML");
	}
}


/*--------------------- PropertyFilterTemplate check() and loadFromXML() functions ---------------------*/

template <typename FilteredObjectType> bool PropertyFilterTemplate<FilteredObjectType>::check(FilteredObjectType obj) const {
	if (!value)
		return true;
	
	auto prop = obj->getProperty(propertyName);
	
	if (prop.isEmpty()) {
		LOG_DEBUG("PropertyFilterTemplate can't find property: " << propertyName);
		return false;
	}
	
	return value->compare(prop);
}

/**
@page XMLSyntax_Filter

@subsection PropertyFilter Property Filter

\<Filter\> element with propertyName attribute realizes PropertySet filtering. \<Filter\> element in this case:
  - required attributes:
    - @b propertyName with name of property to testing
    - @b valueType    with type of value used to compare with property, supported values:
      - "int"    interpret value as integer number
      - "float"  interpret value as floating point number
      - "String" use value as string
      - "Regex"  create regexp based on value
    - @b condition    compare function, supported values:
      - for "int", "float" and "String":
        - "EQUAL"              property value is equal to filter value
        - "NOT_EQUAL"          property value is different that filter value
      - for "int" and "float":
        - "LESS"               property value is less than filter value
        - "GREATER"            property value is greather than filter value
        - "LESS_EQUAL"         property value is equal or less than filter value
        - "GREATER_EQUAL"      property value is equal or greather than filter value
      - for "String":
        - "CONTAINS_WORD"      property value contains word which is the filter value
        - "NOT_CONTAINS_WORD"  property value don't contains word which is the filter value
      - for "Regex":
        - "MATCH"              property value match to regexp
        - "NOT_MATCH"          property value don't match to regexp
  - must have node value, that is used as value to compare with property (after interpreted as type determined by @em valueType)

@subsubsection PropertyFilterExample Example
@code{.xml}
<Filter propertyName="flags list as text property" valueType="String" condition="CONTAINS_WORD">flag1</Filter>
<Filter propertyName="numeric property" valueType="int" condition="LESS">17</Filter>
<Filter propertyName="text property" valueType="Regex" condition="MATCH">.*A[BC]D</Filter>
@endcode <br/>
*/
template <typename FilteredObjectType> void PropertyFilterTemplate<FilteredObjectType>::loadFromXML(const pugi::xml_node& xmlNode) {
	propertyName          = xmlNode.attribute("propertyName").as_string();
	std::string_view type = xmlNode.attribute("valueType").as_string();
	int conditionID = Compare::strintToOperationType( xmlNode.attribute("condition").as_string() );
	
	LOG_DEBUG("PropertyFilter for propertyName=" << propertyName << " valueType=" << type << " condition=" << conditionID);
	if        (type == "int") {
		value = CompareAnyInterface::createCompareAny<int>(
			conditionID,
			xmlNode.text().as_int()
		);
	} else if (type == "float") {
		value = CompareAnyInterface::createCompareAny<float>(
			conditionID,
			xmlNode.text().as_float()
		);
	} else if (type == "String") {
		value = CompareAnyInterface::createCompareAny<std::string>(
			conditionID,
			std::string_view(xmlNode.text().as_string())
		);
	} else if (type == "Regex") {
		value = CompareAnyInterface::createCompareAny<std::regex>(
			conditionID,
			std::string_view(xmlNode.text().as_string())
		);
	} else {
		throw std::logic_error("Unsupported filterValueType in PropertyFilterTemplate XML");
	}
}


/*--------------------- PropertyFilterTemplate constructors and destructor ---------------------*/

template <typename FilteredObjectType> PropertyFilterTemplate<FilteredObjectType>::PropertyFilterTemplate() : value(NULL) {}

template <typename FilteredObjectType> PropertyFilterTemplate<FilteredObjectType>::PropertyFilterTemplate(PropertyFilterTemplate&& src) :
	propertyName(src.propertyName),
	value(src.value)
{
	src.value = NULL;
}

template <typename FilteredObjectType> PropertyFilterTemplate<FilteredObjectType>::PropertyFilterTemplate(const pugi::xml_node& xmlNode) :
	value(NULL)
{
	loadFromXML(xmlNode);
}

template <typename FilteredObjectType> template <typename ValueType> PropertyFilterTemplate<FilteredObjectType>::PropertyFilterTemplate(
	const std::string& _propertyName, const ValueType& _value, int _operationType
) :
	propertyName(_propertyName)
{
	value = CompareAnyInterface::createCompareAny<ValueType>(_operationType, _value);
}

template <typename FilteredObjectType> PropertyFilterTemplate<FilteredObjectType>::~PropertyFilterTemplate() {
	delete value;
}

template <typename FilteredObjectType> MGE::LogicFilter<FilteredObjectType>* PropertyFilterTemplate<FilteredObjectType>::create(const pugi::xml_node& xmlNode) {
	return new PropertyFilterTemplate(xmlNode);
}

}
