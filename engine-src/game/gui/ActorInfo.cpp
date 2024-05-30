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

#include "game/gui/ActorInfo.h"

#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"
#include "gui/utils/CeguiStretchedImage.h"
#include "physics/TimeSystem.h"
#include "physics/GameSpeedMessages.h"

#include "gui/modules/MainMenu.h"
#include "modules/gui/WorldMap.h"

#include "game/actions/ActionQueue.h"
#include "game/actions/Action.h"
#include "game/actions/ActionPrototype.h"
#include "game/actorComponents/Health.h"
#include "game/misc/PrimarySelection.h"

/*--------------------- constructors, destructors, load() function ---------------------*/

MGE::ActorInfo::ActorInfo(MGE::GenericWindows::BaseWindow* baseWin) : 
	MGE::GenericWindows::BaseWindowOwner(baseWin),
	MGE::Unloadable(200)
{
	LOG_INFO("Initialise GUI::ActorInfo");
	
	actor = NULL;
	qLen = -1;
	targetObject = NULL;
	toolObject   = NULL;
	showManualSelectedActionInfo = false;
	actorHealthLevel = static_cast<CEGUI::ProgressBar*>(getWindow()->getChild("Actor")->getChild("Health"));
	actorHealthLevel->hide();
	targetHealthLevel = static_cast<CEGUI::ProgressBar*>(getWindow()->getChild("Target")->getChild("Health"));
	targetHealthLevel->hide();
	
	itemList = getWindow()->getChild("Info")->getChild("ItemList");
	itemList->hide();
	itemList->getChild("start")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::firstItemListPage, this)
	);
	itemList->getChild("next")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::nextItemListPage, this)
	);
	for (int i=1; i <= 9; ++i) {
		itemList->getChild(CEGUI::PropertyHelper<int>::toString(i))->subscribeEvent(
			CEGUI::Window::EventClick,
			CEGUI::Event::Subscriber(&MGE::ActorInfo::clickItemList, this)
		);
	}
	
	getWindow()->getChild("Actor")->getChild("QLen")->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::ActorInfo::actionQueueHandle, this)
	);
	
	actionQueueList = static_cast<CEGUI::ListWidget*>(getWindow()->getChild("Info")->getChild("ActionQueue"));
	actionQueueList->subscribeEvent(
		CEGUI::ListWidget::EventSelectionChanged, CEGUI::Event::Subscriber(&MGE::ActorInfo::actionClickHandle, this)
	);
	actionQueueList->setSortMode(CEGUI::ViewSortMode::NoSorting);
	actionQueueList->setMultiSelectEnabled(false);
	actionQueueList->setTextColour( CEGUI::PropertyHelper<CEGUI::ColourRect>::fromString(actionQueueList->getProperty("TextColour")).getColourAtPoint(0,0) );
	actionQueueList->hide();
	
	getWindow()->getChild("MainMenu")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::mainMenuHandle, this)
	);
	getWindow()->getChild("ShowMap")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::showMapHandle, this)
	);
	getWindow()->getChild("PlayPause")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::pauseHandle, this)
	);
	getWindow()->getChild("Speed")->getChild("Up")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::speedIncHandle, this)
	);
	getWindow()->getChild("Speed")->getChild("Down")->subscribeEvent(
		CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ActorInfo::speedDecHandle, this)
	);
	timeInfo = getWindow()->getChild("Time");
	
	MGE::GUISystem::getPtr()->setTranslatedText(getWindow()->getChild("MainMenu"));
	MGE::GUISystem::getPtr()->setTranslatedText(getWindow()->getChild("ShowMap"));
	
	gameSpeedUpdate();
	
	// register listeners
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, POST_RENDER_GUI);
	
	// subscribe for events message
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::GameSpeedChangeEventMsg::MsgType,
		std::bind(&MGE::ActorInfo::gameSpeedUpdate, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::PrimarySelection::SelectionChangeEventMsg::MsgType,
		std::bind(&MGE::ActorInfo::onSelectionUpdate, this, std::placeholders::_1),
		this
	);
	
	needFullUpdate = true;
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_ActorInfo \<ActorInfo\>

