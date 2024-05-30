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

#include "game/misc/ActorFilter.h"

#include "data/property/LogicFilter.inl"
#include "data/property/PropertyFilter.inl"

#include "data/structs/BaseActor.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/structs/components/ObjectOwner.h"

/*--------------------- ActorFilter ---------------------*/

MGE::ActorFilter::ActorFilter() :
	selectionMask(0),
	selectionMaskCompreValue(0),
	actorFilter(NULL)
{}

MGE::ActorFilter::ActorFilter(MGE::ActorFilter&& src) :
	selectionMask(src.selectionMask),
	selectionMaskCompreValue(src.selectionMaskCompreValue),
	actorFilter(src.actorFilter)
{
	src.actorFilter = NULL;
}

MGE::ActorFilter::ActorFilter(const pugi::xml_node& xmlNode) :
	selectionMask(0),
	selectionMaskCompreValue(0),
	actorFilter(NULL)
{
	loadFromXML(xmlNode);
}

/**
@page XMLSyntax_Filter

@subsection ActorFilter Actor Filter

\<ActorFilter\> element in this case:
  - support optional attributes
    - @b selectionMask (default 0)
    - @b selectionMaskCompreValue (default 0)
  - can have node value with one \<Filter\> element used to create @ref MGE::ActorPropertyFilter, @ref MGE::ActorComponentFilter or @ref MGE::LogicExpression

Values of @em selectionMask and @em selectionMaskCompreValue attributes are interpreted as SelectableObject status mask (see MGE::SelectableObject::StatusFlags for used values)
by MGE::SelectableObject::stringToStatusMask (so they are a space separated list to make bitwise OR and can contain numeric values or flag names).

Interpreted values of @em selectionMask and @em selectionMaskCompreValue are used to filter by compare bitwise AND of selectable_status and @em selectionMask with @em selectionMaskCompreValue.

@subsubsection ActorComponentExample Example
@code{.xml}
<ActorFilter selectionMask="IS_SELECTABLE IS_HIDDEN" selectionMaskCompreValue="0x00">
  <Filter requiredComponents="World3D ActionQueue" requiredMode="any"/>
</ActorFilter>
@endcode <br/>
*/

void MGE::ActorFilter::loadFromXML(const pugi::xml_node& xmlNode) {
	static std::map<std::string, MGE::ActorLogicFilter::FilterCreator> filtersMap = {
		{ MGE::EMPTY_STRING,       &MGE::ActorPropertyFilter::create  },
		{ "requiredComponents"s,   &MGE::ActorComponentFilter::create }
	};
	
	pugi::xml_attribute xmlAttrib;
	if ( (xmlAttrib = xmlNode.attribute("selectionMask")) )
		selectionMask            = MGE::SelectableObject::stringToStatusMask( xmlAttrib.as_string() );
	if ( (xmlAttrib = xmlNode.attribute("selectionMaskCompreValue")) )
		selectionMaskCompreValue = MGE::SelectableObject::stringToStatusMask( xmlAttrib.as_string() );
	
	auto xmlSubNode = xmlNode.child("Filter");
	if (xmlSubNode)
		actorFilter = ActorLogicFilter::create(xmlSubNode, filtersMap); // this is the same as: MGE::LogicFilter<const MGE::NamedObject*>::create(...)
}

bool MGE::ActorFilter::fullCheck(const MGE::NamedObject* obj) const {
	if (selectionMask) {
		const MGE::SelectableObject* selectableObj = obj->getComponent<MGE::SelectableObject>();
		if (!selectableObj || (selectableObj->status & selectionMask) != selectionMaskCompreValue)
			return false;
	}
	return check(obj);
}

MGE::ActorFilter::~ActorFilter() {
	delete actorFilter;
}


/*--------------------- ActorPropertyFilter ---------------------*/

