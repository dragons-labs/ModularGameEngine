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

#include "LogicFilter.h"
#include "LogSystem.h"
#include <regex>

namespace MGE {

/*--------------------- Compare::Functor variants for various OperationTypes ---------------------*/

template <typename TypeA, typename TypeB, int OperationType> struct Compare::Functor {
	inline static bool compare(const TypeA& a, const TypeB& b) {
		throw std::logic_error("Call Compare::Functor with invalid OperationType");
	}
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::EQUAL> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a == b; }
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::NOT_EQUAL> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a != b; }
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::LESS> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a < b; }
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::GREATER> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a > b; }
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::LESS_EQUAL> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a <= b; }
};
template <typename TypeA, typename TypeB> struct Compare::Functor<TypeA, TypeB, Compare::GREATER_EQUAL> {
	inline static bool compare(const TypeA& a, const TypeB& b) { return a >= b; }
};

// StrTypeA and StrTypeB should be string types (std::string, std::string_view, ...)

template <typename StrTypeA, typename StrTypeB> struct Compare::Functor<StrTypeA, StrTypeB, Compare::CONTAINS_WORD> { 
	inline static bool compare(const StrTypeA& a, const StrTypeB& b) {
		std::istringstream valueStream(a);
		std::string        valueSubStr;
		while (std::getline (valueStream, valueSubStr, ' ')) {
			if (b == valueSubStr)
				return true;
		}
		return false;
	}
};
template <typename StrTypeA, typename StrTypeB> struct Compare::Functor<StrTypeA, StrTypeB, Compare::NOT_CONTAINS_WORD> {
	inline static bool compare(const StrTypeA& a, const StrTypeB& b) {
		return ! Compare::Functor<StrTypeA, StrTypeB, Compare::CONTAINS_WORD>::compare(a, b);
	}
};
template <typename StrTypeA> struct Compare::Functor<StrTypeA, std::regex, Compare::MATCH> {
	inline static bool compare(const StrTypeA& a, const std::regex& b) {
		return std::regex_match(a, b);
	}
};
template <typename StrTypeA> struct Compare::Functor<StrTypeA, std::regex, Compare::NOT_MATCH> {
	inline static bool compare(const StrTypeA& a, const std::regex& b) {
		return ! std::regex_match(a, b);
	}
};


/*--------------------- LogicFilter template implementation ---------------------*/

template<typename FilteredObjectType> LogicFilter<FilteredObjectType>* LogicFilter<FilteredObjectType>::create(
	const pugi::xml_node& xmlNode,
	const std::map<std::string, FilterCreator>& filtersMap
) {
	/// try filter expression
	if (xmlNode.attribute("filterExpression")) {
		return new LogicExpression<FilteredObjectType>(xmlNode, filtersMap);
	} else {
		FilterCreator defaultFilterCreator = NULL;
		
		// find registred filter class dedicated for this node
		for (auto& iter : filtersMap) {
			if (iter.first.empty())
				defaultFilterCreator = iter.second;
			else if (xmlNode.attribute(iter.first.c_str())) {
				return iter.second(xmlNode);
			}
		}
		
		// if not found dedicated filter class for this node, try default filter
		if (defaultFilterCreator) {
			return defaultFilterCreator(xmlNode);
		}
	}
	
	LOG_WARNING("Can't create LogicFilter for XML node");
	return NULL;
}


/*--------------------- LogicExpression template implementation ---------------------*/

template<typename FilteredObjectType> bool LogicExpression<FilteredObjectType>::check(FilteredObjectType obj) const {
	bool result = (operation == AND);
	// default result for:
	//  AND => true  (stop loop on first false)
	//  OR  => false (stop loop on first true)
	//  XOR => false (change result on first true)
	
	for (auto& iter : elements) {
		bool subResult = iter->check(obj);
		if (operation == AND && !subResult) {
			// false element in AND => expression is false
			result = false;
			break;
		} else if (operation == OR && subResult) {
			// true element in OR => expression is true
			result = true;
			break;
		} else if (operation == XOR && subResult) {
			if (result) {
				// second true value in XOR => false
				result = false;
				break;
			}
			// first true value in XOR => set results to true
			result = true;
		}
	}
	
	if (isNegated)
		return !result;
	else
		return result;
}

/**
@page XMLSyntax_Filter

@subsection LogicExpression Logic Expression

\<Filter\> element with filterExpression attribute realizes multi-argument logic function (AND, OR, XOR, NAND, NOR, NXOR). \<Filter\> element in this case:
  - required attributes:
    - @b filterExpression with one of folowin value:
      - "or" or "OR"   for logic @b OR function  (return true when any of sub-filters return true)
      - "and" or "AND" for logic @b AND function (return true only when all sub-filters return true)
      - "xor" or "XOR" for logic @b XOR function (return true when only one all of sub-filters return true)
  - support optional attributes
    - @b filterIsNegated when exist and set to "1", "yes" or "true" return negation of function specifies by @em filterExpression
  - contains at least one \<Filter\> sub-element defining sub-filter

@subsubsection LogicExpressionExample Example
@code{.xml}
<Filter filterExpression="AND">
  <Filter ... />
  <Filter ... />
  <Filter filterExpression="OR" filterIsNegated="true">
    <Filter ... />
    <Filter ... />
  </Filter>
  <Filter ... />
</Filter>
@endcode <br/>
*/
template<typename FilteredObjectType> LogicExpression<FilteredObjectType>::LogicExpression(
	const pugi::xml_node& xmlNode,
	const std::map<std::string, typename LogicFilter<FilteredObjectType>::FilterCreator>& filtersMap
) {
	std::string_view type = xmlNode.attribute("filterExpression").as_string();
	if (type == "and" || type == "AND") {
		operation = AND;
	} else if (type == "or" || type == "OR") {
		operation = OR;
	} else if (type == "xor" || type == "XOR") {
		operation = XOR;
	} else {
		throw std::logic_error("Unsupported operation in filterExpression");
	}
	isNegated = xmlNode.attribute("filterIsNegated").as_bool(false);
	
	for (auto xmlSubNode : xmlNode.children("Filter")) {
		LogicFilter<FilteredObjectType>* subFilter = LogicFilter<FilteredObjectType>::create(xmlSubNode, filtersMap);
		if (subFilter) {
			elements.push_back(subFilter);
		}
	}
}

template<typename FilteredObjectType> LogicExpression<FilteredObjectType>::~LogicExpression() {
	for (auto& iter : elements) {
		delete iter;
	}
}

}