@c \<ActorInfo\> is used for enabled and configure GUI widow with information about selected actor.
It have one subnode @c \<ItemsFilter\> with @ref ActorFilter syntax for set standard filter for "items list"
(used when current action don't require special mode - e.g. get tool).
*/
MGE::ActorInfo::ActorInfo(const pugi::xml_node& xmlNode) :
	ActorInfo(new MGE::GenericWindows::MinimizableWindow("ActorWindow.layout"))
{
	pugi::xml_node xmlSubNode = xmlNode.child("ItemsFilter");
	if (xmlSubNode)
		itemStandardFilter.loadFromXML(xmlSubNode);
	
	show();
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(ActorInfo) {
	LOG_INFO("Load / create ActorInfo based on config xml node");
	
	return new MGE::ActorInfo(xmlNode);
}



MGE::ActorInfo::~ActorInfo() {
	LOG_INFO("destroy ActorInfo");
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::GameSpeedChangeEventMsg::MsgType,
		std::bind(&MGE::ActorInfo::gameSpeedUpdate, this, std::placeholders::_1),
		this
	);
	MGE::Engine::getPtr()->getMessagesSystem()->unregisterReceiver(
		MGE::PrimarySelection::SelectionChangeEventMsg::MsgType,
		std::bind(&MGE::ActorInfo::gameSpeedUpdate, this, std::placeholders::_1),
		this
	);
	
	// window->remClient() is in (automatic called) BaseWindowOwner destructor ... and can destroy baseWin too
}


/*--------------------- main window buttons and and speed settings ---------------------*/

bool MGE::ActorInfo::mainMenuHandle(const CEGUI::EventArgs& args) {
	MGE::MainMenu::getPtr()->show();
	return true;
}

bool MGE::ActorInfo::showMapHandle(const CEGUI::EventArgs& args) {
	MGE::WorldMap::getPtr()->show();
	return true;
}

bool MGE::ActorInfo::pauseHandle(const CEGUI::EventArgs& args) {
	MGE::TimeSystem::getPtr()->switchPause();
	return true;
}

bool MGE::ActorInfo::speedIncHandle(const CEGUI::EventArgs& args) {
	float speed = MGE::TimeSystem::getPtr()->getSpeed(false);
	
	LOG_DEBUG("Current speed = " << speed);
	if (speed >= 4.0f) {
		speed = 5.0f;
	} else if (speed >= 1.0f) {
		speed += 0.5f;
	} else if (speed >= 0.6f) {
		speed += 0.2f;
	} else {
		speed += 0.1f;
	}
	LOG_DEBUG("Incrase speed to: " << speed);
	
	MGE::TimeSystem::getPtr()->setSpeed(speed);
	return true;
}

bool MGE::ActorInfo::speedDecHandle(const CEGUI::EventArgs& args) {
	float speed = MGE::TimeSystem::getPtr()->getSpeed(false);
	
	LOG_DEBUG("Current speed = " << speed);
	if (speed <= 0.2f) {
		speed = 0.1f;
	} else if (speed <= 0.6f) {
		speed -= 0.1f;
	} else if (speed <= 1.0f) {
		speed -= 0.2f;
	} else {
		speed -= 0.5f;
	}
	LOG_DEBUG("Deccrase speed to: " << speed);
	
	MGE::TimeSystem::getPtr()->setSpeed(speed);
	return true;
}


/*--------------------- action queue list  ---------------------*/

bool MGE::ActorInfo::actionQueueHandle(const CEGUI::EventArgs& args) {
	if (actionQueueList->isVisible()) {
		showActionTargetAndTool(NULL, false);
		showManualSelectedActionInfo = false;
		actionQueueList->hide();
		showOwnedObjectList();
	} else if (!actorIsTargetActor) {
		showActionsQueueList(false);
		actionQueueList->show();
		itemList->hide();
		showManualSelectedActionInfo = true;
		showActionTargetAndTool(NULL, false);
	}
	return true;
}

void MGE::ActorInfo::showActionsQueueList(bool restoreSelection) {
	MGE::Action* selectedAction = NULL;
	
	if (!actionQueue) {
		actionQueueList->clearList();
		return;
	}
	
	if (restoreSelection) {
		// don't update when no changes in queue
		if (qUpdateTime == actionQueue->getLastUpdateTime())
			return;
		
		// get current selection queued action
		ActionItem* selectedItem = static_cast<ActionItem*>( actionQueueList->getFirstSelectedItem() );
		if (selectedItem) {
			selectedAction = selectedItem->action;
		}
	}
	
	qUpdateTime = actionQueue->getLastUpdateTime();
	
	LOG_DEBUG("update actionQueueList, curr_sel=" << selectedAction);
	
	actionQueueList->clearList();
	for (auto& iter : *actionQueue) {
		LOG_DEBUG("  - " << iter);
		CEGUI::StandardItem* item = new ActionItem(iter->getScriptName(), iter);
		actionQueueList->addItem(item);
		
		if (iter == selectedAction) {
			actionQueueList->setIndexSelectionState(item, true);
		}
	}
}

bool MGE::ActorInfo::actionClickHandle(const CEGUI::EventArgs& args) {
	ActionItem* selectedItem = static_cast<ActionItem*>( actionQueueList->getFirstSelectedItem() );
	if (selectedItem) {
		LOG_DEBUG("  - " << selectedItem << "  " << selectedItem->action);
		showActionTargetAndTool(selectedItem->action, false);
	} else {
		showActionTargetAndTool(NULL, false);
	}
	return true;
}


/*--------------------- owned objects list  ---------------------*/

void MGE::ActorInfo::showOwnedObjectList(bool firstPage) {
	LOG_DEBUG(" - showOwnedObjectList");
	int i = 0;
	CEGUI::Window* w;
	
	itemsUpdateTime = MGE::Engine::getPtr()->getMainLoopTime();
	
	if (firstPage) {
		listedObjectOwner = actor->getComponent<MGE::ObjectOwner>();
		itemFilter = &itemStandardFilter;
		getToolMode = false;
		
		if (targetObject && actionQueue) {
			MGE::Action* action = actionQueue->getFirstAction();
			if (action && (action->getType() & MGE::ActionPrototype::SELECT_TOOL)) {
				listedObjectOwner = targetObject->getComponent<MGE::ObjectOwner>();
				itemFilter = &(action->getPrototype()->targetFilter);
				getToolMode = true;
			}
		}
		
		if (listedObjectOwner)
			itemsIter = listedObjectOwner->begin();
		itemsSubIter = 0;
		itemList->getChild("start")->hide();
	} else {
		itemList->getChild("start")->show();
	}
	
	for (; listedObjectOwner && itemsIter != listedObjectOwner->end(); ++itemsIter) {
		if ( itemFilter->fullCheck(itemsIter->first) ) {
			LOG_DEBUG("adding personel/object to ItemList: " << itemsIter->first->getPropertyValue<std::string>("_name", MGE::EMPTY_STRING));
			
			if (itemsSubIter == 0) { // when start new element of relatedObjects
				if (itemsIter->second.plannedQuantity > itemsIter->second.currentQuantity) {
					itemsSubMax = itemsIter->second.plannedQuantity;
					itemsSubAvailable = itemsIter->second.currentQuantity;
				} else { // this is possible only for prototypes - each actor is a separate element of relatedObjects
					itemsSubMax = itemsIter->second.currentQuantity;
					itemsSubAvailable = itemsIter->second.plannedQuantity;
				}
			}
			LOG_DEBUG("plannedQuantity=" << itemsIter->second.plannedQuantity);
			LOG_DEBUG("currentQuantity=" << itemsIter->second.currentQuantity);
			LOG_DEBUG("itemsSubIter=" << itemsSubIter);
			LOG_DEBUG("itemsSubMax=" << itemsSubMax);
			LOG_DEBUG("itemsSubAvailable=" << itemsSubAvailable);
			
			for (; itemsSubIter < itemsSubMax; ++itemsSubIter) {
				if (i == 9) {
					itemList->getChild("next")->show();
					itemList->show();
					return;
				}
				
				w = itemList->getChild(CEGUI::PropertyHelper<int>::toString(++i));
				
				if (itemsSubIter < itemsSubAvailable) {
					w->setUserData(const_cast<void*>(static_cast<const void*>(itemsIter->first)));
					w->setProperty("Alpha", "1.0");
				} else {
					w->setUserData(NULL);
					w->setProperty("Alpha", "0.5");
				}
				
				MGE::setStretchedImage(
					w,
					itemsIter->first->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING),
					itemsIter->first->getPropertyValue<std::string>("_imgGrp", MGE::EMPTY_STRING)
				);
				
				setHealthLevel(
					static_cast<CEGUI::ProgressBar*>( w->getChild("Health") ), 
					itemsIter->first
				);
			}
			
			if (itemsSubIter == itemsIter->second.plannedQuantity)
				itemsSubIter = 0;
		}
	}
	
	if (i == 0) {
		itemList->hide();
	} else {
		itemList->getChild("next")->hide();
		while (i < 9) {
			w = itemList->getChild(CEGUI::PropertyHelper<int>::toString(++i));
			w->setProperty("Image", "");
		}
		
		itemList->show();
	}
}