/**
@page XMLSyntax_Filter

@subsection ActorPropertyFilter Actor Property Filter

Is extended version of @ref PropertyFilter \<Filter\> element. It support one addional, optional attribute:
    - @b onOwnedObject when exist and set to "1", "yes" or "true" perform this filter on each object from ObjectOwner component
                       return true if any of owned objects fulfills filter conditions

@subsubsection ActorPropertyFilterExample Example
@code{.xml}
<Filter propertyName="numeric property" valueType="int" condition="LESS" onOwnedObject="true">17</Filter>
@endcode <br/>
*/

MGE::ActorPropertyFilter::ActorPropertyFilter(const pugi::xml_node& xmlNode) {
	onOwnedObject = xmlNode.attribute("onOwnedObject").as_bool(false);
	propertyFilter.loadFromXML(xmlNode);
}

bool MGE::ActorPropertyFilter::check(const MGE::NamedObject* obj) const {
	if (onOwnedObject) {
		auto objectOwner = obj->getComponent<MGE::ObjectOwner>();
		if (!objectOwner)
			return false;
		for (auto& iter : *objectOwner) {
			if (propertyFilter.check(iter.first))
				return true;
		}
		return false;
	} else {
		return propertyFilter.check(obj);
	}
}

MGE::ActorLogicFilter* MGE::ActorPropertyFilter::create(const pugi::xml_node& xmlNode) {
	MGE::ActorLogicFilter* filterObj = new MGE::ActorPropertyFilter(xmlNode);
	return filterObj;
}


/*--------------------- ActorComponentFilter ---------------------*/

/**
@page XMLSyntax_Filter

@subsection ActorComponentFilter Actor Component Filter

\<Filter\> element with requiredComponents attribute realizes actors components set filtering. \<Filter\> element in this case:
  - required attributes:
    - @b requiredComponents with space separated list of components names or numeric IDs
  - support optional attributes
    - @b requiredMode when exist and is equal "any" or "ANY" filter return true when any of components from @em requiredComponents is present in actor
                      otherwise required present of all components
    - @b onOwnedObject when exist and set to "1", "yes" or "true" perform this filter on each object from ObjectOwner component
                       return true if any of owned objects fulfills filter conditions
  - do not have any value

@subsubsection ActorComponentFilterExample Example
@code{.xml}
<Filter requiredComponents="World3D ActionQueue" requiredMode="any"/>
@endcode <br/>
*/

MGE::ActorComponentFilter::ActorComponentFilter(const pugi::xml_node& xmlNode) {
	onOwnedObject = xmlNode.attribute("onOwnedObject").as_bool(false);
	
	std::istringstream  valueStream( xmlNode.attribute("requiredComponents").as_string() );
	std::string         valueSubStr;
	while (std::getline (valueStream, valueSubStr, ' ')) {
		int newVal = MGE::ComponentFactory::getPtr()->getID(valueSubStr.c_str());
		if (newVal > 0) {
			requiredComponents.insert(newVal);
		}
	}
	
	std::string_view mode = xmlNode.attribute("requiredMode").as_string();
	requiredAll = !(mode == "any" || mode == "ANY");
}

bool MGE::ActorComponentFilter::_check(const MGE::NamedObject* obj) const {
	for (auto& iter : requiredComponents) {
		const MGE::BaseComponent* component = obj->getComponent(iter);
		if (component && !requiredAll)
			return true;
		else if (!component && requiredAll)
			return false;
	}
	return requiredAll;
}

bool MGE::ActorComponentFilter::check(const MGE::NamedObject* obj) const {
	if (onOwnedObject) {
		auto objectOwner = obj->getComponent<MGE::ObjectOwner>();
		if (!objectOwner)
			return false;
		for (auto& iter : *objectOwner) {
			if (_check(iter.first))
				return true;
		}
		return false;
	} else {
		return _check(obj);
	}
}

MGE::ActorLogicFilter* MGE::ActorComponentFilter::create(const pugi::xml_node& xmlNode) {
	MGE::ActorLogicFilter* filterObj = new MGE::ActorComponentFilter(xmlNode);
	return filterObj;
}
