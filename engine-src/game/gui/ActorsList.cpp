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

#include "game/gui/ActorsList.h"

#include "ConfigParser.h"
#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"
#include "input/Selection.h"
#include "physics/GameSpeedMessages.h"
#include "data/property/G11n.h"

#include "data/structs/components/3DWorld.h"
#include "data/structs/ActorMessages.h"
#include "game/misc/PrimarySelection.h"
#include "game/actions/ActionQueue.h"
#include "game/actorComponents/SelectableObject.h"
// #include "data/property/PropertyFilter.h"
// 

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_ActorsList \<ActorsList\>

@c \<ActorsList\> is used for enabled and configure GUI (sub)widow with list of actors in scene. It have required subnodes:
	- @ref XMLNode_BaseWin

and optional subnodes:
	- @c \<BaseFilter\> for setting base filter option (requirements that an actor must meet to be listed) with attributes:
		- @c mask
		- @c value
		.
		Values of @em mask and @em value attributes are interpreted as SelectableObject status mask (see MGE::SelectableObject::StatusFlags for used values)
		by MGE::SelectableObject::stringToStatusMask (so they are a space separated list to make bitwise OR and can contain numeric values or flag names).
		
		Interpreted values of @em mask and @em value are used to filter by compare bitwise AND of selectable_status and @em mask with @em value.
		Default value of BaseFilter is: IS_SELECTABLE and not IS_HIDDEN and not IS_UNAVAILABLE and not IS_ACTION_TARGET.
		
		See too: selectionMask and selectionMaskCompreValue in @ref ActorFilter.
	- @c \<FilterA\> with subsets of @c \<ActorFilter\> nodes for settings first user selectable filter in actor window
	- @c \<FilterB\> with subsets of @c \<ActorFilter\> nodes for settings second user selectable filters in actor window

@subsubsection XMLNode_ActorsList_ActorFilter \<ActorFilter\> subnodes

@c \<ActorFilter\> subnodes of @c \<FilterA\> and/or @c \<FilterB\> use standard @ref ActorFilter, but should have additional set of @c \<Text\> subnodes
	with @c lang attribute and containing text to show in list of selectable filters (name of filter)

@subsubsection XMLNode_ActorsList_Example Example
@code{.xml}
<ActorsList>
	<BaseWin name="WorldInfoWindow" type="TabsWindow" layoutFile="WorldInfoWindow.layout" />
	<FilterA>
		<ActorFilter>
			<Text>fire truck</Text>
			<Filter propertyName="_name" valueType="Regex" condition="MATCH">.*fire truck.*</Filter>
		</ActorFilter>
		<ActorFilter text="police car">
			<Text>police car</Text>
			<Text lang="pl">samochód policyjny</Text>
			<Filter filterExpression="OR">
				<Filter propertyName="_name" valueType="Regex" condition="MATCH">.*police car.*</Filter>
				<Filter propertyName="_name" valueType="Regex" condition="MATCH">.*police truck.*</Filter>
			</Filter>
		</ActorFilter>
	</FilterA>
</ActorsList>
@endcode <br/>
*/

MGE::ActorsList::ActorsList(const pugi::xml_node& xmlNode) :
	MGE::GenericWindows::BaseWindowOwner(
		MGE::GenericWindows::Factory::getPtr()->get(xmlNode)
	),
	MGE::Unloadable(200),
	needUpdate(true),
	isVisible(false)
{
	if(!window) {
		throw std::logic_error("Could not create base window for ActorsList");
	}
	
	LOG_INFO("Initialise ActorsList based on config xml node");
	
	auto xmlSubNode = xmlNode.child("BaseFilter");
	if (xmlSubNode) {
		defMask   = MGE::SelectableObject::stringToStatusMask(xmlSubNode.attribute("mask").as_string());
		defCmpVal = MGE::SelectableObject::stringToStatusMask(xmlSubNode.attribute("value").as_string());
	} else {
		defMask   = MGE::SelectableObject::IS_SELECTABLE | MGE::SelectableObject::IS_HIDDEN | MGE::SelectableObject::IS_UNAVAILABLE | MGE::SelectableObject::IS_ACTION_TARGET;
		defCmpVal = MGE::SelectableObject::IS_SELECTABLE;
	}

	getWindow()->getChild("Units")->subscribeEvent(
		CEGUI::Window::EventShown, CEGUI::Event::Subscriber(&MGE::ActorsList::onShow, this)
	);
	getWindow()->getChild("Units")->subscribeEvent(
		CEGUI::Window::EventHidden, CEGUI::Event::Subscriber(&MGE::ActorsList::onHide, this)
	);
	
	unitsList = static_cast<CEGUI::MultiColumnList*>(getWindow()->getChild("Units")->getChild("List"));
	
	unitsList->addColumn("Image", 0, CEGUI::UDim(0, 144));
	unitsList->addColumn("Name", 1, CEGUI::UDim(1.0f, -224));
	unitsList->addColumn("QLen", 2, CEGUI::UDim(0, 64));
	unitsList->setSelectionMode(CEGUI::MultiColumnList::SelectionMode::RowMultiple);
	unitsList->subscribeEvent(
		CEGUI::MultiColumnList::EventSelectionChanged, CEGUI::Event::Subscriber(&MGE::ActorsList::unitsListSelectionChanged, this)
	);
	unitsList->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::ActorsList::unitsListDoubleClick, this)
	);
	
	filters.emplace_back();
	filterA = _configureFilter("FilterA", xmlNode);
	filterB = _configureFilter("FilterB", xmlNode);
	
	// register listeners
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, POST_RENDER_GUI);
	
	// subscribe for events message
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::ActorCreatedEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::ActorDestroyEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::PrimarySelection::SelectionChangeEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::ActionQueue::ActionQueueUpdateEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
}