bool MGE::ActorInfo::clickItemList(const CEGUI::EventArgs& args) {
	CEGUI::Window* w = static_cast<const CEGUI::WindowEventArgs&>(args).window;
	MGE::BaseActor* a = static_cast<MGE::BaseActor*>(w->getUserData());
	
	if (a) {
		listedObjectOwner->update(a, 0, -1);
		
		MGE::Action* action = new MGE::Action();
		if (getToolMode) {
			action->setType(MGE::ActionPrototype::GET_TOOLS);
			actor->getComponent<MGE::ObjectOwner>(MGE::ObjectOwner::classID, MGE::ObjectOwner::classID)->update(a, 0, 1);
		} else {
			action->setType(MGE::ActionPrototype::EXIT);
		}
		action->toolObjects.insert(a);
		actor->getComponent<MGE::ActionQueue>(MGE::ActionQueue::classID, MGE::ActionQueue::classID)->addActionAtEnd(action);
		
		w->setUserData(NULL);
		w->setProperty("Alpha", "0.5");
	}
	
	return true;
}

bool MGE::ActorInfo::firstItemListPage(const CEGUI::EventArgs& args) {
	showOwnedObjectList();
	return true;
}

bool MGE::ActorInfo::nextItemListPage(const CEGUI::EventArgs& args) {
	showOwnedObjectList(false);
	return true;
}


