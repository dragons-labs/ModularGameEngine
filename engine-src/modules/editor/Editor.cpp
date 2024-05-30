/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "modules/editor/Editor.h"

#include "with.h"

#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"

#include "input/SelectionContextMenu.h"

#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"
#include "data/LoadingSystem.h"
#include "data/DotSceneLoader.h"
#include "data/utils/OgreUtils.h"

#include <OgreItem.h>
#include <iomanip>

class MGE::Editor::ContextMenu : public MGE::SelectionContextMenu {
public:
	ContextMenu(MGE::Editor* ed) :
		editor(ed)
	{
		menuWin = static_cast<CEGUI::PopupMenu*>( CEGUI::WindowManager::getSingleton().createWindow("PopupMenu") );
		MGE::GUISystem::getPtr()->getMainWindow()->addChild(menuWin);
		
		addItem("3D marker to camera", 1);
		addItem("3D marker to selection", 2);
		addItem("select 3D marker", 3);
		
		WITH_NOT_NULL(MGE::Selection::getPtr())->setContextMenu(this);
	}
	
	void addItem(const CEGUI::String& text, int id) {
		CEGUI::MenuItem* menuItem = static_cast<CEGUI::MenuItem*>(CEGUI::WindowManager::getSingleton().createWindow( "MenuItem", "item_" + std::to_string(id) ));
		menuItem->setText( text );
		menuItem->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::ContextMenu::callback, this) );
		menuItem->setDestroyedByParent( true );
		menuItem->setUserData( reinterpret_cast<void*>(id) );
		menuWin->addItem( menuItem );
	}
	
	bool callback(const CEGUI::EventArgs& args) {
		const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
		int id = static_cast<int>(reinterpret_cast<size_t>(wargs.window->getUserData()));
		switch (id) {
			case 1:
				editor->marker3DNode->setPosition( MGE::CameraSystem::getPtr()->getCurrentCamera()->getPosition() );
				break;
			case 2:
				editor->marker3DNode->setPosition( editor->targetNode->_getDerivedPosition() );
				break;
			case 3:
				editor->selectionSet.unselectAll();
				editor->selectionSet.select(editor->marker3DNode);
				break;
		}
		return true;
	}
	
	void showContextMenu(const Ogre::Vector2& _mousePos, CEGUI::Window* _tgrWin, MGE::RayCast::ResultsPtr _clickSearch) override {
		// calculate and set position
		Ogre::Vector2 clickMousePos(_mousePos);
		float w = menuWin->getPixelSize().d_width / menuWin->getParentPixelSize().d_width;
		if (clickMousePos.x + w > 0.999) {
			clickMousePos.x = 0.999 - w;
		}
		float h = menuWin->getPixelSize().d_height / menuWin->getParentPixelSize().d_height;
		if (clickMousePos.y + h > 0.999) {
			clickMousePos.y = 0.999 - h;
		}
		menuWin->setPosition( CEGUI::UVector2(CEGUI::UDim(clickMousePos.x, 0), CEGUI::UDim(clickMousePos.y, 0)) );
		
		// show window
		menuWin->show();
	}
	
	void hideContextMenu() override {
		menuWin->hide();
	}
	
protected:
	MGE::Editor* editor;
	CEGUI::PopupMenu* menuWin;
};



/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_EditorModule \<EditorModule\>

@c \<EditorModule\> is used for enabled and configure editor and can have following attributes:
	- @c rotateSpeedFactor   - factor used for rotating speed, default 6
	- @c negScaleFactor      - factor used for scaling speed when object scale < 1, default 2
	- @c posScaleFactor      - factor used for scaling speed when object scale > 1, default 4
	- @c axisGizmoSizeFactor - factor used for size axis gizmo widget, default 25
	- @c defaultGroup        - name of group for searching elements from .scene file in resource system
	.
	and next subnodes:
	- @c \<SelectionMarker\> with syntax of @ref XMLNode_VisualMarkerSettingsSet
	- @c \<Marker3D\> with syntax of @ref XMLNode_Item
	- @c \<ListMeshesFromGroup\> with name of group to list meshes in selection combobox (can be used multiple times for list meshes from different groups)
	see too: @ref MGE::Editor::Editor
*/


