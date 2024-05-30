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

#include "rendering/markers/GuiSimpleBox.h"

#include "rendering/utils/RenderQueueGroups.h"
#include "data/utils/OgreUtils.h"

#include <OgreRoot.h>
#include <OgreWindow.h>

MGE::SimpleBox::SimpleBox(const Ogre::ColourValue& color, Ogre::SceneManager* scnMgr, Ogre::uint32 visibilityFlag, float linesThickness) :
	colorName( MGE::OgreUtils::getColorDatablock(color) ),
	lineWidthX( 0 ), lineWidthY( 0 )
{
	manualObj = scnMgr->createManualObject();
	
	if (linesThickness == 0) {
		manualObj->begin(colorName, Ogre::OT_LINE_STRIP);
			manualObj->position(-1,       -1,       0);
			manualObj->position(-0.9999F, -1,       0);
			manualObj->position(-0.9999F, -0.9999F, 0);
			manualObj->position(-1,       -0.9999F, 0);
			manualObj->index(0);
			manualObj->index(1);
			manualObj->index(2);
			manualObj->index(3);
			manualObj->index(0);
		manualObj->end();
	} else {
		auto win = Ogre::Root::getSingletonPtr()->getAutoCreatedWindow();
		lineWidthX = linesThickness / win->getWidth();
		lineWidthY = linesThickness / win->getHeight();
		manualObj->begin(colorName, Ogre::OT_TRIANGLE_LIST);
			// left top
			manualObj->position(-1+.0002F, -1+.0002F,       0);
			manualObj->position(-1+.0003F, -1+.0003F,       0);
			// left bottom
			manualObj->position(-1+.0002F, -1+.0008F,       0);
			manualObj->position(-1+.0003F, -1+.0007F,       0);
			// right bottom
			manualObj->position(-1+.0008F, -1+.0008F,       0);
			manualObj->position(-1+.0007F, -1+.0007F,       0);
			// right top
			manualObj->position(-1+.0008F, -1+.0002F,       0);
			manualObj->position(-1+.0007F, -1+.0003F,       0);
			
			// left
			manualObj->triangle(0, 1, 2);
			manualObj->triangle(1, 2, 3);
			// bottom
			manualObj->triangle(2, 4, 3);
			manualObj->triangle(3, 5, 4);
			// right
			manualObj->triangle(4, 5, 6);
			manualObj->triangle(5, 7, 6);
			// top
			manualObj->triangle(6, 0, 7);
			manualObj->triangle(7, 0, 1);
		manualObj->end();
	}
	
	sceneNode = scnMgr->getRootSceneNode()->createChildSceneNode();
	sceneNode->attachObject(manualObj);
	
	for (unsigned int i = 0; i < manualObj->getNumSections(); ++i) {
		Ogre::ManualObject::ManualObjectSection* s = static_cast<Ogre::ManualObject::ManualObjectSection*>(manualObj->getSection(i));
		s->setUseIdentityProjection(true);
		//s->setUseIdentityView(true);
	}
	
	manualObj->setLocalAabb(Ogre::Aabb::BOX_INFINITE);
	manualObj->setRenderQueueGroup(MGE::RenderQueueGroups::OVERLAY_V2);
	manualObj->setVisibilityFlags(visibilityFlag);
	manualObj->setVisible(false);
	manualObj->setQueryFlags(0);
}

void MGE::SimpleBox::setCorners(float left, float top, float right, float bottom) {
	left = left * 2 - 1;
	right = right * 2 - 1;
	top = 1 - top * 2;
	bottom = 1 - bottom * 2;
	
	if (lineWidthX == 0) {
		manualObj->beginUpdate(0);
			manualObj->position(left, top, 0);
			manualObj->position(right, top, 0);
			manualObj->position(right, bottom, 0);
			manualObj->position(left, bottom, 0);
			manualObj->index(0);
			manualObj->index(1);
			manualObj->index(2);
			manualObj->index(3);
			manualObj->index(0);
		manualObj->end();
	} else {
		manualObj->beginUpdate(0);
			// left top
			manualObj->position(left, top,       0);
			manualObj->position(left + lineWidthX, top - lineWidthY,       0);
			// left bottom
			manualObj->position(left, bottom, 0);
			manualObj->position(left + lineWidthX, bottom + lineWidthY, 0);
			// right bottom
			manualObj->position(right, bottom, 0);
			manualObj->position(right - lineWidthX, bottom + lineWidthY, 0);
			// right top
			manualObj->position(right, top, 0);
			manualObj->position(right - lineWidthX, top - lineWidthY, 0);
			
			// left
			manualObj->triangle(0, 2, 1);
			manualObj->triangle(1, 2, 3);
			// bottom
			manualObj->triangle(2, 4, 3);
			manualObj->triangle(3, 5, 4);
			// right
			manualObj->triangle(4, 5, 6);
			manualObj->triangle(5, 7, 6);
			// top
			manualObj->triangle(6, 0, 7);
			manualObj->triangle(7, 0, 1);
		manualObj->end();
	}
	
	manualObj->setVisible(true);
}