/*--------------------- action target and tool info ---------------------*/

void MGE::ActorInfo::showActionTargetAndTool(MGE::Action* action, bool force) {
	MGE::BaseActor*   newTargetObject = NULL;
	MGE::NamedObject* newToolObject   = NULL;
	if (action) {
		// get target ... only when action have exacly one target object
		if (action->targetObjects.size() == 1) {
			newTargetObject = *(action->targetObjects.begin());
		}
		
		// get tool ... only when action have exacly one tool
		if (action->toolObjects.size() == 1) {
			newToolObject = *(action->toolObjects.begin());
		}
	}
	
	if (targetObject != newTargetObject || force) {
		targetObject = newTargetObject;
		
		if (targetObject) {
			MGE::setStretchedImage(
				getWindow()->getChild("Target"),
				targetObject->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING),
				targetObject->getPropertyValue<std::string>("_imgGrp", "")
			);
			targetHealthLevel->show();
		} else {
			getWindow()->getChild("Target")->setProperty("Image", "");
			targetHealthLevel->hide();
		}
	}
	
	if (targetObject) {
		setHealthLevel(targetHealthLevel, targetObject);
	}
	
	if (toolObject != newToolObject || force) {
		toolObject = newToolObject;
		
		if (toolObject) {
			MGE::setStretchedImage(
				getWindow()->getChild("Target")->getChild("Tool"),
				toolObject->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING),
				toolObject->getPropertyValue<std::string>("_imgGrp", MGE::EMPTY_STRING)
			);
			getWindow()->getChild("Target")->getChild("Tool")->show();
		} else {
			getWindow()->getChild("Target")->getChild("Tool")->hide();;
		}
	}
}