MGE::Editor::Editor(const pugi::xml_node& xmlNode, Ogre::SceneManager* scnMgr) :
	MGE::Unloadable(900),
	targetNode(NULL),
	context(scnMgr, false, true, xmlNode.attribute("defaultGroup").as_string("Map_Scene"))
{
	CEGUI::Colour itemColour;
	CEGUI::Window* win;
	
	// create gui window
	winEditor = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("Editor.layout");
	MGE::GUISystem::getPtr()->getMainWindow()->addChild(winEditor);
	
	// get gui sub windows
	win = winEditor->getChild("General");
	winTransformSpace   = static_cast<CEGUI::Combobox*>(win->getChild("TransformSpace"));
	winTransformPivot   = static_cast<CEGUI::Combobox*>(win->getChild("TransformPivot"));
	winValueSpace       = static_cast<CEGUI::Combobox*>(win->getChild("ValueSpace"));
	winOperation        = static_cast<CEGUI::Combobox*>(win->getChild("Operation"));
	winIdividualObjects = static_cast<CEGUI::ToggleButton*>(win->getChild("IdividualObjects"));
	
	winTransform        = winEditor->getChild("Transform");
	win = winTransform->getChild("Position");
	winPositionX        = static_cast<CEGUI::Spinner*>(win->getChild("X"));
	winPositionY        = static_cast<CEGUI::Spinner*>(win->getChild("Y"));
	winPositionZ        = static_cast<CEGUI::Spinner*>(win->getChild("Z"));
	win = winTransform->getChild("Scale");
	winScaleX           = static_cast<CEGUI::Spinner*>(win->getChild("X"));
	winScaleY           = static_cast<CEGUI::Spinner*>(win->getChild("Y"));
	winScaleZ           = static_cast<CEGUI::Spinner*>(win->getChild("Z"));
	win = winTransform->getChild("Rotation");
	winRotationX        = static_cast<CEGUI::Spinner*>(win->getChild("X"));
	winRotationY        = static_cast<CEGUI::Spinner*>(win->getChild("Y"));
	winRotationZ        = static_cast<CEGUI::Spinner*>(win->getChild("Z"));
	winRotationW        = static_cast<CEGUI::Spinner*>(win->getChild("W"));
	winRotationMode     = static_cast<CEGUI::Combobox*>(win->getChild("Mode"));
	
	winNodeInfo         = winEditor->getChild("NodeAndItem");
	winNodeName         = static_cast<CEGUI::Editbox*>(winNodeInfo->getChild("NodeName"));
	winItemName         = static_cast<CEGUI::Editbox*>(winNodeInfo->getChild("ItemName"));
	winMesh             = static_cast<CEGUI::Combobox*>(winNodeInfo->getChild("Mesh"));
	
	winShowTriggers     = static_cast<CEGUI::ToggleButton*>(winEditor->getChild("Misc")->getChild("ShowTriggers"));
	
	
	// set default values for selection marker
	markerSettings.markerType      = MGE::VisualMarker::OBBOX | MGE::VisualMarker::NO_THICKNESS | MGE::VisualMarker::CORNER_BOX;
	markerSettings.materialName    = MGE::OgreUtils::getColorDatablock(Ogre::ColourValue(.916, 0.88, 0.53));
	markerSettings.linesThickness  = 0;
	
	// configure selection marker settings from XML config
	markerSettings.loadFromXML( xmlNode.child("SelectionMarker") );
	
	// load other setting from XML
	rotateSpeedFactor = xmlNode.attribute("rotateSpeedFactor").as_float(6.0);
	negScaleFactor    = xmlNode.attribute("negScaleFactor").as_float(2.0);
	posScaleFactor    = xmlNode.attribute("posScaleFactor").as_float(4.0);
	
	// create gizmo (if need)
	axisGizmo = new MGE::AxisGizmo( context.scnMgr, xmlNode.attribute("axisGizmoSizeFactor").as_float(25.0) );
	
	// create 3D cursor
	marker3DNode = context.scnMgr->getRootSceneNode()->createChildSceneNode();
	auto xmlSubNode = xmlNode.child("Marker3D");
	if (xmlSubNode) {
		MGE::DotSceneLoader::processItem( xmlSubNode, &context, {marker3DNode, NULL} );
	} else {
		marker3DNode->attachObject(context.scnMgr->createItem("Marker3D.mesh"));
	}
	
	// create group "center" node
	groupNode = context.scnMgr->getRootSceneNode()->createChildSceneNode();
	
	// create contex menu
	contextMenu = new ContextMenu(this);
	
	
	// configure selection system
	WITH_NOT_NULL(MGE::Selection::getPtr())->setSelectionMode( MGE::Selection::GET_OBJECTS, &selectionSet );
	
	// set defaults
	selTransformSpace = Ogre::Node::TS_LOCAL;
	selValueSpace     = TRANSFORM_VALUES_LOCAL;
	selTransformPivot = TRANSFORM_POINT_OBJECT;
	selGizmoOperation = MGE::AxisGizmo::MOVE;
	setRotationMode(ROT_QUATERNION);
	setTransformInfo(NULL);
	setNodeInfo(NULL);
	
	// configure AxisGizmo
	gizmoInMoveMode = false;
	axisGizmo->setMode(selGizmoOperation, selTransformSpace, this);
	
	// configure comboboxes
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winTransformSpace->getProperty("DefaultItemTextColour") );
	winTransformSpace->getDropList()->setTextColour(itemColour);
	addItemToCombobox(winTransformSpace, "World",  Ogre::Node::TS_WORLD,  selTransformSpace);
	addItemToCombobox(winTransformSpace, "Parent", Ogre::Node::TS_PARENT, selTransformSpace);
	addItemToCombobox(winTransformSpace, "Local",  Ogre::Node::TS_LOCAL,  selTransformSpace);
	winTransformSpace->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::Editor::handleCombobox, this)
	);
	
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winValueSpace->getProperty("DefaultItemTextColour") );
	winValueSpace->getDropList()->setTextColour(itemColour);
	addItemToCombobox(winValueSpace, "Global", TRANSFORM_VALUES_GLOBAL, selValueSpace);
	addItemToCombobox(winValueSpace, "Local",  TRANSFORM_VALUES_LOCAL,  selValueSpace);
	addItemToCombobox(winValueSpace, "Offset", TRANSFORM_VALUES_OFFSET, selValueSpace);
	winValueSpace->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::Editor::handleCombobox, this)
	);
	
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winTransformPivot->getProperty("DefaultItemTextColour") );
	winTransformPivot->getDropList()->setTextColour(itemColour);
	addItemToCombobox(winTransformPivot, "World",     TRANSFORM_POINT_WORLD,  selTransformPivot);
	addItemToCombobox(winTransformPivot, "Parent",    TRANSFORM_POINT_PARENT, selTransformPivot);
	addItemToCombobox(winTransformPivot, "Object",    TRANSFORM_POINT_OBJECT, selTransformPivot);
	addItemToCombobox(winTransformPivot, "Marker 3D", TRANSFORM_POINT_MARKER, selTransformPivot);
	winTransformPivot->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::Editor::handleCombobox, this)
	);
	
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winOperation->getProperty("DefaultItemTextColour") );
	winOperation->getDropList()->setTextColour(itemColour);
	addItemToCombobox(winOperation, "Move",   MGE::AxisGizmo::MOVE,   selGizmoOperation);
	addItemToCombobox(winOperation, "Rotate", MGE::AxisGizmo::ROTATE, selGizmoOperation);
	addItemToCombobox(winOperation, "Scale",  MGE::AxisGizmo::SCALE,  selGizmoOperation);
	winOperation->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::Editor::handleCombobox, this)
	);
	
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winRotationMode->getProperty("DefaultItemTextColour") );
	winRotationMode->getDropList()->setTextColour(itemColour);
	addItemToCombobox(winRotationMode, "Quaternion", ROT_QUATERNION, rotationMode);
	addItemToCombobox(winRotationMode, "XYZ",        ROT_XYZ,        rotationMode);
	addItemToCombobox(winRotationMode, "XZY",        ROT_XZY,        rotationMode);
	addItemToCombobox(winRotationMode, "YXZ",        ROT_YXZ,        rotationMode);
	addItemToCombobox(winRotationMode, "YZX",        ROT_YZX,        rotationMode);
	addItemToCombobox(winRotationMode, "ZXY",        ROT_ZXY,        rotationMode);
	addItemToCombobox(winRotationMode, "ZYX",        ROT_ZYX,        rotationMode);
	winRotationMode->subscribeEvent(
		CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&MGE::Editor::handleCombobox, this)
	);
	
	itemColour = CEGUI::PropertyHelper<CEGUI::Colour>::fromString( winMesh->getProperty("DefaultItemTextColour") );
	winMesh->getDropList()->setTextColour(itemColour);
	for (auto xmlSubNode2 : xmlNode.children("ListMeshesFromGroup")) {
		std::string groupName = xmlSubNode2.text().as_string();
		
		LOG_DEBUG("add item to mesh selection list from group: " << groupName);
		Ogre::FileInfoListPtr filesInfo = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo( groupName, "*.mesh" );
		for (auto& fi : *filesInfo) {
			LOG_DEBUG(" -- add: " << fi.filename);
			winMesh->addItem( new CEGUI::StandardItem(fi.filename, 0) );
		}
	}
	winMesh->subscribeEvent(
		CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber(&MGE::Editor::handleComboboxKey, this)
	);
	winMesh->subscribeEvent(
		CEGUI::Combobox::EventTextAccepted, CEGUI::Event::Subscriber(&MGE::Editor::nodeApplyCallback, this)
	);
	winItemName->subscribeEvent(
		CEGUI::Combobox::EventTextAccepted, CEGUI::Event::Subscriber(&MGE::Editor::nodeApplyCallback, this)
	);
	winNodeName->subscribeEvent(
		CEGUI::Combobox::EventTextAccepted, CEGUI::Event::Subscriber(&MGE::Editor::nodeApplyCallback, this)
	);
	
	// configure buttons
	winTransform->getChild("Preview")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::valEditCallback, this));
	
	winTransform->getChild("Apply")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::valEditCallback, this));
	
	winNodeInfo->getChild("New")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::newNodeCallback, this));
	
	winNodeInfo->getChild("NewChild")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::newNodeCallback, this));
	
	winNodeInfo->getChild("Apply")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::nodeApplyCallback, this));
	
	winEditor->getChild("Misc")->getChild("SelectParent")->
		subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::Editor::selectParentCallback, this));
	
	winShowTriggers->subscribeEvent(
		CEGUI::ToggleButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&MGE::Editor::handleCheckbox, this)
	);
	
	// show window
	winEditor->show();
	
	// load .scene file
	MGE::LoadingSystem::getPtr()->loadDotSceneFile(
		MGE::LoadingSystem::getPtr()->getLoadingFilePath(),
		&context,
		nullptr,
		&dotSceneFile
	);
	
	// show triggers by default
	MGE::CameraSystem::getPtr()->getCurrentCamera()->addToVisibilityMask(MGE::VisibilityFlags::TRIGGERS | MGE::VisibilityFlags::TARGETS);
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(EditorModule) {
	return new MGE::Editor(xmlNode, context->scnMgr);
}

