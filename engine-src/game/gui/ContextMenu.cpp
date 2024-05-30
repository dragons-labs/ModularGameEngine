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

#include "game/gui/ContextMenu.h"

#include "with.h"

#include "ConfigParser.h"
#include "gui/GuiSystem.h"
#include "gui/utils/CeguiString.h"
#include "data/property/G11n.h"
#include "data/utils/OgreUtils.h"

#include "data/structs/BaseActor.h"
#include "data/structs/components/3DWorld.h"
#include "data/structs/components/ObjectOwner.h"
#include "game/actorComponents/SelectableObject.h"
#include "game/misc/PrimarySelection.h"
#include "game/actions/ActionFactory.h"
#include "game/actions/ActionQueue.h"
#include "game/actions/Action.h"
#include "game/actions/ActionPrototype.h"

#include <boost/format.hpp>

/*--------------------- constructors, destructors ---------------------*/

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_ContextMenu \<ContextMenu\>

@c \<ContextMenu\> is used for enabled context menu. This xml node don't have any attributes, but can have subnodes:
	- @c \<TargetSelectionMarker\> with syntax of @ref XMLNode_VisualMarkerSettingsSet
*/

MGE::ContextMenu::ContextMenu(const pugi::xml_node& xmlNode, CEGUI::Window* parent) :
	MGE::Unloadable(200),
	waitForTargetType (0),
	forceWaitForMenuChoice (false),
	actionTargetObjects (NULL)
{
	LOG_INFO("Initialise GUIContextMenu");
	
	if (parent == NULL) {
		parent = MGE::GUISystem::getPtr()->getMainWindow();
	}
	
	menuWin = static_cast<CEGUI::PopupMenu*>( CEGUI::WindowManager::getSingleton().createWindow("PopupMenu") );
	defaultParent = parent;
	parent->addChild(menuWin);
	curentParent = parent;
	
	WITH_NOT_NULL(MGE::Selection::getPtr())->setContextMenu(this);
	
	setSelectionMode(PRIMARY);
	
	// set default values for target selection marker
	targetSelectionMarkerSettings.markerType      = MGE::VisualMarker::OUTLINE;
	targetSelectionMarkerSettings.materialName    = MGE::OgreUtils::getColorDatablock(Ogre::ColourValue(0, 0, 0.916));
	targetSelectionMarkerSettings.linesThickness  = 0.035;
	
	targetSelectionMarkerSettings.loadFromXML(
		xmlNode.child("TargetSelectionMarker")
	);
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(ContextMenu) {
	return new MGE::ContextMenu(xmlNode);
}


MGE::ContextMenu::~ContextMenu(void) {
	LOG_INFO("destroy ContextMenu");
	CEGUI::WindowManager::getSingleton().destroyWindow(menuWin);
	WITH_NOT_NULL(MGE::Selection::getPtr())->setContextMenu(nullptr);
}


/*--------------------- build and show menu, hide menu ---------------------*/

void MGE::ContextMenu::showContextMenu(const Ogre::Vector2& _mousePos, CEGUI::Window* _tgrWin, MGE::RayCast::ResultsPtr _clickSearch) {
	if (forceWaitForMenuChoice)
		return;
	
	LOG_DEBUG("showContextMenu");
	
	bool isNonEmpty = false;
	bool addStopAction = false;
	clickMousePos = _mousePos;
	clickSearch = _clickSearch;
	menuWin->resetList();
	
	// build menu based on selected actors properties
	std::set<MGE::ActionPrototype*> isAddToMenu;
	for (auto& actorIter : MGE::PrimarySelection::getPtr()->selectedObjects.selection) {
		auto propList = actorIter->getPropertyValue< std::list<std::string> >("PosibleActions", {});
		for (auto& iter : propList) {
			MGE::ActionPrototype* actionProto = MGE::ActionFactory::getPtr()->getAction(iter);
			
			if (!actionProto)
				continue;
			
			// check uniqueness of action and target actor compatibility
			if (
				isAddToMenu.find(actionProto) == isAddToMenu.end()
				// && checkTargetActorCompatibility(clickSearch, actionProto, NULL)
				// we can select compatible target later, so now add all actions to menu
			) {
				isAddToMenu.insert(actionProto);
				if (actionProto->subMenuText) {
					CEGUI::PopupMenu* subMenu = static_cast<CEGUI::PopupMenu*>( CEGUI::WindowManager::getSingleton().createWindow("PopupMenu") );
					subMenu->setDestroyedByParent(true);
					addItemToMenu(menuWin, actionProto->menuText + "    ", NULL, 0, NONE)->addChild( subMenu );
					
					for (auto& iter2 : *(actionProto->subMenuText)) {
						addItemToMenu(subMenu, iter2.second, actionProto, iter2.first, ACTION);
					}
				} else {
					addItemToMenu(menuWin, actionProto->menuText, actionProto, 0, ACTION);
				}
				isNonEmpty = true;
			}
		}
		MGE::ActionQueue* actionQueue = actorIter->getComponent<MGE::ActionQueue>();
		if (actionQueue && !actionQueue->isEmpty()) {
			addStopAction = true;
		}
	}
	
	if (addStopAction) {
		addInternalActionToMenu(MGE::G11n::getLocaleString("STOP"), ACTION);
		isNonEmpty = true;
	}
	
	// determinate and set menu parent window
	if (_tgrWin == NULL)
		_tgrWin = defaultParent;
	
	if (_tgrWin != curentParent) {
		curentParent->removeChild(menuWin);
		_tgrWin->addChild(menuWin);
		curentParent = _tgrWin;
	}
	
	if (isNonEmpty)
		fixPositionAndShow();
	else
		menuWin->hide();
}

void MGE::ContextMenu::hideContextMenu() {
	if (forceWaitForMenuChoice)
		return;
	
	clickSearch = NULL; // menu close without creating action, so we don't need clickSearch anymore, so free (if no more instancies) RayCast::Results
	menuWin->resetList();
	menuWin->hide();
}


CEGUI::MenuItem* MGE::ContextMenu::addItemToMenu(CEGUI::PopupMenu* menu, const std::string_view& name, void* ptr, unsigned int id, int callback) {
	LOG_DEBUG("ContextMenu", "add item \"" << name << "\"");
	CEGUI::MenuItem* menuItem = static_cast<CEGUI::MenuItem*>(CEGUI::WindowManager::getSingleton().createWindow( "MenuItem" ));
	
	menuItem->setText(STRING_TO_CEGUI(name));
	switch(callback) {
		case NONE:
			break;
		case ACTION:
			menuItem->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ContextMenu::handleAction, this));
			break;
		case TARGET_DONE:
			menuItem->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ContextMenu::handleTargetDone, this));
			break;
		case SWITCH_SELECTION_MODE:
			menuItem->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::ContextMenu::handleSwitchSelectionMode, this));
			break;
	}
	menuItem->setUserData( ptr );
	menuItem->setID( id );
	menuItem->setDestroyedByParent(true);
	menu->addItem( menuItem );
	
	return menuItem;
}