/*--------------------- on main loop update ---------------------*/

// every frame info update
void MGE::ActorInfo::update(bool force) {
	timeInfo->setText(STRING_TO_CEGUI(
		MGE::TimeSystem::getPtr()->gameTimer->getCounterStr()
	));
	
	if (needFullUpdate) {
		needFullUpdate = false;
		return fullUpdate();
	}
	
	if (actor) {
		// update actor info in "Actor" sub-window
		actorHealthLevel->show();
		setHealthLevel(actorHealthLevel, actor);
		
		int actorQLen = 0;
		if (actionQueue) {
			actorQLen = actionQueue->getLength();
		} else {
			actionQueue = actor->getComponent<MGE::ActionQueue>(); // in case of creation ActionQueue Component on currently selected actor
		}
		
		if (!actorIsTargetActor && qLen != actorQLen) {
			qLen = actorQLen;
			getWindow()->getChild("Actor")->getChild("QLen")->setProperty("Text", 
				CEGUI::PropertyHelper<int>::toString( qLen )
			);
		}
		
		// update actions queue list or owned object list in "Info" sub-window
		if (actionQueueList->isVisible()) {
			showActionsQueueList();
		} else if ((objectOwner && objectOwner->getLastUpdateTime() > itemsUpdateTime) || force) {
			showOwnedObjectList();
		}
		
		if (showManualSelectedActionInfo)
			return;
		
		// update target and tool in "Info" sub-window
		if (!actorIsTargetActor && actorQLen) {
			showActionTargetAndTool( actionQueue->getFirstAction(), force );
		} else {
			showActionTargetAndTool( NULL, force );
		}
	}
}

// switch selected actor update - detect actor change and reinit when needed
void MGE::ActorInfo::fullUpdate() {
	LOG_DEBUG("ActorInfo : fullUpdate");
	
	MGE::BaseActor* oldActor = actor;
	
	if (MGE::PrimarySelection::getPtr()->selectedObjects.selection.size() == 1) {
		LOG_DEBUG(" - selection single");
		actor = *(MGE::PrimarySelection::getPtr()->selectedObjects.selection.begin());
		MGE::SelectableObject* selActor = actor->getComponent<MGE::SelectableObject>();
		if (
			selActor &&
			!(selActor->status & MGE::SelectableObject::IS_SELECTABLE) &&
			  selActor->status & MGE::SelectableObject::IS_ACTION_TARGET
		) {
			LOG_DEBUG("   -> this is target actor");
			actorIsTargetActor = true;
		} else {
			actorIsTargetActor = false;
		}
	} else  {
		LOG_DEBUG(" - selection multiple or empty");
		actor = NULL;
	}
	
	if (actor != oldActor) {
		LOG_DEBUG(" - actor changed");
		if (actor) {
			actionQueue = actor->getComponent<MGE::ActionQueue>();
			objectOwner = actor->getComponent<MGE::ObjectOwner>();
			MGE::setStretchedImage(
				getWindow()->getChild("Actor"),
				actor->getPropertyValue<std::string>("_img", MGE::EMPTY_STRING),
				actor->getPropertyValue<std::string>("_imgGrp", MGE::EMPTY_STRING)
			);
			getWindow()->getChild("Actor")->getChild("Name")->setProperty(
				"Text",
				STRING_TO_CEGUI( actor->getPropertyValue<std::string>("_name", MGE::EMPTY_STRING) )
			);
			if (actorIsTargetActor) {
				getWindow()->getChild("Actor")->getChild("QLen")->setProperty("Text", "");
				qLen = -1;
				targetHealthLevel->hide();
			} else {
				if (actionQueueList->isVisible())
					showActionsQueueList(false);
			}
			update(true);
		} else {
			itemList->hide();
			actorHealthLevel->hide();
			targetHealthLevel->hide();
			actionQueueList->hide();
			getWindow()->getChild("Actor")->setProperty("Image", "");
			getWindow()->getChild("Actor")->getChild("Name")->setProperty("Text", "");
			getWindow()->getChild("Actor")->getChild("QLen")->setProperty("Text", "");
			getWindow()->getChild("Target")->setProperty("Image", "");
			actionQueue  = NULL;
			objectOwner  = NULL;
			toolObject   = NULL;
			targetObject = NULL;
			qLen = -1;
		}
	}
}