MGE::Editor::~Editor() {
	LOG_INFO("destroy Editor");
	
	CEGUI::WindowManager::getSingleton().destroyWindow(winEditor);
	
	delete dotSceneFile;
	delete axisGizmo;
	
	MGE::OgreUtils::recursiveDeleteSceneNode(marker3DNode);
	MGE::OgreUtils::recursiveDeleteSceneNode(groupNode);
}

bool MGE::Editor::handleComboboxKey(const CEGUI::EventArgs& args) {
	auto scancode = static_cast<const CEGUI::KeyEventArgs&>(args).d_key;
	CEGUI::StandardItem* item;
	switch( scancode ) {
		case CEGUI::Key::Scan::ArrowUp:
			item = winMesh->getItemFromIndex( winMesh->getItemIndex(winMesh->getSelectedItem()) - 1 );
			if (item)
				winMesh->setText( item->getText() );
			return true;
		case CEGUI::Key::Scan::ArrowDown:
			item = winMesh->getItemFromIndex( winMesh->getItemIndex(winMesh->getSelectedItem()) + 1 );
			if (item)
				winMesh->setText( item->getText() );
			return true;
		case CEGUI::Key::Scan::Return:
		case CEGUI::Key::Scan::NumpadEnter:
			nodeApplyCallback( CEGUI::EventArgs() );
			return true;
		default:
			return false;
	}
}

void MGE::Editor::setNodeInfo(Ogre::SceneNode* node) {
	winNodeName->setText("");
	winItemName->setText("");
	winMesh->setText("");
	
	if(node) {
		pugi::xml_node xmlNode;
		xmlNode = MGE::Any::getFromBindings(node, "xml").getValue<pugi::xml_node>(xmlNode);
		if (!xmlNode) {
			LOG_DEBUG("no xml info for this node ... skipping");
			return;
		}
		
		winNodeName->setText( xmlNode.attribute("name").as_string() );
		
		xmlNode = xmlNode.child("item");
		if (xmlNode) {
			winItemName->setText( xmlNode.attribute("name").as_string() );
			winMesh->setText( xmlNode.attribute("meshFile").as_string() );
		}
	}
}

bool MGE::Editor::nodeApplyCallback(const CEGUI::EventArgs& args) {
	if (!targetNode)
		return true;
	
	pugi::xml_node xmlNode;
	xmlNode = MGE::Any::getFromBindings(targetNode, "xml").getValue<pugi::xml_node>(xmlNode);
	
	MGE::XMLUtils::updateXMLNodeAttrib(xmlNode, "name", STRING_FROM_CEGUI(winNodeName->getText()).data());
	
	pugi::xml_node xmlSubNode = xmlNode.child("item");
	if (!xmlSubNode) {
		LOG_DEBUG("No item in this node ... skip updating");
		return true;
	}
	
	MGE::XMLUtils::updateXMLNodeAttrib(xmlSubNode, "name", STRING_FROM_CEGUI(winItemName->getText()).data());
	if ( MGE::XMLUtils::updateXMLNodeAttrib(xmlSubNode, "meshFile", STRING_FROM_CEGUI(winMesh->getText()).data()) ) {
		auto objIter   = targetNode->getAttachedObjectIterator();
		while(objIter.hasMoreElements()) {
			Ogre::MovableObject* m = objIter.getNext();
			if (m->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
				context.scnMgr->destroyMovableObject(m);
			}
		}
		for (auto xmlSubNode2 : xmlNode.children("item")) {
			MGE::DotSceneLoader::processItem(xmlSubNode2, &context, {targetNode, NULL});
		}
	}
	
	return true;
}