void MGE::ContextMenu::fixPositionAndShow() {
	// determinate and set menu position
	float w = menuWin->getPixelSize().d_width / menuWin->getParentPixelSize().d_width;
	float h = menuWin->getPixelSize().d_height / menuWin->getParentPixelSize().d_height;
	if (clickMousePos.x + w > 0.999) {
		clickMousePos.x = 0.999 - w;
	}
	if (clickMousePos.y + h > 0.999) {
		clickMousePos.y = 0.999 - h;
	}
	menuWin->setPosition( CEGUI::UVector2(CEGUI::UDim(clickMousePos.x, 0), CEGUI::UDim(clickMousePos.y, 0)) );
	
	// show window
	menuWin->show();
}


/*--------------------- change cursor due to the raycasting for target type ---------------------*/

/*
void MGE::ContextMenu::updateCursor(const Ogre::Vector2& _mousePos) {
	/// @todo TODO.5: change cursor due to the raycasting for target type, use MGE::Target::getClassID and map "id -> cursor name"
};
*/

/*--------------------- check and get action target(s) ---------------------*/

int MGE::ContextMenu::checkTargetActorCompatibility(MGE::RayCast::ResultsPtr _clickSearch, MGE::ActionPrototype* _actionProto, MGE::Action* _action) {
	int retVal = 0;
	
	for (auto& iter : _clickSearch->hitObjects) {
		if ( iter.gameObject && _actionProto->isValidTarget(iter.gameObject) ) { // isValidTarget() check selection mask and do full actor filtering (using propertis, components, etc)
			retVal = 1;
			if(_action)
				_action->targetObjects.insert(dynamic_cast<MGE::BaseActor*>(iter.gameObject));
		}
	}
	
	return retVal;
}

struct MGE::ContextMenu::TargetSelection : public MGE::SelectionSet<MGE::BaseActor*, MGE::QueryFlags::GAME_OBJECT, MGE::ContextMenu::TargetSelection, void> {
	/// @copydoc MGE::SelectionSetTemplate::canSelect
	static bool canSelect(MGE::BaseActor* obj, int mode) {
		MGE::Action* action = MGE::ContextMenu::getPtr()->action;
		return action->getPrototype()->isValidTarget(obj);  // isValidTarget() check selection mask and do full actor filtering (using propertis, components, etc)
	}
	
