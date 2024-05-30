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

#include "StringUtils.h"

#include <string>
#include <list>
#include <map>

namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup PropertySystem
/// @{
/// @file

/**
 * @brief Base class for all logic filters.
 */
template <typename FilteredObjectType> struct LogicFilter {
	/// type of static function for creating filter
	/// used in @a filtersMap argument in @ref create
	typedef LogicFilter<FilteredObjectType>* (*FilterCreator)(const pugi::xml_node& xmlNode);
	
	/**
	 * @brief funtion for create LogicFilter object using XML config and @a filtersMap
	 * 
	 * @param  xmlNode     xml node to parse for creating filter
	 * @param  filtersMap  map of {required xml attribute name} -\> {filter creation function} used to parsing xml nodes of filter
	 *                     empty key string means default filter creation function
	 * 
	 * @note @ref LogicExpression is internally supported, so it does not have to be present in the map
	 */
	static LogicFilter<FilteredObjectType>* create(
		const pugi::xml_node& xmlNode,
		const std::map<std::string, FilterCreator>& filtersMap
	);
	
	/// run filter and return results
	virtual bool check(FilteredObjectType obj) const = 0;
	
	/// destructor
	virtual ~LogicFilter() = default;
};

/**
 * @brief Class implemented logic expression.
 *        Support multi-argument AND, OR, XOR operation and result negation.
 */
template <typename FilteredObjectType> struct LogicExpression : LogicFilter<FilteredObjectType> {
	/// enum with logic operation types ids
	enum LogicOperators { AND = 1, OR, XOR };
	
	/// logic operation to do betwen expression elemets
	LogicOperators operation;
	
	/// flag indicating negation of expression results
	bool isNegated;
	
	/// store all expression elemets
	std::list< LogicFilter<FilteredObjectType>* >  elements;
	
	/// run filter and return results
	virtual bool check(FilteredObjectType obj) const override;
	
	/// constructor from xml config node
	LogicExpression(
		const pugi::xml_node& xmlNode,
		const std::map<std::string, typename LogicFilter<FilteredObjectType>::FilterCreator>& filtersMap
	);
	
	// destructor
	virtual ~LogicExpression();
};

/**
 * @brief Class for functor comparisons.
 */
struct Compare {
	/// enum with operation types ids
	enum OperationTypes { EQUAL = 1, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, CONTAINS_WORD, NOT_CONTAINS_WORD, MATCH, NOT_MATCH };
	
	/// convert string to OperationTypes value
	static int strintToOperationType(const std::string_view& str) {
		if      (str == "EQUAL")             return EQUAL;
		else if (str == "NOT_EQUAL")         return NOT_EQUAL;
		else if (str == "LESS")              return LESS;
		else if (str == "GREATER")           return GREATER;
		else if (str == "LESS_EQUAL")        return LESS_EQUAL;
		else if (str == "GREATER_EQUAL")     return GREATER_EQUAL;
		else if (str == "CONTAINS_WORD")     return CONTAINS_WORD;
		else if (str == "NOT_CONTAINS_WORD") return NOT_CONTAINS_WORD;
		else if (str == "MATCH")             return MATCH;
		else if (str == "NOT_MATCH")         return NOT_MATCH;
		return 0;
	}
	
	/// compare functor-like template
	/// @note this is not real functor, because use static compare() member function insted of operator()
	template <typename TypeA, typename TypeB, int OperationType> struct Functor;
};

/// @}

}
