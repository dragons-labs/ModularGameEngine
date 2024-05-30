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

#include "game/actions/ActionPrototype.h"

#include "data/property/G11n.h"

#include "data/structs/BaseActor.h"
#include "game/actions/ActionFactory.h"

/**
@page XMLSyntax_Misc

@subsection XMLNode_Action \<Action\>

Each @c \<Action\> node have attributes:
  - @c name
    - name of action
    - for action with @a type == RUN_SCRIPT also name of python function to execute, script should take 3 arguments:
      -# pointer to actor executed action
      -# pointer to action object
      -# time from last action queue proccessing (in milisecond of game time)
  - @c type
    - type of action
    - string parsed with @ref MGE::ActionPrototype::stringToActionTypeFlag and @ref MGE::Utils::stringToNumericMask – can be a space delimited list of flags
    - strings is literally identical to @ref MGE::ActionPrototype::ActionType enum elements names
    - default RUN_SCRIPT
  - @c scriptOnStart
    - name of script to execute when action started (is first time executed), before was executed, script should take 2 arguments:
      -# pointer to actor executed action
      -# pointer to action object
      .
      script should return numeric value (1 == init OK, 2 == init must be call again, 3 == init fail, see @ref MGE::Action::InitState),
      when script fail (e.g. throw exception) or return 3 (action can't be run) then ActionExecutor clear ActionQueue of this actor
    - default empty string ( == not run script)
  - @c scriptOnEnd
    - name of script to execute when started action was finished / destroyed, script should take 2 arguments:
      -# pointer to actor executed action
      -# pointer to action object
    - default empty string ( == not run script)
    - script is executed only on started actions – after scriptOnStart (if not empty) was run and return INIT_DONE_OK
  - @c needMask
    - mask of action target "need" type
    - string parsed with @ref MGE::ActionPrototype::stringToTargetTypeFlag and @ref MGE::Utils::stringToNumericMask – can be a space delimited list of flags
    - strings is literally identical to @ref MGE::ActionPrototype::TargetType enum elements names
    - default empty
  - @c menuText
    - text to display in action menu
    - 
  - @c menuIcon
    - icon to show in action menu
  .
And subnodes:
  - set of @c \<MenuText\> subnodes with @c lang attribute and containing text to show in menu entry
    - when can't find correct language value try use subnode without @c lang attribute, when not found use action name (@c name attribute of @c \<Action\> node).
  - @c \<ExecutorActorFilter\>
    - optional filter for actor whio can execute this action
    - see @ref ActorFilter for filter syntax
  - @c \<TargetActorFilter\>
    - optional filter used to target object when @a needMask contains @c SINGLE_ACTOR or @c MULTIPLE_ACTORS
    - see @ref ActorFilter for filter syntax
  - @c \<SubAction\>
    - option subaction menu entry
    - can be used multiple times
    - must have attribute @c mode with a numeric value identifying the sub-action (selected sub-menu entry) passed to @ref MGE::Action::mode of created action
    - should have set of @c \<MenuText\> subnodes with @c lang attribute and containing text to show in sub-menu entry
      - when can't find correct language value try use subnode without @c lang attribute, when not found use subaction mode (@c mode attribute of @c \<SubAction\> node).
*/

MGE::ActionPrototype::ActionPrototype( const pugi::xml_node& xmlNode ) {
	// get name
	name = xmlNode.attribute("name").as_string();
	
	// get type
	type = MGE::StringUtils::stringToNumericMask<uint32_t>(
		xmlNode.attribute("type").as_string("RUN_SCRIPT"),
		&MGE::ActionPrototype::stringToActionTypeFlag
	);
	
	// get "on start" and "on finish" script name
	scriptOnStart = xmlNode.attribute("scriptOnStart").as_string();
	scriptOnEnd = xmlNode.attribute("scriptOnEnd").as_string();
	
	// get need mask
	needMask = MGE::StringUtils::stringToNumericMask<uint32_t>(
		xmlNode.attribute("needMask").as_string(),
		&MGE::ActionPrototype::stringToTargetTypeFlag
	);
	
	// get target actor filter
	if (needMask & MGE::ActionPrototype::NEED_SELECTABLE_ACTOR) {
		targetFilter.selectionMask            = MGE::SelectableObject::IS_HIDDEN | MGE::SelectableObject::IS_UNAVAILABLE | MGE::SelectableObject::IS_SELECTABLE;
		targetFilter.selectionMaskCompreValue = MGE::SelectableObject::IS_SELECTABLE;
	} else if (needMask & MGE::ActionPrototype::NEED_ACTOR) {
		targetFilter.selectionMask            = MGE::SelectableObject::IS_HIDDEN | MGE::SelectableObject::IS_UNAVAILABLE | MGE::SelectableObject::IS_ACTION_TARGET;
		targetFilter.selectionMaskCompreValue = MGE::SelectableObject::IS_ACTION_TARGET;
	}
	auto xmlSubNode = xmlNode.child("TargetActorFilter");
	if (xmlSubNode) {
		targetFilter.loadFromXML(xmlSubNode);
	}
	
	// get executor actor filter
	xmlSubNode = xmlNode.child("ExecutorActorFilter");
	if (xmlSubNode) {
		executorFilter.loadFromXML(xmlSubNode);
	}
	
	// get more info
	menuText = MGE::G11n::getLocaleStringFromXML( xmlNode, "MenuText", name.c_str() );
	menuIcon = xmlNode.attribute("menuIcon").as_string();
	
	// get sub-menu entries
	xmlSubNode = xmlNode.child("SubAction");
	if (xmlSubNode) {
		subMenuText = new std::map<int, std::string>;
		for (; xmlSubNode; xmlSubNode = xmlSubNode.next_sibling("SubAction")) {
			subMenuText->insert(std::make_pair(
				xmlSubNode.attribute("mode").as_int(0),
				MGE::G11n::getLocaleStringFromXML(
					xmlSubNode, "MenuText",
					xmlSubNode.attribute("mode").as_string()
				)
			));
		}
	} else {
		subMenuText = NULL;
	}
}

MGE::ActionPrototype::~ActionPrototype() {
	if (subMenuText)
		delete subMenuText;
}

bool MGE::ActionPrototype::actorCanEmmitAction(
	const MGE::BaseActor* actor, const std::string_view& actionName, bool fullCheck
) {
	auto propList = actor->getPropertyValue< std::list<std::string> >("PosibleActions", {});
	for (auto& iter : propList) {
		if (iter == actionName) {
			if (fullCheck)
				return MGE::ActionFactory::getPtr()->getAction(actionName)->executorFilter.fullCheck(actor);
			return true;
		}
	}
	return false;
}