	/// @copydoc MGE::SelectionSetTemplate::markSelection
	static void markSelection(MGE::BaseActor* obj, bool selection, int mode) {
		MGE::World3DObject* w3d = obj->getComponent<MGE::World3DObject>();
		if (!w3d) {
			LOG_WARNING("can mark selection for no World3DObject actor: " + obj->getName());
			return;
		}
		
		if (selection) {
			LOG_DEBUG("target select: " << obj->getName() << " / " << obj);
			MGE::VisualMarkersManager::getPtr()->showMarker(
				w3d->getOgreSceneNode(), NULL,
				MGE::ContextMenu::getPtr()->targetSelectionMarkerSettings
			);
		} else {
			LOG_DEBUG("target deselect: " << obj->getName() << " / " << obj);
			MGE::VisualMarkersManager::getPtr()->hideMarker(w3d->getOgreSceneNode());
		}
	}
};

void MGE::ContextMenu::setSelectionMode(int type, int selMode) {
	LOG_DEBUG("setSelectionMode type=" << type << " selMode=" << selMode << " waitForTargetType=" << waitForTargetType);
	if (type == PRIMARY) {
		if (selMode < 0)
			selMode = MGE::Selection::GET_OBJECTS;
		WITH_NOT_NULL(MGE::Selection::getPtr())->setSelectionMode( selMode, &(MGE::PrimarySelection::getPtr()->selectedObjects) );
	} else if (type == TARGET) {
		if (selMode < 0) {
			if (waitForTargetType & MGE::ActionPrototype::NEED_AREA) {
				selMode = MGE::Selection::GET_RECTANGLE;
			} else if (waitForTargetType & MGE::ActionPrototype::NEED_POLYGONAL_CHAIN) {
				selMode = MGE::Selection::GET_POLYGONAL_CHAIN;
			} else if (waitForTargetType & MGE::ActionPrototype::NEED_ACTOR) {
				selMode = MGE::Selection::GET_OBJECTS;
			}
		}
		if (selMode >= 0) {
			WITH_NOT_NULL(MGE::Selection::getPtr())->setSelectionMode( selMode, actionTargetObjects, &(action->targetPoints), 0.4 );
		}
	}
}

void MGE::ContextMenu::showTargetWaitMessage() {
	menuWin->resetList();
	
	WITH_NOT_NULL(MGE::Selection::getPtr(), selectionSystem) {
		switch (selectionSystem->getSelectionMode()) {
			case MGE::Selection::GET_OBJECTS:
				addInternalActionToMenu(MGE::G11n::getLocaleString("select target OBJECTS"), TARGET_DONE);
				
				break;
			case MGE::Selection::GET_POLYGONAL_CHAIN:
				if (waitForTargetType & MGE::ActionPrototype::NEED_AREA) {
					addInternalActionToMenu(MGE::G11n::getLocaleString("select target AREA"), TARGET_DONE);
					addInternalActionToMenu(MGE::G11n::getLocaleString("switch to rectangle mode"), SWITCH_SELECTION_MODE, MGE::Selection::GET_RECTANGLE);
				} else {
					addInternalActionToMenu(MGE::G11n::getLocaleString("select target POLYGONAL CHAIN"), TARGET_DONE);
				}
				break;
			case MGE::Selection::GET_RECTANGLE:
				addInternalActionToMenu(MGE::G11n::getLocaleString("select target AREA"), TARGET_DONE);
				addInternalActionToMenu(MGE::G11n::getLocaleString("switch to polygonal mode"), SWITCH_SELECTION_MODE, MGE::Selection::GET_POLYGONAL_CHAIN);
				break;
		}
	}
	
	addInternalActionToMenu(MGE::G11n::getLocaleString("cancel"), TARGET_DONE, 1);
	
	forceWaitForMenuChoice = true;
	fixPositionAndShow();
}

bool MGE::ContextMenu::handleSwitchSelectionMode(const CEGUI::EventArgs& args) {
	LOG_DEBUG("ContextMenu::handleSwitchSelectionMode");
	
	const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	
	action->targetPoints.clear();
	setSelectionMode(TARGET, wargs.window->getID());
	showTargetWaitMessage();
	
	return true;
}


