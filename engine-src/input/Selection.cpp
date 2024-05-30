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

Based on:
	→ public domain code from Ogre Wiki (http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Intermediate+Tutorials)
*/

#include "input/Selection.h"

#include "ConfigParser.h"
#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"

#include "input/SelectionContextMenu.h"

#include "rendering/RenderingSystem.h"
#include "physics/Raycast.h"
#include "rendering/markers/GuiSimpleBox.h"
#include "rendering/markers/GuiSimplePolygonalChain.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

MGE::Selection::Selection(
	const Ogre::ColourValue& _selectionBoxColour,   float _selectionBoxLineThickness,
	const Ogre::ColourValue& _selectionChainColour, float _selectionChainLineThickness
) :
	clickHasDown         (false),
	selectionBox         (nullptr),
	polygonalChainMarker (nullptr),
	contextMenu          (nullptr),
	selectionMode        (NONE),
	selectedObjectsPtr   (nullptr),
	selectionBoxColour          (_selectionBoxColour),
	selectionBoxLineThickness   (_selectionBoxLineThickness),
	selectionChainColour        (_selectionChainColour),
	selectionChainLineThickness (_selectionChainLineThickness)
{
	LOG_HEADER("Create Selection system");
	MGE::InputSystem::getPtr()->registerListener(
		this,
		MGE::InputSystem::Listener::SELECTION_INIT, MGE::InputSystem::Listener::SELECTION_CONTINUE, MGE::InputSystem::Listener::SELECTION_CONTINUE,
		-1, -1, -1
	);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_Selection \<Selection\>

@c \<Selection\> is used for configure <b>selection markers</b> (see @ref MGE::UserInterface::Selection::Selection):
	- @c \<selectionBoxColour\> @ref XML_ColourValue with colour of selection box (default green)
	- @c \<selectionBoxLineThickness\> @ref XML_ColourValue with lines thickness of selection box (0 for one pixel default) (default 0)
	- @c \<selectionChainColour\> @ref XML_ColourValue with colour of selection polygonal chain (default blue)
	- @c \<selectionChainLineThickness\> @ref XML_ColourValue with lines thickness of selection polygonal chain (0 for one pixel default) (default 0.35)
*/

MGE::Selection::Selection(const pugi::xml_node& xmlNode) : Selection(
	MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlNode.child("selectionBoxColour"), Ogre::ColourValue(0, 1, 0)),
	xmlNode.child("selectionBoxLineThickness").text().as_float(0),
	MGE::XMLUtils::getValue<Ogre::ColourValue>(xmlNode.child("selectionChainColour"), Ogre::ColourValue(0, 0, 1)),
	xmlNode.child("selectionChainLineThickness").text().as_float(0.35)
) {}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(Selection) {
	return new Selection(xmlNode);
}

MGE::Selection::~Selection() {
	LOG_INFO("Destroy Selection");
	deleteSelectionBox();
	deletePolygonalChainMarker();
}


void MGE::Selection::setSelectionMode( int mode, SelectionSetBase* objects, std::list<Ogre::Vector3>* points, float precision ) {
	selectionMode           = mode;
	selectedObjectsPtr      = objects;
	selectedPointsPtr       = points;
	polygonalChainPrecision = precision;
	
	if (selectionMode != GET_POLYGONAL_CHAIN && selectionMode != GET_RECTANGLE) {
		deletePolygonalChainMarker();
	}
	if (selectionMode == GET_POLYGONAL_CHAIN && !selectedPointsPtr->empty()) {
		reInitPolygonalChainMarker(MGE::CameraSystem::getPtr()->getCurrentCamera());
	}
}

bool MGE::Selection::mouseMoved(const Ogre::Vector2& mouseViewportPos, const OIS::MouseEvent& /*arg*/, MGE::InteractiveTexture* /*_activeTextureObject*/) {
	if (selectionBox) {
		selectionBox->setCorners(selectionStart, mouseViewportPos);
		return true;
	}
	return false;
}