CEGUI::Combobox* MGE::ActorsList::_configureFilter(MGE::null_end_string name, const pugi::xml_node& xmlNode) {
	auto filter = static_cast<CEGUI::Combobox*>(
		getWindow()->getChild("Units")->getChild(name)
	);
	filter->getDropList()->setTextColour(
			CEGUI::PropertyHelper<CEGUI::Colour>::fromString( filter->getProperty("DefaultItemTextColour") )
	);
	
	MGE::GUISystem::getPtr()->setTranslatedText(filter, "(no filter)");
	filter->addItem( new CEGUI::StandardItem(filter->getText(), 0) );
		
	auto xmlSubNode = xmlNode.child( name );
	if (xmlSubNode) {
		for (auto xmlSubSubNode : xmlSubNode.children("ActorFilter")) {
			filters.emplace_back(xmlSubSubNode);
			
			CEGUI::StandardItem* item = new CEGUI::StandardItem(
				MGE::G11n::getLocaleStringFromXML(xmlSubSubNode, "Text", ""),
				filters.size() - 1
			);
			filter->addItem( item );
		}
		filter->setEnabled(true);
	} else {
		filter->setEnabled(false);
	}
	
	filter->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::ActorsList::handleFilter, this)
	);
	return filter;
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(ActorsList) {
	return new MGE::ActorsList(xmlNode);
}


MGE::ActorsList::~ActorsList(void) {
	LOG_INFO("destroy ActorsList");
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::ActorCreatedEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::ActorDestroyEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::PrimarySelection::SelectionChangeEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::ActionQueue::ActionQueueUpdateEventMsg::MsgType,
		std::bind(&MGE::ActorsList::updateOnEvent, this, std::placeholders::_1),
		this
	);
	
	// window->remClient() is in (automatic called) BaseWindowOwner destructor ... and can destroy baseWin too
}


bool MGE::ActorsList::onShow(const CEGUI::EventArgs& args) {
	if (needUpdate) {
		needUpdate = false;
		doUpdate();
	}
	
	isVisible = true;
	
	return true;
}

bool MGE::ActorsList::onHide(const CEGUI::EventArgs& args) {
	isVisible = false;
	
	return true;
}

bool MGE::ActorsList::handleFilter(const CEGUI::EventArgs& args) {
	doUpdate();
	return true;
}