/*--------------------- utils ---------------------*/

void MGE::ActorInfo::setHealthLevel(
	CEGUI::ProgressBar* progressBar, MGE::NamedObject* object
) {
	MGE::Health* health = object->getComponent<MGE::Health>();
	if(!health) {
		if (reinterpret_cast<intptr_t>(progressBar->getUserData()) != 9) {
			progressBar->setProgress(1.0);
			progressBar->setProperty("ProgressColour", "ffeeeeee");
			progressBar->setUserData(reinterpret_cast<void*>(9));
		}
	} else if (health->isInjured()) {
		progressBar->setProgress(health->getInjuredHealthLevel());
		if (reinterpret_cast<intptr_t>(progressBar->getUserData()) != 2) {
			progressBar->setProperty("ProgressColour", "ffee0000");
			progressBar->setUserData(reinterpret_cast<void*>(2));
		}
	} else {
		progressBar->setProgress(health->getNormalHealthLevel());
		if (reinterpret_cast<intptr_t>(progressBar->getUserData()) != 3) {
			progressBar->setProperty("ProgressColour", "ff00ee00");
			progressBar->setUserData(reinterpret_cast<void*>(3));
		}
	}
}

void MGE::ActorInfo::onSelectionUpdate(const MGE::EventMsg*)  {
	LOG_DEBUG("ActorInfo::onSelectionUpdate");
	needFullUpdate = true;
}

void MGE::ActorInfo::gameSpeedUpdate(const MGE::EventMsg* eventMsg) {
	// auto speedMsg = static_cast<const MGE::GameSpeedChangeEventMsg*>(eventMsg);
	
	if (MGE::TimeSystem::getPtr()->gameIsPaused())
		MGE::GUISystem::getPtr()->setTranslatedText( getWindow()->getChild("PlayPause"), "Unpause", "txt:onPaused:" );
	else
		MGE::GUISystem::getPtr()->setTranslatedText( getWindow()->getChild("PlayPause"), "Pause", "txt:onUnpaused:" );
	
	std::stringstream toStr;
	toStr << MGE::TimeSystem::getPtr()->getSpeed(false);
	getWindow()->getChild("Speed")->getChild("Value")->setText(toStr.str());
}

void MGE::ActorInfo::show(const CEGUI::String& /*name*/) {
	window->show();
}

void MGE::ActorInfo::toggleVisibility() {
	window->getWindow()->setVisible( !window->getWindow()->isVisible() );
}

bool MGE::ActorInfo::update(float gameTimeStep, float realTimeStep) {
	update(false);
	return true;
}

MGE::ActorInfo::ActionItem::ActionItem(const CEGUI::String& text, MGE::Action* a) :
	CEGUI::StandardItem( text, 0 ),
	action(a)
{ }

bool MGE::ActorInfo::ActionItem::operator==(const CEGUI::GenericItem& other) const {
	const ActionItem* myOther = dynamic_cast<const ActionItem*>(&other);
	if (myOther && action != myOther->action)
		return false;
	return CEGUI::GenericItem::operator==(other);
}