bool MGE::Selection::mousePressed(const Ogre::Vector2& mouseViewportPos, OIS::MouseButtonID clickButtonID, const OIS::MouseEvent& /*arg*/, MGE::InteractiveTexture*& /*_activeTextureObject*/, CEGUI::Window* window) {
	// find the current mouse position
	clickMousePos = mouseViewportPos;
	clickWindow   = window;
	clickHasDown  = true;
	
	// hide context menu when click not hit in context menu
	if (contextMenu)
		contextMenu->hideContextMenu();
	
	if (selectionMode == GET_POLYGONAL_CHAIN) {
		LOG_DEBUG("mousePressed: selectionMode == GET_POLYGONAL_CHAIN");
		if (clickButtonID == OIS::MB_Left) {
			// get and add new point to polygonal chain
			MGE::RayCast::ResultsPtr res = MGE::RayCast::searchFromCamera(clickMousePos.x, clickMousePos.y);
			
			DEBUG2_LOG("click: " << clickMousePos.x << ", " << clickMousePos.y << ", ray start: " <<
				MGE::CameraSystem::getPtr()->getCurrentCamera()->getCameraRay(clickMousePos.x, clickMousePos.y).getOrigin() <<
				" objects count: " << res->hitObjects.size()
			);
			
			if (res->hasGround) {
				if (selectedPointsPtr->empty()) {
					LOG_DEBUG(" init: " << res->groundPoint);
					selectedPointsPtr->push_back(res->groundPoint);
					reInitPolygonalChainMarker(MGE::CameraSystem::getPtr()->getCurrentCamera());
				} else {
					LOG_DEBUG(" add: " << res->groundPoint);
					if (res->groundPoint.squaredDistance(selectedPointsPtr->front()) > polygonalChainPrecision) {
						selectedPointsPtr->push_back(res->groundPoint);
					}  else {
						selectedPointsPtr->push_back(selectedPointsPtr->front());
					}
					polygonalChainMarker->update();
				}
			}
		} else if (clickButtonID == OIS::MB_Right) {
			// remove last point
			if (selectedPointsPtr->size() > 0) {
				selectedPointsPtr->pop_back();
				polygonalChainMarker->update();
			}
		}
	} else if (selectionMode != NONE) {
		LOG_DEBUG("mousePressed: selectionMode == (GET_OBJECTS || GET_RECTANGLE)");
		if (clickButtonID == OIS::MB_Left) {
			// get area or actor, so ... initialize selection box
			if (selectionMode == GET_RECTANGLE) {
				selectedPointsPtr->clear();
			}
			reInitSelectionBox(clickMousePos.x, clickMousePos.y, MGE::CameraSystem::getPtr()->getCurrentCamera());
		}
	}
	return false;
}

#ifdef MGE_DEBUG_SELECTION_VISUAL
#include <OgreMeshManager2.h>
#include "rendering/markers/Shapes.h"

void showHits(MGE::RayCast::ResultsPtr res) {
	Ogre::MeshPtr gridNodeSphereMarkerMesh = Ogre::MeshManager::getSingleton().getByName("PathFinder_SphereMesh");
	if ( gridNodeSphereMarkerMesh.isNull() )
		gridNodeSphereMarkerMesh = MGE::Shapes::createSphereMesh(MGE::LoadingSystem::getPtr()->getGameSceneManager(), "PathFinder_SphereMesh", "General", "MAT_GIZMO_ALL", 0.2, 16, 16);
	int i = 0;
	
	for (auto& iter : res->hitObjects) {
		auto node = MGE::NamedSceneNodes::createSceneNode();
		node->setPosition( iter.hitPoint );
		auto item = node->getCreator()->createItem(gridNodeSphereMarkerMesh);
		
		switch (i++) {
			case 0: item->setDatablock("MAT_GIZMO_X"); break;
			case 1: item->setDatablock("MAT_GIZMO_Y"); break;
			case 2: item->setDatablock("MAT_GIZMO_Z"); break;
			default: item->setDatablock("MAT_GIZMO_ALL"); break;
		}
		
		item->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
		item->setQueryFlags(0);
		node->attachObject(item);
	}
}
#endif

