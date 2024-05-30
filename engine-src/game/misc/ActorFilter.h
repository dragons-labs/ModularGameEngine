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

#include "data/structs/BaseActor.h"

#include "data/property/PropertyFilter.h"
#include "game/actorComponents/SelectableObject.h"

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

typedef MGE::LogicFilter<const MGE::NamedObject*> ActorLogicFilter;

/**
 * @brief class for actor filtering
 */
struct ActorFilter {
	/// selection mask (will be OR with default filter and other addional filter mask)
	MGE::SelectableObject::status_t selectionMask;
	/// selection compare value for (selection_status & mask) resluts (will be OR with default filter and other addional filter compare value)
	MGE::SelectableObject::status_t selectionMaskCompreValue;
	
	/// actor property and components filter
	ActorLogicFilter*  actorFilter;
	
	/// do actorFilter check
	bool check(const MGE::NamedObject* obj) const {
		if (actorFilter)
			return actorFilter->check(obj);
		else
			return true;
	}
	
	/// do actorFilter and selectionMask* check
	bool fullCheck(const MGE::NamedObject* obj) const;
	
	/// default constructor
	ActorFilter();
	
	/// move constructor
	ActorFilter(ActorFilter&&);
	
	/// constructor from xml config node
	ActorFilter(const pugi::xml_node& xmlNode);
	
	/// load from xml config node
	void loadFromXML(const pugi::xml_node& xmlNode);
	
	/// destructor
	~ActorFilter();
	
	
	/// deleted copy constructor
	ActorFilter(const ActorFilter&)            = delete;
	
	/// deleted copy assignment operator
	ActorFilter& operator=(const ActorFilter&) = delete;
	
	/// deleted move assignment operator
	ActorFilter& operator=(ActorFilter&&)      = delete;
};

/**
 * @brief class for actor filtering by property value
 */
struct ActorPropertyFilter : public ActorLogicFilter {
	/// actor property filter object
	MGE::PropertyFilter propertyFilter;
	
	/// when true, run propertyFilter on actor owned objects (insted of actor)
	bool onOwnedObject;
	
	/// constructor from xml node
	ActorPropertyFilter(const pugi::xml_node& xmlNode);
	
	/// get property from @a obj, compare with stored value and return results
	virtual bool check(const MGE::NamedObject* obj) const override;
	
	/// static function to create ActorPropertyFilter object
	static ActorLogicFilter* create(const pugi::xml_node& xmlNode);
};

/**
 * @brief class for actor filtering by available components
 */
struct ActorComponentFilter : public ActorLogicFilter {
	/// actor property filter object
	std::set<int> requiredComponents;
	
	/// when true required availability of ALL components from requiredComponents
	bool requiredAll;
	
	/// when true, run propertyFilter on actor owned objects (insted of actor)
	bool onOwnedObject;
	
	/// constructor from xml node
	ActorComponentFilter(const pugi::xml_node& xmlNode);
	
	/// check availability of required components
	virtual bool check(const MGE::NamedObject* obj) const override;
	
	/// static function to create ActorComponentFilter object
	static ActorLogicFilter* create(const pugi::xml_node& xmlNode);
	
protected:
	/// do real check on providet object (main actor or single owned object)
	bool _check(const MGE::NamedObject* obj) const;
};

/// @}

}