bool MGE::ContextMenu::handleAction(const CEGUI::EventArgs& args) {
	LOG_DEBUG("ContextMenu::handleAction");
	
	const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	MGE::ActionPrototype* actionProto = static_cast<MGE::ActionPrototype*>(wargs.window->getUserData());
	
	if (actionProto == 0) {
		LOG_DEBUG("Run ContextMenu action \"STOP\"");
		
		for(auto& iter : MGE::PrimarySelection::getPtr()->selectedObjects.selection) {
			MGE::ActionQueue* actionQueue = iter->getComponent<MGE::ActionQueue>();
			if (actionQueue)
				actionQueue->clear();
			MGE::ObjectOwner* objectOwner = iter->getComponent<MGE::ObjectOwner>();
			if (objectOwner)
				objectOwner->resetPlanned();
		}
		return true;
	}
	
	LOG_DEBUG("Create ContextMenu action \"" << actionProto->name << "\"");
	action = new MGE::Action( actionProto );
	action->mode = wargs.window->getID();
	waitForTargetType = 0;
	
	// if action need points
	if (actionProto->needMask & MGE::ActionPrototype::NEED_AREA) {
		waitForTargetType |= MGE::ActionPrototype::NEED_AREA;
	} else if (actionProto->needMask & MGE::ActionPrototype::NEED_POLYGONAL_CHAIN) {
		waitForTargetType |= MGE::ActionPrototype::NEED_POLYGONAL_CHAIN;
	} else if (actionProto->needMask & MGE::ActionPrototype::NEED_POINT) {
		action->targetPoints.push_front(clickSearch->groundPoint);
	}
	
	// if action need actors
	if (
		actionProto->needMask & MGE::ActionPrototype::NEED_ACTOR
		&& !checkTargetActorCompatibility(clickSearch, actionProto, action)
	) {
		actionTargetObjects = new MGE::ContextMenu::TargetSelection();
		waitForTargetType |= actionProto->needMask & MGE::ActionPrototype::NEED_ACTOR;
	}
	
	clickSearch = NULL; // action created, so we don't need clickSearch anymore, so free (if no more instancies) RayCast::Results
	
	if (waitForTargetType) {
		setSelectionMode(TARGET);
		showTargetWaitMessage();
	} else {
		addActionToQueue();
	}
	
	return true;
}

bool MGE::ContextMenu::handleTargetDone(const CEGUI::EventArgs& args) {
	LOG_DEBUG("ContextMenu::handleTargetDone");
	
	const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	
	if (wargs.window->getID()) {
		LOG_DEBUG(" ... cancel");
		setSelectionMode(PRIMARY);
		forceWaitForMenuChoice = false;
		return true;
	}
	
	if (waitForTargetType & MGE::ActionPrototype::NEED_ACTOR) {
		LOG_DEBUG("need target actors ...");
		for (auto& iter : actionTargetObjects->selection) {
			// we check action compatibility while selecting (in MGE::ContextMenu::TargetSelection::canSelect),
			// so don't need recheck here --> only add to action and disable visual mark
			action->targetObjects.insert(iter);
			MGE::ContextMenu::TargetSelection::markSelection(iter, false, 0);
		}
		if (! action->targetObjects.empty()) {
			delete actionTargetObjects;
			actionTargetObjects = NULL;
			waitForTargetType &= ~MGE::ActionPrototype::NEED_ACTOR;
		}
	}
	
	if (waitForTargetType & MGE::ActionPrototype::NEED_AREA && action->targetPoints.size() > 2) {
		waitForTargetType &= ~MGE::ActionPrototype::NEED_AREA;
	}
	
	if (waitForTargetType & MGE::ActionPrototype::NEED_POLYGONAL_CHAIN && action->targetPoints.size() > 1) {
		waitForTargetType &= ~MGE::ActionPrototype::NEED_POLYGONAL_CHAIN;
	}
	
	if (waitForTargetType) {
		setSelectionMode(TARGET);
		showTargetWaitMessage();
	} else {
		setSelectionMode(PRIMARY);
		forceWaitForMenuChoice = false;
		addActionToQueue();
	}
	
	return true;
}

/*--------------------- add action to queues ---------------------*/

void MGE::ContextMenu::addActionToQueue() {
	LOG_DEBUG("Action Target is complete ...");
	
	for(auto& iter : MGE::PrimarySelection::getPtr()->selectedObjects.selection) {
		LOG_DEBUG(" - add action for: " << iter->getName());
		
		// check if action is supported by actor
		if ( ! action->getPrototype()->canBeEmitBy(iter) )
			continue;
		
		// queued action (via copy constructor - every actor must have own Action object)
		MGE::ActionQueue* actionQueue = iter->getComponent<MGE::ActionQueue>(MGE::ActionQueue::classID, MGE::ActionQueue::classID);
		actionQueue->addActionAtEnd( new MGE::Action( *action ) );
	}
}