void MGE::ActorsList::doUpdate() {
	LOG_INFO("ActorsList: updating list of actors");
	float scrollPos;
	
	onUpdate = true;
	
	scrollPos = unitsList->getVertScrollbar()->getScrollPosition();
	unitsList->resetList();
	unitsList->setSortColumn(0);
	const CEGUI::String& brushImage = unitsList->getProperty("DefaultItemSelectionBrushImage");
	
	int aID = 0, bID = 0;
	CEGUI::StandardItem* item;
	item = filterA->getSelectedItem();
	if (item)
		aID = item->getId();
	item = filterB->getSelectedItem();
	if (item)
		bID = item->getId();
	uint64_t mask = defMask | filters[aID].selectionMask | filters[bID].selectionMask;
	uint64_t maskCmpVal = defCmpVal | filters[aID].selectionMaskCompreValue | filters[bID].selectionMaskCompreValue;
	LOG_DEBUG("need objects with " << std::hex << std::showbase << mask << " / " << maskCmpVal << " aID=" << aID << " bID=" << bID);
	
	for (auto& iter : MGE::SelectableObject::allSelectableObject) {
		if ((iter->status & mask) != maskCmpVal)
			continue;
		
		MGE::BaseActor* actor = iter->owner;
		LOG_DEBUG("try object with name = " << actor->getName() << " and selection status mask =" << std::hex << std::showbase << iter->status);
		
		if (! filters[aID].check(actor) || ! filters[bID].check(actor)) {
			continue;
		}
		
		MGE::ActionQueue* actionQueue = actor->getComponent<MGE::ActionQueue>();
		CEGUI::ListboxTextItem* textItem;
		int  rowNum    = unitsList->addRow();
		bool selection = MGE::PrimarySelection::getPtr()->selectedObjects.isSelected(actor);
		int  queueLen  = actionQueue ? actionQueue->getLength() : 0;
		
		// image
		textItem = new CEGUI::ListboxTextItem(
			STRING_TO_CEGUI(("[padding='l:8 t:0 r:8 b:0'][image-size='w:128h:64'][aspect-lock='true'][image='" + actor->getPropertyValue<std::string>("_img", "missing.png") + "']")),
			rowNum
		);
		textItem->setSelectionBrushImage(brushImage);
		textItem->setAutoDeleted(true);
		textItem->setUserData(actor);
		textItem->setCustomTextParser(CEGUI::System::getSingleton().getDefaultTextParser());
		unitsList->setItem( textItem, 0, rowNum);
		unitsList->setItemSelectState( textItem, selection );
		
		// text colour and format
		CEGUI::String text_format("[colour='FF000000']");
		if (queueLen == 0)
			text_format = "[colour='FF00FF00']";
		
		// type name
		textItem = new CEGUI::ListboxTextItem(
			text_format + STRING_TO_CEGUI(actor->getPropertyValue<std::string>("_name", MGE::EMPTY_STRING)),
			rowNum
		);
		textItem->setCustomTextParser(CEGUI::System::getSingleton().getDefaultTextParser());
		textItem->setSelectionBrushImage(brushImage);
		textItem->setAutoDeleted(true);
		unitsList->setItem( textItem, 1, rowNum);
		unitsList->setItemSelectState( textItem, selection );
		
		// action queue length
		textItem = new CEGUI::ListboxTextItem(
			text_format + CEGUI::PropertyHelper<int>::toString(queueLen),
			rowNum
		);
		textItem->setCustomTextParser(CEGUI::System::getSingleton().getDefaultTextParser());
		textItem->setSelectionBrushImage(brushImage);
		textItem->setAutoDeleted(true);
		unitsList->setItem( textItem, 2, rowNum);
		unitsList->setItemSelectState( textItem, selection );
	}
	unitsList->setSortColumn(2);
	unitsList->getVertScrollbar()->setScrollPosition(scrollPos);
	
	onUpdate = false;
}

bool MGE::ActorsList::unitsListSelectionChanged(const CEGUI::EventArgs& args) {
	LOG_INFO("ActorsList: updating selecting of actors");
	
	if (! onUpdate) {
		MGE::PrimarySelection::getPtr()->selectedObjects.unselectAll();
		
		CEGUI::ListboxItem* item;
		item = unitsList->getFirstSelectedItem();
		while(item) {
			MGE::BaseActor* actor = static_cast<MGE::BaseActor*>(item->getUserData());
			if (actor) {
				MGE::PrimarySelection::getPtr()->selectedObjects.select(actor, 0, true);
			}
			item = unitsList->getNextSelected(item);
		}
	} else {
		LOG_INFO("skip - list of actors is on update");
	}
	return true;
}

bool MGE::ActorsList::unitsListDoubleClick(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	if (mbargs.d_generatedClickEventOrder == 2) {
		CEGUI::ListboxItem* item = unitsList->getFirstSelectedItem();
		if (item) {
			MGE::BaseActor* actor = static_cast<MGE::BaseActor*>(item->getUserData());
			if (actor) {
				LOG_INFO("ActorsList: center camera on actor");
				MGE::CameraSystem::getPtr()->getCurrentCamera()->setPosition(
					actor->getComponent<MGE::World3DObject>()->getWorldPosition()
				);
			}
		}
	}
	return true;
}

void MGE::ActorsList::show(const CEGUI::String& name) {
	if (name.empty())
		window->show("Units");
	else
		window->show(name);
}

bool MGE::ActorsList::update(float gameTimeStep, float realTimeStep) {
	if (needUpdate && isVisible) {
		needUpdate = false;
		doUpdate();
		return true;
	}
	return false;
}

void MGE::ActorsList::updateOnEvent(const MGE::EventMsg*) {
	needUpdate = true;
}
