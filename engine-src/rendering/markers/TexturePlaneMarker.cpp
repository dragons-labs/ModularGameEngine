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

#include "rendering/markers/TexturePlaneMarker.h"

#include "LogSystem.h"
#include "rendering/utils/VisibilityFlags.h"
#include "rendering/utils/RenderQueueGroups.h"

#include <OgreManualObject2.h>

MGE::TexturePlaneMarker::TexturePlaneMarker(const Ogre::String& material, int mode, float factor, const Ogre::AxisAlignedBox& aabb, Ogre::SceneNode* node) : 
	VisualMarker(mode),
	scale(factor)
{
	manualObj = node->getCreator()->createManualObject();
	manualObj->begin(material, Ogre::OT_TRIANGLE_LIST);
	
	manualObj->position(-1, 0,  1);
	manualObj->textureCoord(0, 1);
	manualObj->position( 1, 0,  1);
	manualObj->textureCoord(1, 1);
	manualObj->position( 1, 0, -1);
	manualObj->textureCoord(1, 0);
	manualObj->position(-1, 0, -1);
	manualObj->textureCoord(0, 0);
	
	manualObj->quad(0, 1, 2, 3);
	
	manualObj->end();
	manualObj->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
	manualObj->setQueryFlags(0); // set a query flag to exlude from queries (if necessary).
	manualObj->setVisibilityFlags(MGE::VisibilityFlags::UI_3D);
	
	planeNode = node->createChildSceneNode();
	planeNode->attachObject(manualObj);
	
	setupVertices(aabb);
}

void MGE::TexturePlaneMarker::setupVertices(const Ogre::AxisAlignedBox& aabb) {
	Ogre::Vector3 min = aabb.getMinimum();
	Ogre::Vector3 max = aabb.getMaximum();
	
	Ogre::Vector3 sizeVect(max - min);
	Ogre::Vector3 center(
		min.x + sizeVect.x * 0.5 ,
		min.y + sizeVect.y * 0.05,
		min.z + sizeVect.z * 0.5
	);
	
	size = Ogre::Vector2(sizeVect.x, sizeVect.z).length() / 2.0;
	
	planeNode->setPosition( center );
	planeNode->setScale(Ogre::Vector3(scale * size));
}

void MGE::TexturePlaneMarker::update(int, const Ogre::String& material, float factor) {
	for (size_t i = 0; i < manualObj->getNumSections(); ++i) {
		static_cast<Ogre::ManualObject::ManualObjectSection*>(manualObj->getSection(i))->setDatablock(material);
	}
	scale = factor;
	planeNode->setScale(Ogre::Vector3(scale * size));
}

MGE::TexturePlaneMarker::~TexturePlaneMarker() {
	planeNode->detachObject(manualObj);
	manualObj->_getManager()->destroyManualObject(manualObj);
	planeNode->getParent()->removeChild(planeNode);
	planeNode->getCreator()->destroySceneNode(planeNode);
}