bool MGE::Editor::newNodeCallback(const CEGUI::EventArgs& args) {
	CEGUI::Window* win = static_cast<const CEGUI::WindowEventArgs&>(args).window;
	
	Ogre::SceneNode*  parent;
	pugi::xml_node    xmlParent;
	
	if (win->getName() == "NewChild") {
		parent = targetNode;
		xmlParent = MGE::Any::getFromBindings(targetNode, "xml").getValue<pugi::xml_node>(xmlParent);
	} else {
		parent = context.scnMgr->getRootSceneNode();
		xmlParent = dotSceneFile->child("scene").child("nodes");
	}
	
	// create new node and item
	Ogre::SceneNode* newNode = parent->createChildSceneNode();
	Ogre::Item* newItem = context.scnMgr->createItem( "Cube_1x1x1.mesh" );
	newItem->setDatablockOrMaterialName( "Axis_X" );
	newNode->attachObject( newItem );
	
	// add new node to XML
	auto newXMLNode = xmlParent.append_child("node");
	newXMLNode.append_child("item").append_attribute("meshFile").set_value("Cube_1x1x1.mesh");
	
	newNode->getUserObjectBindings().setUserAny("xml", Ogre::Any(newXMLNode));
	
	// set position
	if (parent != targetNode && targetNode) {
		Ogre::Vector3 position, scale;
		Ogre::Quaternion orientation;
		getTransformInfo(position, scale, orientation, targetNode);
		
		newNode->setPosition(position);
		
		newXMLNode.append_child("position") << position;
	}

	// select new node
	selectionSet.unselectAll();
	selectionSet.select(newNode);
	
	return true;
}


void MGE::Editor::setTransformInfo(Ogre::SceneNode* node, bool updateInitValues) {
	if(!node) {
		if (!winTransform->isDisabled()) {
			winTransform->disable();
			winPositionX->setText("");
			winPositionY->setText("");
			winPositionZ->setText("");
			winRotationX->setText("");
			winRotationY->setText("");
			winRotationZ->setText("");
			winRotationW->setText("");
			winScaleX->setText("");
			winScaleY->setText("");
			winScaleZ->setText("");
		}
		return;
	} else if (winTransform->isDisabled()) {
			winTransform->enable();
			winPositionX->setText("0");
			winPositionY->setText("0");
			winPositionZ->setText("0");
			winRotationX->setText("0");
			winRotationY->setText("0");
			winRotationZ->setText("0");
			if (!winRotationW->isDisabled())
				winRotationW->setText("0");
			winScaleX->setText("0");
			winScaleY->setText("0");
			winScaleZ->setText("0");
	}
	
	if (updateInitValues) {
		// get initial transform (for gizmo mode, TRANSFORM_VALUES_OFFSET mode, ...)
		initScale       = node->getScale();
		initOrientation = node->getOrientation();
		initPosition    = node->getPosition();
		
		if (selectionSet.selection.size() > 1) {
			initOrientations.clear();
			initPositions.clear();
			for (auto& iter : selectionSet.selection) {
				initOrientations[iter] = iter->getOrientation();
				initPositions[iter]    = iter->getPosition();
			}
		}
	}
	
	Ogre::Vector3    position, scale;
	Ogre::Quaternion orientation;
	
	switch(selValueSpace) {
		case TRANSFORM_VALUES_OFFSET:
			position    = Ogre::Vector3::ZERO;
			orientation = Ogre::Quaternion::IDENTITY;
			scale       = Ogre::Vector3::ZERO;
			break;
		case TRANSFORM_VALUES_LOCAL:
			// get relative to PARENT values in PARENT transform space
			position    = node->getPosition();
			orientation = node->getOrientation();
			scale       = node->getScale();
			
			// convert (only rotation of coordinate system) from TS_PARENT (position and orientation) and TS_LOCAL (scale) to selTransformSpace
			switch(selTransformSpace) {
				case Ogre::Node::TS_LOCAL:
					// position is in PARENT axis, so convert it:
					position    = node->getOrientation().Inverse() * position;
					// orientation in LOCAL axis is IDENTITY, so use orientation in PARENT axis
					// scale is in LOCAL axis
					break;
				case Ogre::Node::TS_PARENT:
					// position is in PARENT axis
					// orientation is in PARENT axis
					// scale is in LOCAL axis, so convert it:
					scale       = node->getOrientation() * scale;
					break;
				case Ogre::Node::TS_WORLD: {
					Ogre::Quaternion transform = node->getParent()->_getDerivedOrientation();
					// position is in PARENT axis, so convert it:
					position    = transform * position;
					// orientation is in PARENT axis, so convert it:
					orientation = transform * orientation;
					// scale is in LOCAL axis, so convert it:
					scale       = node->_getDerivedOrientation() * scale;
					break;
				}
			}
			break;
		case TRANSFORM_VALUES_GLOBAL:
			// get relative to WORLD values in WORLD transform space
			MGE::OgreUtils::updateCachedTransform(node, false, false, true);
			position    = node->_getDerivedPosition();
			orientation = node->_getDerivedOrientation();
			scale       = node->_getDerivedScale();
			
			// convert (only rotation of coordinate system) from TS_WORLD (position, orientation) and TS_LOCAL (scale) to selTransformSpace
			switch(selTransformSpace) {
				case Ogre::Node::TS_LOCAL: {
					Ogre::Quaternion transform = node->_getDerivedOrientation().Inverse();
					position    = transform * position;
					orientation = transform * orientation;
					break;
				}
				case Ogre::Node::TS_PARENT: {
					Ogre::Quaternion transform = node->getParent()->_getDerivedOrientation().Inverse();
					position    = transform * position;
					orientation = transform * orientation;
					scale       = node->getOrientation() * scale;
					break;
				}
				case Ogre::Node::TS_WORLD:
					scale       = node->_getDerivedOrientation() * scale;
					break;
			}
	}
	
	winPositionX->setCurrentValue(position.x);
	winPositionY->setCurrentValue(position.y);
	winPositionZ->setCurrentValue(position.z);
	
	if (rotationMode == ROT_QUATERNION) {
		winRotationX->setCurrentValue(orientation.x);
		winRotationY->setCurrentValue(orientation.y);
		winRotationZ->setCurrentValue(orientation.z);
		winRotationW->setCurrentValue(orientation.w);
	} else {
		Ogre::Matrix3 rotation;
		Ogre::Radian x, y, z;
		
		orientation.ToRotationMatrix(rotation);
		switch (rotationMode) {
			case ROT_XYZ: rotation.ToEulerAnglesXYZ(x, y, z); break;
			case ROT_XZY: rotation.ToEulerAnglesXZY(x, z, y); break;
			case ROT_YXZ: rotation.ToEulerAnglesYXZ(y, x, z); break;
			case ROT_YZX: rotation.ToEulerAnglesYZX(y, z, x); break;
			case ROT_ZXY: rotation.ToEulerAnglesZXY(z, x, y); break;
			case ROT_ZYX: rotation.ToEulerAnglesZYX(z, y, x); break;
		}
		
		winRotationX->setCurrentValue(x.valueDegrees());
		winRotationY->setCurrentValue(y.valueDegrees());
		winRotationZ->setCurrentValue(z.valueDegrees());
	}
	
	winScaleX->setCurrentValue(scale.x);
	winScaleY->setCurrentValue(scale.y);
	winScaleZ->setCurrentValue(scale.z);
}

