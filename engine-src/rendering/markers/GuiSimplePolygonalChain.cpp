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

Inspired by:
	→ OGRE (MIT licensed)
*/

#include "rendering/markers/GuiSimplePolygonalChain.h"

#include "LogSystem.h"
#include "rendering/utils/RenderQueueGroups.h"

Ogre::Vector3 MGE::SimplePolygonalChain::perpendicularVector(const Ogre::Vector3& vect) {
	// based on Ogre::Vector3::perpendicular, but first try Y
	static const Ogre::Real fSquareZero = (1e-06 * 1e-06);
	
	Ogre::Vector3 perp = vect.crossProduct( Ogre::Vector3::UNIT_Y );
	
	if( perp.squaredLength() < fSquareZero ) {
		perp = vect.crossProduct( Ogre::Vector3::UNIT_X );
	}
	perp.normalise();
	
	return perp;
}

void MGE::SimplePolygonalChain::addTriangle(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, Ogre::Vector3 dir, float size) {
	dir.normalise(); // when dir is normalise, results of Quaternion(angle, dir) * vector save vector oryginal length
	
	Ogre::Vector3 offset = perpendicularVector(dir) * size;
	
	offset = Ogre::Quaternion(Ogre::Degree(90), dir) * offset;
	manualObj->position(point + offset);
	
	offset = Ogre::Quaternion(Ogre::Degree(120), dir) * offset;
	manualObj->position(point + offset);
	
	offset = Ogre::Quaternion(Ogre::Degree(120), dir) * offset;
	manualObj->position(point + offset);
}

MGE::SimplePolygonalChain::SimplePolygonalChain(
	const Ogre::ColourValue& color, Ogre::SceneManager* scnMgr, int visibilityFlag,
	std::list<Ogre::Vector3>* _points, float _linesThickness
) {
	if (_points) {
		points = _points;
		hasOwnPoints = false;
	} else {
		points = new std::list<Ogre::Vector3>();
		hasOwnPoints = true;
	}
	linesThickness = _linesThickness;
	
	colorName = MGE::OgreUtils::getColorDatablock(color);
	
	manualObj = scnMgr->createManualObject();
	
	sceneNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	sceneNode->attachObject(manualObj);
	
	manualObj->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
	manualObj->setVisibilityFlags(visibilityFlag);
	manualObj->setQueryFlags(0);
}

MGE::SimplePolygonalChain::~SimplePolygonalChain() {
	Ogre::SceneManager* scnMgr = sceneNode->getCreator();
	
	sceneNode->detachObject(manualObj);
	scnMgr->destroyManualObject(manualObj);
	
	scnMgr->getRootSceneNode()->removeChild(sceneNode);
	scnMgr->destroySceneNode(sceneNode);
	
	if (hasOwnPoints)
		delete points;
}

void MGE::SimplePolygonalChain::update() {
	if (linesThickness < 1e-06) {
		int i = 0;
		
		manualObj->clear();
		manualObj->begin(colorName, Ogre::OT_LINE_STRIP);
		for (auto& iter : *points) {
			manualObj->position(iter);
			manualObj->index(i++);
		}
		manualObj->end();
		
		manualObj->setVisible(true);
	} else {
		std::list<Ogre::Vector3>::iterator  prev, curr, next;
		
		// clear previous chain visualisation
		manualObj->clear();
		
		// we need at least 2 points to draw line
		if (points->size() < 2) {
			manualObj->setVisible(false);
			return;
		}
		
		// start manual object
		manualObj->begin(colorName, Ogre::OT_TRIANGLE_LIST);
		
		// add points
		prev = points->end();
		curr = next = points->begin();
		while (true) {
			// from prev
			if (prev != points->end()) {
				addTriangle(manualObj, *curr, *curr -* prev, linesThickness);
			}
			
			// to next
			if (++next != points->end()) {
				addTriangle(manualObj, *curr, *next -* curr, linesThickness);
			} else {
				break;
			}
			
			prev = curr;
			curr = next;
		}
		
		// add indexes
		int numOfSections = points->size() - 1, ii = 0;
		manualObj->triangle(ii+2, ii+1, ii+0);
		for (int i=0; i<numOfSections; ++i) {
			if (i>0) {
				ii = (i-1)*6 + 3;
				manualObj->quad(ii+0, ii+1, ii+4, ii+3);
				manualObj->quad(ii+1, ii+2, ii+5, ii+4);
				manualObj->quad(ii+2, ii+0, ii+3, ii+5);
			}
			ii = i*6;
			manualObj->quad(ii+0, ii+1, ii+4, ii+3);
			manualObj->quad(ii+1, ii+2, ii+5, ii+4);
			manualObj->quad(ii+2, ii+0, ii+3, ii+5);
		}
		manualObj->triangle(ii+3, ii+4, ii+5);
		
		// finish manual object
		manualObj->end();
		manualObj->setVisible(true);
	}
}