bool MGE::Selection::mouseReleased(const Ogre::Vector2& mouseViewportPos, OIS::MouseButtonID clickButtonID, const OIS::MouseEvent& /*arg*/, MGE::InteractiveTexture* /*_activeTextureObject*/) {
	std::vector<Ogre::Ray>  selectionRays;
	
	if (!clickHasDown) {
		return false;
	} else {
		clickHasDown = false;
	}
	
	if (clickButtonID == OIS::MB_Right && selectionMode != GET_POLYGONAL_CHAIN && contextMenu) {
		// show context menu
		MGE::RayCast::ResultsPtr res = MGE::RayCast::searchFromCamera(clickMousePos.x, clickMousePos.y);
		#ifdef MGE_DEBUG_SELECTION_VISUAL
		showHits(res);
		#endif
		contextMenu->showContextMenu(clickMousePos, clickWindow, res);
	} else if (clickButtonID == OIS::MB_Left && selectionBox) {
		if (selectionMode == GET_RECTANGLE) {
			LOG_DEBUG("mouseReleased: selectionMode == GET_RECTANGLE");
			
			// finish selection of area
			finishSelection(selectionRays, mouseViewportPos, MGE::CameraSystem::getPtr()->getCurrentCamera());
			
			// do raycasting and write results to selectedPointsPtr
			for (auto& iter : selectionRays) {
				MGE::RayCast::ResultsPtr res = MGE::RayCast::searchFromRay(MGE::CameraSystem::getPtr()->getCurrentSceneManager(), iter, 0xFFFFFFFF, true);
				if (! res->hitObjects.empty()) {
					selectedPointsPtr->push_back(res->hitObjects.front().hitPoint);
				}
			}
			selectedPointsPtr->push_back(selectedPointsPtr->front());
		} else if (selectionMode == GET_OBJECTS) {
			LOG_DEBUG("mouseReleased: selectionMode == GET_OBJECTS");
			
			// finish (point / area) selection of actors
			finishSelection(selectionRays, mouseViewportPos, MGE::CameraSystem::getPtr()->getCurrentCamera());
			
			MGE::RayCast::ResultsPtr res;
			int resType;
			if (selectionRays.size() == 1) {
				res = MGE::RayCast::searchFromRay(MGE::CameraSystem::getPtr()->getCurrentSceneManager(), selectionRays[0], selectedObjectsPtr->getSearchMask());
				#ifdef MGE_DEBUG_SELECTION_VISUAL
				showHits(res);
				#endif
				resType = FROM_POINT;
			} else {
				res = MGE::RayCast::searchOnArea(MGE::CameraSystem::getPtr()->getCurrentSceneManager(), selectionRays, selectedObjectsPtr->getSearchMask());
				resType = FROM_AREA;
			}
			
			int selectSwitchMode;
			if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RSHIFT)
				|| MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LSHIFT)
			) {
				selectSwitchMode = SWITCH_SELECTION;
			} else if (MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_RCONTROL)
				|| MGE::InputSystem::getPtr()->isKeyDown(OIS::KC_LCONTROL)
			) {
				selectSwitchMode = ADD_TO_SELECTION;
			} else {
				selectSwitchMode = RESET_SELECTION;
			}
			
			selectedObjectsPtr->select(res, selectSwitchMode, selectionMode | resType);
		}
	}
	
	return true;
}


void MGE::Selection::reInitSelectionBox(float x, float y, MGE::CameraNode* camera) {
	delete selectionBox;
	
	selectionBox = new MGE::SimpleBox(
		selectionBoxColour, camera->getSceneManager(), MGE::VisibilityFlags::SELECTION, selectionBoxLineThickness
	);
	
	selectionStart.x = x;
	selectionStart.y = y;
	
	selectionCamera = camera;
	selectionCamera->addToVisibilityMask(MGE::VisibilityFlags::SELECTION);
}

void MGE::Selection::reInitPolygonalChainMarker(MGE::CameraNode* camera) {
	delete polygonalChainMarker;
	
	polygonalChainMarker = new MGE::SimplePolygonalChain(
		selectionChainColour, camera->getSceneManager(), MGE::VisibilityFlags::SELECTION, selectedPointsPtr, selectionChainLineThickness
	);
	
	camera->addToVisibilityMask(MGE::VisibilityFlags::SELECTION);
}

void MGE::Selection::finishSelection(
	std::vector<Ogre::Ray>& rays, const Ogre::Vector2& selectionStop, MGE::CameraNode* camera
) {
	deleteSelectionBox();
	
	if (selectionStop != selectionStart) {
		float left = selectionStop.x;
		float right = selectionStart.x;
		float top = selectionStop.y;
		float bottom = selectionStart.y;
		float tmp;
	
		if(left > right) {
			tmp = left; left = right; right = tmp;
		}
		if(top > bottom) {
			tmp = top; top = bottom; bottom = tmp;
		}
	
		if((right - left) * (bottom - top) > 0.0001) {
			// finish area mode
			rays.push_back( camera->getCameraRay( left,  top )    );
			rays.push_back( camera->getCameraRay( right, top )    ); 
			rays.push_back( camera->getCameraRay( right, bottom ) );
			rays.push_back( camera->getCameraRay( left,  bottom ) );
			return;
		}
	}
	// finish point mode
	rays.push_back( camera->getCameraRay( selectionStart.x, selectionStart.y ) );
}

void MGE::Selection::deleteSelectionBox() {
	if (selectionBox) {
		selectionCamera->remFromVisibilityMask(MGE::VisibilityFlags::SELECTION);
		delete selectionBox;
	}
	selectionBox = NULL;
}

void MGE::Selection::deletePolygonalChainMarker() {
	delete polygonalChainMarker;
	polygonalChainMarker = NULL;
}