void MGE::Editor::getTransformInfo(Ogre::Vector3& position, Ogre::Vector3& scale, Ogre::Quaternion& orientation, Ogre::SceneNode* node) {
	position.x = winPositionX->getCurrentValue();
	position.y = winPositionY->getCurrentValue();
	position.z = winPositionZ->getCurrentValue();
	
	scale.x = winScaleX->getCurrentValue();
	scale.y = winScaleY->getCurrentValue();
	scale.z = winScaleZ->getCurrentValue();
	
	if (rotationMode == ROT_QUATERNION) {
		orientation.w = winRotationW->getCurrentValue();
		orientation.x = winRotationX->getCurrentValue();
		orientation.y = winRotationY->getCurrentValue();
		orientation.z = winRotationZ->getCurrentValue();
	} else {
		Ogre::Matrix3 rotation;
		Ogre::Degree x(winRotationX->getCurrentValue());
		Ogre::Degree y(winRotationY->getCurrentValue());
		Ogre::Degree z(winRotationZ->getCurrentValue());
		
		LOG_DEBUG("Input Euler Angles mode=" << rotationMode << " x=" << x << " y=" << y << " z=" << z);
		
		switch (rotationMode) {
			case ROT_XYZ: rotation.FromEulerAnglesXYZ(x, y, z); break;
			case ROT_XZY: rotation.FromEulerAnglesXZY(x, z, y); break;
			case ROT_YXZ: rotation.FromEulerAnglesYXZ(y, x, z); break;
			case ROT_YZX: rotation.FromEulerAnglesYZX(y, z, x); break;
			case ROT_ZXY: rotation.FromEulerAnglesZXY(z, x, y); break;
			case ROT_ZYX: rotation.FromEulerAnglesZYX(z, y, x); break;
		}
		
		orientation.FromRotationMatrix(rotation);
	}
	
	LOG_DEBUG("Editor::getTransformInfo input:  position=" << position << " orientation=" << orientation << " scale=" << scale << " in VS=" << selValueSpace << " TS=" << selTransformSpace);
	orientation.normalise();
	
	if (!node)
		return;
	
	switch(selValueSpace) {
		case TRANSFORM_VALUES_OFFSET: {
			// convert (only rotation of coordinate system) from selTransformSpace to TS_PARENT (position and orientation) and TS_LOCAL (scale)
			switch(selTransformSpace) {
				case Ogre::Node::TS_LOCAL:
					position     = initPosition + initOrientation * position;
					orientation  = initOrientation * orientation;
					scale        = initScale + scale;
					break;
				case Ogre::Node::TS_PARENT:
					position     = initPosition + position;
					orientation  = orientation * initOrientation;
					scale        = initScale + initOrientation.Inverse() * scale;
					break;
				case Ogre::Node::TS_WORLD: {
					Ogre::Quaternion transform, transformInv;
					transform    = node->getParent()->_getDerivedOrientation();
					position     = initPosition + transform.Inverse() * position;
					
					transform    = node->_getDerivedOrientation() * initOrientation;
					transformInv = transform.Inverse();
					orientation  = initOrientation * transformInv * orientation * transform;
					scale        = initScale + transformInv * scale;
					break;
				}
			}
			
			if (selTransformPivot != TRANSFORM_POINT_OBJECT && !orientation.orientationEquals(Ogre::Quaternion::IDENTITY)) {
				// get pivotNode position and convert it to our PARENT space
				Ogre::Vector3 pivotPosition = pivotNode->_getDerivedPosition();
				pivotPosition = node->getParent()->_getDerivedOrientation().Inverse() * (pivotPosition - node->getParent()->_getDerivedPosition());
				pivotPosition = pivotPosition / node->getParent()->_getDerivedScale();
				
				// calculate offset from Pivot
				Ogre::Vector3 offset = position - pivotPosition;
				
				// rotate offset
				Ogre::Quaternion rotation = orientation * initOrientation.Inverse();
				offset = rotation * offset;
				
				// update our position
				position = pivotPosition + offset;
			}
			break;
		}
		case TRANSFORM_VALUES_LOCAL:
			// convert (only rotation of coordinate system) from selTransformSpace to TS_PARENT (position and orientation) and TS_LOCAL (scale)
			switch(selTransformSpace) {
				case Ogre::Node::TS_LOCAL:
					position   = node->getOrientation() * position;
					// in TS_LOCAL we use the same orientation as in TS_PARENT, so we don't need convert it
					break;
				case Ogre::Node::TS_PARENT:
					scale       = node->getOrientation().Inverse() * scale;
					break;
				case Ogre::Node::TS_WORLD: {
					Ogre::Quaternion transformInv = node->getParent()->_getDerivedOrientation().Inverse();
					position    = transformInv * position;
					orientation = transformInv * orientation;
					scale       = node->_getDerivedOrientation().Inverse() * scale;
					break;
				}
			}
			break;
		case TRANSFORM_VALUES_GLOBAL: {
			// convert (only rotation of coordinate system) from selTransformSpace to TS_WORLD
			switch(selTransformSpace) {
				case Ogre::Node::TS_LOCAL: {
					Ogre::Quaternion transformFromLocalToWorld = node->_getDerivedOrientation();
					position    = transformFromLocalToWorld * position;
					orientation = transformFromLocalToWorld * orientation;
					scale       = transformFromLocalToWorld * scale;
					break;
				}
				case Ogre::Node::TS_PARENT: {
					Ogre::Quaternion transformRotationFromParentToWorld = node->getParent()->_getDerivedOrientation();
					position    = transformRotationFromParentToWorld * position;
					orientation = transformRotationFromParentToWorld * orientation;
					scale       = transformRotationFromParentToWorld * scale;
					break;
				}
				case Ogre::Node::TS_WORLD: {
					break;
				}
			}
			
			// convert values from WORLD to TS_PARENT (position and orientation) and TS_LOCAL (scale)
			Ogre::Quaternion transformRotationFromWorldToParent = node->getParent()->_getDerivedOrientation().Inverse();
			Ogre::Quaternion transformRotationFromParentToLocal = node->getOrientation().Inverse();
			
			position    = (transformRotationFromWorldToParent * (position - node->getParent()->_getDerivedPosition())) / node->getParent()->_getDerivedScale();
			orientation =  transformRotationFromWorldToParent * orientation;
			scale       = (transformRotationFromWorldToParent * scale) / node->getParent()->_getDerivedScale();
			scale       =  transformRotationFromParentToLocal * scale;
			
			break;
		}
	}
	
	LOG_DEBUG("Editor::getTransformInfo output: position=" << position << " orientation=" << orientation << " scale=" << scale << " in VS=" << selValueSpace << " TS=" << selTransformSpace);
}

void MGE::Editor::gizmoCallback(
	int gizmoMode, Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
	const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint, const OIS::MouseEvent& mouseArg,
	bool  endOfOperation
) {
	Ogre::Vector3 position, scale;
	Ogre::Quaternion orientation;
	int operationsToDo = gizmoMode;
	
	switch(gizmoMode) {
		case MGE::AxisGizmo::MOVE:
			position = Callback::getMove(
				transformSpace, axis, targetNode,
				MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(mouseCurrentPoint.x, mouseCurrentPoint.y),
				gizmoZeroOffset, gizmoInMoveMode
			);
			position = targetNode->getPosition() + position;
			break;
		case MGE::AxisGizmo::SCALE:
			if (transformSpace == Ogre::Node::TS_LOCAL && selTransformPivot == TRANSFORM_POINT_MARKER) {
				// scale axis relative to pivot node
				scale = Callback::getScale(Ogre::Node::TS_PARENT, axis, pivotNode, mouseClickPoint, mouseCurrentPoint, initScale, negScaleFactor, posScaleFactor);
			} else if (selectionSet.selection.size() > 1 && winIdividualObjects->isSelected()) {
				// individual scale axis for each selected node
				for (auto& iter : selectionSet.selection) {
					iter->setScale( MGE::AxisGizmo::Callback::getScale(transformSpace, axis, iter, mouseClickPoint, mouseCurrentPoint, initScale, negScaleFactor, posScaleFactor) );
				}
				targetNode->setScale( MGE::AxisGizmo::Callback::getScale(transformSpace, axis, targetNode, mouseClickPoint, mouseCurrentPoint, initScale, negScaleFactor, posScaleFactor) );
				operationsToDo = 0x00;
			} else {
				// common scale axis for each selected node
				scale = Callback::getScale(transformSpace, axis, targetNode, mouseClickPoint, mouseCurrentPoint, initScale, negScaleFactor, posScaleFactor);
			}
			break;
		case MGE::AxisGizmo::ROTATE:
			Ogre::SceneNode* tmpPivot = NULL;
			
			if (selectionSet.selection.size() > 1) {
				if (winIdividualObjects->isSelected() && selTransformPivot != TRANSFORM_POINT_OBJECT)
					tmpPivot = pivotNode;
				else if (!winIdividualObjects->isSelected() && selTransformPivot == TRANSFORM_POINT_OBJECT)
					tmpPivot = targetNode;
			}
			
			if (tmpPivot) { // rotate individual objects around tmpPivot
				Ogre::Vector3 tmpPivotPosition = tmpPivot->_getDerivedPosition();
				
				for (auto& iter : selectionSet.selection) {
					orientation = Callback::getOrientation(
						transformSpace, axis, iter, mouseClickPoint, mouseCurrentPoint, initOrientations[iter], rotateSpeedFactor
					);
					iter->setPosition( Callback::calculateRotatedPosition(
						iter, tmpPivotPosition, initPositions[iter], initOrientations[iter], orientation
					) );
					iter->setOrientation( orientation );
				}
				
				orientation = Callback::getOrientation(
					transformSpace, axis, targetNode, mouseClickPoint, mouseCurrentPoint, initOrientation, rotateSpeedFactor
				);
				targetNode->setPosition( calculateRotatedPosition(
					targetNode, tmpPivotPosition, initPosition, initOrientation, orientation
				) );
				targetNode->setOrientation( orientation );
				
				gizmoMode |= MGE::AxisGizmo::MOVE;
				operationsToDo = 0x00;
			} else { // common rotate for all nodes (individual objects around themselves OR group center around external pivot point)
				orientation = Callback::getOrientation(
					transformSpace, axis, targetNode, mouseClickPoint, mouseCurrentPoint, initOrientation, rotateSpeedFactor
				);
				
				if (selTransformPivot != TRANSFORM_POINT_OBJECT) {
					position = calculateRotatedPosition(
						targetNode, pivotNode->_getDerivedPosition(), initPosition, initOrientation, orientation
					);
					gizmoMode |= MGE::AxisGizmo::MOVE;
					operationsToDo |= MGE::AxisGizmo::MOVE;
				}
			}
			break;
	}
	
	// update visual position, scale and orientation, update transform info and when endOfOperation update XML too
	if (endOfOperation) {
		updateNodes(position, scale, orientation, operationsToDo, gizmoMode);
		gizmoInMoveMode = false;
	} else {
		updateNodes(position, scale, orientation, operationsToDo, 0x00);
	}
}

bool MGE::Editor::valEditCallback(const CEGUI::EventArgs& args) {
	CEGUI::Window* win = static_cast<const CEGUI::WindowEventArgs&>(args).window;
	
	// get info from editor window
	Ogre::Vector3 position, scale;
	Ogre::Quaternion orientation;
	getTransformInfo(position, scale, orientation, targetNode);
	
	int operationsToDo = MGE::AxisGizmo::ALL;
	
	if (selectionSet.selection.size() > 1 && winIdividualObjects->isSelected()) {
		Ogre::Vector3 oldScale  = targetNode->getScale();
		Ogre::Vector3 diffScale = scale / oldScale;
		
		for (auto& iter : selectionSet.selection) {
			Ogre::Vector3 newScale(Ogre::Vector3::ZERO);
			for (int i=0; i<3; i++) {
				newScale += MGE::AxisGizmo::Callback::getScale(selTransformSpace, MGE::AxisGizmo::AxisArray[i], iter, oldScale, diffScale[i]);
			}
			iter->setScale( newScale );
		}
		
		scale = Ogre::Vector3::ZERO;
		for (int i=0; i<3; i++) {
			scale += MGE::AxisGizmo::Callback::getScale(selTransformSpace, MGE::AxisGizmo::AxisArray[i], targetNode, oldScale, diffScale[i]);
		}
		targetNode->setScale( scale );
		
		operationsToDo &= (~MGE::AxisGizmo::SCALE);
	}
	
	// update visual position, scale and orientation, update transform info and when win->getName() == "Apply" update XML too
	updateNodes(position, scale, orientation, operationsToDo, (win->getName() == "Apply") ? MGE::AxisGizmo::ALL : 0x00);
	
	return true;
}

void MGE::Editor::updateNodes(
	const Ogre::Vector3& position,
	const Ogre::Vector3& scale,
	const Ogre::Quaternion& orientation,
	int operationsToDo,
	int operationsToSave
) {
	LOG_DEBUG("Editor::updateNodes " << position << " " << scale << " " << orientation);
	
	if (selectionSet.selection.size() > 1) {
		Ogre::Vector3    dPosition    = position - targetNode->getPosition();
		Ogre::Quaternion dOrientation = orientation * targetNode->getOrientation().Inverse();
		Ogre::Vector3    dScale       =  scale / targetNode->getScale();
		
		for (auto& iter : selectionSet.selection) {
			if (operationsToDo & MGE::AxisGizmo::MOVE)
				iter->setPosition(dPosition + iter->getPosition());
			
			if (operationsToDo & MGE::AxisGizmo::SCALE) {
				if (! winIdividualObjects->isSelected()) {
					Ogre::Vector3 offset = iter->getPosition() - pivotNode->getPosition();
					iter->setPosition(pivotNode->getPosition() + dScale * offset);
					iter->setScale(dScale * iter->getScale());
				} else {
					iter->setScale(scale);
				}
			}
			
			if (operationsToDo & MGE::AxisGizmo::ROTATE)
				iter->setOrientation(dOrientation * iter->getOrientation());
			
			if (operationsToSave)
				updateXML(iter, iter->getPosition(), iter->getScale(), iter->getOrientation(), operationsToSave);
		}
	}
	
	if (operationsToDo & MGE::AxisGizmo::MOVE)
		targetNode->setPosition(position);
	
	if (operationsToDo & MGE::AxisGizmo::SCALE)
		targetNode->setScale(scale);
	
	if (operationsToDo & MGE::AxisGizmo::ROTATE)
		targetNode->setOrientation(orientation);
	
	if (operationsToSave && selectionSet.selection.size() == 1)
		updateXML(targetNode, position, scale, orientation, operationsToSave);
	
	setTransformInfo(targetNode, operationsToSave);
}

void MGE::Editor::updateXML(
	Ogre::SceneNode* node,
	const Ogre::Vector3& position,
	const Ogre::Vector3& scale,
	const Ogre::Quaternion& orientation,
	int operations
) {
	if (node == marker3DNode)
		return;
	
	// get pointer to xml node
	pugi::xml_node xmlNode; xmlNode = MGE::Any::getFromBindings(node, "xml").getValue<pugi::xml_node>(xmlNode);
	
	if (!xmlNode) {
		LOG_DEBUG("no xml info for this node ... skipping");
		return;
	}
	
	if (operations & MGE::AxisGizmo::MOVE) {
		auto xmlSubNode = xmlNode.child("position");
		if (!xmlSubNode) {
			xmlSubNode = xmlNode.append_child("position");
		} else {
			xmlSubNode.remove_children();
			xmlSubNode.remove_attributes();
		}
		xmlSubNode << position;
	}
	
	if (operations & MGE::AxisGizmo::ROTATE) {
		auto xmlSubNode = xmlNode.child("rotation");
		if (!xmlSubNode) {
			xmlSubNode = xmlNode.append_child("rotation");
		} else {
			xmlSubNode.remove_children();
			xmlSubNode.remove_attributes();
		}
		xmlSubNode << orientation;
	}
	
	if (operations & MGE::AxisGizmo::SCALE) {
		auto xmlSubNode = xmlNode.child("scale");
		if (!xmlSubNode) {
			xmlSubNode = xmlNode.append_child("scale");
		} else {
			xmlSubNode.remove_children();
			xmlSubNode.remove_attributes();
		}
		xmlSubNode << scale;
	}
}


bool MGE::Editor::handleCheckbox(const CEGUI::EventArgs& args) {
	CEGUI::ToggleButton*  win = static_cast<CEGUI::ToggleButton*>(static_cast<const CEGUI::WindowEventArgs&>(args).window);
	bool             selected = win->isSelected();
	if (win == winShowTriggers) {
		LOG_DEBUG("Switch visibility of triggers to: " << selected);
		if (selected) {
			MGE::CameraSystem::getPtr()->getCurrentCamera()->addToVisibilityMask(MGE::VisibilityFlags::TRIGGERS | MGE::VisibilityFlags::TARGETS);
		} else {
			MGE::CameraSystem::getPtr()->getCurrentCamera()->remFromVisibilityMask(MGE::VisibilityFlags::TRIGGERS | MGE::VisibilityFlags::TARGETS);
		}
	}
	return true;
}

bool MGE::Editor::handleCombobox(const CEGUI::EventArgs& args) {
	CEGUI::Combobox*    win  = static_cast<CEGUI::Combobox*>(static_cast<const CEGUI::WindowEventArgs&>(args).window);
	CEGUI::StandardItem* item = win->getSelectedItem();
	
	if (!item) {
		LOG_WARNING("No selected item in " + STRING_FROM_CEGUI(win->getName()));
		return false;
	}
	
	int id = item->getId();
	
	LOG_DEBUG(" select: " << win->getText().c_str() << "  id=" << id);
	
	if (win == winTransformSpace) {
		selTransformSpace = static_cast<Ogre::Node::TransformSpace>(id);
		axisGizmo->setMode(selGizmoOperation, selTransformSpace, this);
		setTransformInfo(targetNode);
	} else if (win == winValueSpace) {
		selValueSpace = id;
		setTransformInfo(targetNode);
	} else if (win == winTransformPivot) {
		setTransformPivot(id);
	} else if (win == winOperation) {
		selGizmoOperation = id;
		axisGizmo->setMode(selGizmoOperation, selTransformSpace, this);
	} else if (win == winRotationMode) {
		setRotationMode(id);
		setTransformInfo(targetNode);
	}
	
	return true;
}

void MGE::Editor::setRotationMode(int id) {
	rotationMode = id;
	
	if (rotationMode == ROT_QUATERNION) {
		winRotationW->enable();
		winRotationW->setText("0");
		winRotationX->setStepSize(0.01);
		winRotationY->setStepSize(0.01);
		winRotationZ->setStepSize(0.01);
		winRotationX->setPrecision(2);
		winRotationY->setPrecision(2);
		winRotationZ->setPrecision(2);
	} else {
		winRotationW->disable();
		winRotationW->setText("");
		winRotationX->setStepSize(1.0);
		winRotationY->setStepSize(1.0);
		winRotationZ->setStepSize(1.0);
		winRotationX->setPrecision(1);
		winRotationY->setPrecision(1);
		winRotationZ->setPrecision(1);
	}
}

void MGE::Editor::setTransformPivot(int id) {
	if (id != NONE)
		selTransformPivot = id;
	
	if (targetNode) {
		switch(selTransformPivot) {
			case TRANSFORM_POINT_WORLD:
				pivotNode = targetNode->getCreator()->getRootSceneNode();
				break;
			case TRANSFORM_POINT_PARENT:
				pivotNode = targetNode->getParentSceneNode();
				break;
			case TRANSFORM_POINT_OBJECT:
				pivotNode = targetNode;
				break;
			case TRANSFORM_POINT_MARKER:
				pivotNode = marker3DNode;
				break;
		}
		axisGizmo->show(pivotNode);
	}
}

void MGE::Editor::addItemToCombobox(CEGUI::Combobox* win, const CEGUI::String& txt, int id, int defId) {
	CEGUI::StandardItem* item = new CEGUI::StandardItem(txt, id);
	win->addItem( item );
	
	if(id == defId)
		win->setText(txt);
}

bool MGE::Editor::selectParentCallback(const CEGUI::EventArgs& args) {
	if (selectionSet.selection.size() == 1) {
		Ogre::SceneNode* parent = static_cast<Ogre::SceneNode*>(targetNode->getParent());
		if (parent && parent->getParent() != NULL) {
			selectionSet.unselectAll();
			selectionSet.select(parent);
		}
	}
	return true;
}

void MGE::Editor::SelectionSet::onSelectionChanged() {
	auto editor = MGE::Editor::getPtr();
	
	LOG_DEBUG("SelectionSet::onSelectionChanged() with set size = " << editor->selectionSet.selection.size());
	
	switch (editor->selectionSet.selection.size()) {
		case 0:
			editor->setTransformInfo(NULL);
			editor->setNodeInfo(NULL);
			editor->axisGizmo->hide();
			break;
		case 1:
			editor->targetNode = *(editor->selectionSet.selection.begin());
			editor->setTransformPivot();
			editor->setTransformInfo(editor->targetNode);
			editor->setNodeInfo(editor->targetNode);
			break;
		default:
			editor->targetNode = editor->groupNode;
			editor->targetNode->setScale(Ogre::Vector3::UNIT_SCALE);
			editor->targetNode->setOrientation(Ogre::Quaternion::IDENTITY);
			Ogre::Vector3 center(Ogre::Vector3::ZERO);
			for (auto& iter : editor->selectionSet.selection) {
				center += iter->getPosition();
			}
			center /= editor->selectionSet.selection.size();
			editor->targetNode->setPosition(center);
			editor->setTransformPivot();
			editor->setTransformInfo(editor->targetNode);
			editor->setNodeInfo(NULL);
			break;
	}
}

void MGE::Editor::SelectionSet::markSelection(Ogre::SceneNode* obj, bool selection, int mode) {
	if (selection && obj != MGE::Editor::getPtr()->marker3DNode) {
		MGE::VisualMarkersManager::getPtr()->showMarker(
			obj, NULL,
			MGE::Editor::getPtr()->markerSettings
		);
	} else if (!selection && obj != MGE::Editor::getPtr()->marker3DNode) {
		MGE::VisualMarkersManager::getPtr()->hideMarker(
			obj
		);
	}
}

bool MGE::Editor::SelectionSet::select( MGE::RayCast::ResultsPtr searchResults, int _selectSwitchMode, int _selectionMode ) {
	LOG_DEBUG("SelectionSet::select() with selectSwitchMode=" << _selectSwitchMode << " searchResults size is: " << searchResults->hitObjects.size() << " selectionMode=" << _selectionMode);
	
	if (searchResults->hitObjects.size() == 0)
		return false;
	
	bool selectTriggers = MGE::Editor::getPtr()->winShowTriggers->isSelected();
	
	// convert searchResults->hitObjects to list of Ogre scene nodes
	std::list<Ogre::SceneNode*> nodes;
	for (auto& iter : searchResults->hitObjects) {
		if (iter.ogreObject) {
			if (!selectTriggers && (iter.ogreObject->getQueryFlags() &  MGE::QueryFlags::TRIGGER)) {
				continue;
			}
			nodes.push_back(iter.ogreObject->getParentSceneNode());
		}
	}
	
	// remove child of parents existed in list
	for (auto iter = nodes.begin(); iter!=nodes.end(); ) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
		if ( std::find(std::begin(nodes), std::end(nodes), (*iter)->getParent()) != std::end(nodes) ) { // if us parent is on list
			nodes.erase(iter++); // remove us from list
		} else {
			++iter;
		}
	}
	
	// select parents of non selectable object (parts of sub-scene)
	for (auto iter = nodes.begin(); iter!=nodes.end(); ) { // don't use `for(auto& it : set)` because of using `set.erase(it)` in the loop
		Ogre::SceneNode* node = (*iter);
		
		// set node to its parent, until node don't have "xml" bindings
		while ( node && ! MGE::Any::getFromBindings(node, "xml").getValue<pugi::xml_node>( pugi::xml_node() ) ) {
			node = static_cast<Ogre::SceneNode*>(node->getParent());
		}
		
		if (node == (*iter)) {
			++iter;
		} else if (node && node->getParent()) { // if we get correct, regular node (not scene root node)
			LOG_DEBUG("selection: replace node without \"xml\" bindings by parrent");
			(*iter) = node;
			++iter;
		} else {
			LOG_DEBUG("selection: remove node without \"xml\" bindings by parrent");
			nodes.erase(iter++);
		}
	}
	
	// remove doubles
	nodes.unique();
	
	LOG_DEBUG("selection: number of selected scene nodes is " << nodes.size());
	
	if (nodes.size() == 0)
		return false;
	
	this->initSelect(_selectSwitchMode, _selectionMode);
	if (_selectionMode & MGE::Selection::SelectionModes::FROM_AREA) {
		for (auto iter : nodes) {
			this->doSelect(iter);       // => call markSelection()
		}
	} else {
		this->doSelect(nodes.front());  // => call markSelection()
	}
	this->finishSelect();               // => call onSelectionChanged()
	return this->selectionChanged;
}
