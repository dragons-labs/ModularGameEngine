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

#include "LogSystem.h"

#include <OgreManualObject2.h>

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

class OBBoxRenderable : public MGE::VisualMarker {
public:
	/**
	 * @brief constructor
	 * 
	 * @param boxMaterial     material (colour) of box
	 * @param boxMode         type of box (0x00, 0x10 => full box, 0x01, 0x11 => only corners)
	 * @param linesThickness  thickness of box lines (used only when boxMode is 0x10 or 0x11)
	 * @param aabb            axis aligned bounding box from object (in LOCAL object space)
	 * @param node            scene node to attach box
	 */
	OBBoxRenderable(const Ogre::String& boxMaterial, int boxMode, float linesThickness, const Ogre::AxisAlignedBox& aabb, Ogre::SceneNode* node);
	
	/// destructor
	~OBBoxRenderable();
	
	/**
	 * @brief prepare renderable based on ABB info from object
	 * 
	 * @param aabb  axis aligned bounding box from object (in LOCAL object space)
	 */
	void setupVertices(const Ogre::AxisAlignedBox& aabb) override;
	
	/**
	 * @brief updating type (type) and colour of box
	 * 
	 * @param boxColour       colour of box
	 * @param boxMode         type of box (0x00, 0x10 => full box, 0x01, 0x11 => only corners)
	 * @param linesThickness  thickness of box lines (used only when boxMode is 0x10 or 0x11)
	 */
	void update(int markerType, const Ogre::String& markerMaterial, float linesThickness) override;
	
	/**
	 * @brief return box movable object
	 */ 
	Ogre::MovableObject* getMovable() override {
		return manualObj;
	}
	
protected:
	/// poinbter to box manual object
	Ogre::ManualObject* manualObj;
	
	/// box line thickness
	float thickness;
	
	/// box min and max corners
	Ogre::Vector3 vmin, vmax;
	
	/// box colour
	Ogre::String colorName;
	
	/// function to create box manual object
	void createManualObject(Ogre::SceneManager* scnMgr, bool recreate);
	
private:
	/// helper function for adding corner points to manualObj in FULL_BOX mode with thickness
	static void addCorner(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, const Ogre::Vector3& toCenter, float size);
	
	/// helper function for adding corner points to manualObj in CORNER_BOX mode with thickness
	static void addCorner2(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, const Ogre::Vector3& toCenter, float size, int num, const Ogre::Vector3& delta);
};

OBBoxRenderable::OBBoxRenderable(const Ogre::String& boxMaterial, int boxMode, float linesThickness, const Ogre::AxisAlignedBox& aabb, Ogre::SceneNode* node) : 
	MGE::VisualMarker(boxMode)
{
	colorName = boxMaterial;
	thickness = linesThickness;
	vmax      = aabb.getMaximum();
	vmin      = aabb.getMinimum();
	manualObj = NULL;
	
	createManualObject(node->getCreator(), true);
	node->attachObject(manualObj);
}

OBBoxRenderable::~OBBoxRenderable() {
	manualObj->getParentSceneNode()->detachObject(manualObj);
	manualObj->_getManager()->destroyManualObject(manualObj);
}

void OBBoxRenderable::setupVertices(const Ogre::AxisAlignedBox& aabb) {
	vmax = aabb.getMaximum();
	vmin = aabb.getMinimum();
	createManualObject(manualObj->_getManager(), false);
}

void OBBoxRenderable::update(int markerType, const Ogre::String& markerMaterial, float linesThickness) {
	colorName = markerMaterial;
	if (markerType != type || linesThickness != thickness) {
		type      = markerType;
		thickness = linesThickness;
		createManualObject(manualObj->_getManager(), true);
	} else {
		for (size_t i = 0; i < manualObj->getNumSections(); ++i) {
			static_cast<Ogre::ManualObject::ManualObjectSection*>(manualObj->getSection(i))->setDatablock(colorName);
		}
	}
}

void OBBoxRenderable::addCorner(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, const Ogre::Vector3& toCenter, float size) {
	Ogre::Vector3 scale(toCenter * size);
	
	manualObj->position(point); // main point
	
	manualObj->position(point + Ogre::Vector3(1, 1, 0) * scale); // XY offset point
	manualObj->position(point + Ogre::Vector3(1, 0, 1) * scale); // ZX offset point
	manualObj->position(point + Ogre::Vector3(0, 1, 1) * scale); // ZY offset point
}

void OBBoxRenderable::addCorner2(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, const Ogre::Vector3& toCenter, float size, int num, const Ogre::Vector3& delta) {
	Ogre::Vector3 scale(toCenter * size);
	Ogre::Vector3 pointA, pointB, pointC;
	
	pointA = pointB = pointC = point;
	pointA.x = pointA.x + delta.x * toCenter.x;
	pointB.y = pointB.y + delta.y * toCenter.y;
	pointC.z = pointC.z + delta.z * toCenter.z;
	
	manualObj->position(point); // main point
	manualObj->position(pointA);
	manualObj->position(pointB);
	manualObj->position(pointC);
	
	manualObj->position(point  + Ogre::Vector3(1, 1, 0) * scale); // XY offset point
	manualObj->position(pointA + Ogre::Vector3(0, 1, 0) * scale);
	manualObj->position(pointB + Ogre::Vector3(1, 0, 0) * scale);
	
	manualObj->position(point  + Ogre::Vector3(1, 0, 1) * scale); // ZX offset point
	manualObj->position(pointA + Ogre::Vector3(0, 0, 1) * scale);
	manualObj->position(pointC + Ogre::Vector3(1, 0, 0) * scale);
	
	manualObj->position(point  + Ogre::Vector3(0, 1, 1) * scale); // ZY offset point
	manualObj->position(pointB + Ogre::Vector3(0, 0, 1) * scale);
	manualObj->position(pointC + Ogre::Vector3(0, 1, 0) * scale);
	
	int ii = num * 13;
	// XY
	manualObj->quad(ii+0, ii+4, ii+5, ii+1);
	manualObj->quad(ii+0, ii+2, ii+6, ii+4);
	
	// XZ
	manualObj->quad(ii+0, ii+7, ii+8, ii+1);
	manualObj->quad(ii+0, ii+3, ii+9, ii+7);
	
	// YZ
	manualObj->quad(ii+0, ii+10, ii+11, ii+2);
	manualObj->quad(ii+0, ii+3, ii+12, ii+10);
}

void OBBoxRenderable::createManualObject(Ogre::SceneManager* scnMgr, bool recreate) {
	//LOG_DEBUG("OBBoxRenderable::createManualObject");
	
	if (recreate && manualObj)
		scnMgr->destroyManualObject(manualObj);
	
	if (recreate || manualObj == NULL) {
		manualObj = scnMgr->createManualObject();
		manualObj->begin(colorName, (type < 0x10) ? Ogre::OT_LINE_LIST : Ogre::OT_TRIANGLE_LIST);
	} else {
		manualObj->beginUpdate(0);
	}
	
	float size;
	if ((type & LineThicknessTypeMask) == ABSOLUTE_THICKNESS) {
		size = thickness;
	} else if ((type & LineThicknessTypeMask) == BOX_PROPORTIONAL_THICKNESS) {
		Ogre::Vector3 tmp = vmax - vmin;
		size = thickness * std::fmin(std::fmin(tmp.x, tmp.y), tmp.z);
	}
	
	#if 22 == 11
           .-------B
          /|      /|
         / |     / |
        C-------.  |
        |  A----|--.
        | /     | /
        |/      |/
        .-------D
	#endif
	switch(type & (OOBoxSubTypeMask | LineThicknessTypeMask)) {
		case CORNER_BOX | ABSOLUTE_THICKNESS:
		case CORNER_BOX | BOX_PROPORTIONAL_THICKNESS: {
			Ogre::Vector3 delta((vmax - vmin) / 4.0f);
			
			addCorner2(manualObj, vmin,                                  Ogre::Vector3( 1,  1,  1), size, 0, delta);
			addCorner2(manualObj, Ogre::Vector3(vmin.x, vmax.y, vmin.z), Ogre::Vector3( 1, -1,  1), size, 1, delta);
			addCorner2(manualObj, Ogre::Vector3(vmax.x, vmax.y, vmin.z), Ogre::Vector3(-1, -1,  1), size, 2, delta);
			addCorner2(manualObj, Ogre::Vector3(vmax.x, vmin.y, vmin.z), Ogre::Vector3(-1,  1,  1), size, 3, delta);
			addCorner2(manualObj, vmax,                                  Ogre::Vector3(-1, -1, -1), size, 4, delta);
			addCorner2(manualObj, Ogre::Vector3(vmax.x, vmin.y, vmax.z), Ogre::Vector3(-1,  1, -1), size, 5, delta);
			addCorner2(manualObj, Ogre::Vector3(vmin.x, vmin.y, vmax.z), Ogre::Vector3( 1,  1, -1), size, 6, delta);
			addCorner2(manualObj, Ogre::Vector3(vmin.x, vmax.y, vmax.z), Ogre::Vector3( 1, -1, -1), size, 7, delta);
			break;
		}
		case FULL_BOX | ABSOLUTE_THICKNESS:
		case FULL_BOX | BOX_PROPORTIONAL_THICKNESS: {
			// front XY plane
			addCorner(manualObj, vmin,                                  Ogre::Vector3( 1,  1,  1), size);
			addCorner(manualObj, Ogre::Vector3(vmin.x, vmax.y, vmin.z), Ogre::Vector3( 1, -1,  1), size);
			addCorner(manualObj, Ogre::Vector3(vmax.x, vmax.y, vmin.z), Ogre::Vector3(-1, -1,  1), size);
			addCorner(manualObj, Ogre::Vector3(vmax.x, vmin.y, vmin.z), Ogre::Vector3(-1,  1,  1), size);
			
			manualObj->quad(0+4,  1+4,  1+0,  0+0);
			manualObj->quad(0+8,  1+8,  1+4,  0+4);
			manualObj->quad(0+12, 1+12, 1+8,  0+8);
			manualObj->quad(0+0,  1+0,  1+12, 0+12);
			
			// back XY plane
			addCorner(manualObj, vmax,                                  Ogre::Vector3(-1, -1, -1), size);
			addCorner(manualObj, Ogre::Vector3(vmax.x, vmin.y, vmax.z), Ogre::Vector3(-1,  1, -1), size);
			addCorner(manualObj, Ogre::Vector3(vmin.x, vmin.y, vmax.z), Ogre::Vector3( 1,  1, -1), size);
			addCorner(manualObj, Ogre::Vector3(vmin.x, vmax.y, vmax.z), Ogre::Vector3( 1, -1, -1), size);
			
			manualObj->quad(16+1+4,  16+0+4,  16+0+0,  16+1+0);
			manualObj->quad(16+1+8,  16+0+8,  16+0+4,  16+1+4);
			manualObj->quad(16+1+12, 16+0+12, 16+0+8,  16+1+8);
			manualObj->quad(16+1+0,  16+0+0,  16+0+12, 16+1+12);
			
			// top ZX plane
			manualObj->quad(   2+4,     0+4,  16+0+12, 16+2+12);
			manualObj->quad(16+2+0,  16+0+0,     0+8,     2+8);
			manualObj->quad(   2+8,     0+8,     0+4,     2+4);
			manualObj->quad(16+0+0,  16+2+0,  16+2+12, 16+0+12);
			
			// bottom ZX plane
			manualObj->quad(2+12,       0+12, 16+0+4,  16+2+4);
			manualObj->quad(16+2+8,  16+0+8,     0+0,     2+0);
			manualObj->quad(   2+0,     0+0,     0+12,    2+12);
			manualObj->quad(16+0+8,  16+2+8,  16+2+4,  16+0+4);
			
			// ZY planes (sides)
			manualObj->quad(   0+4,     3+4,  16+3+12, 16+0+12);
			manualObj->quad(16+0+0,  16+3+0,     3+8,     0+8);
			manualObj->quad(   0+12,    3+12, 16+3+4,  16+0+4);
			manualObj->quad(16+0+8,  16+3+8,     3+0,     0+0);
			manualObj->quad(   3+4,     0+4,     0+0,     3+0);
			manualObj->quad(   3+12,    0+12,    0+8,     3+8);
			manualObj->quad(16+0+4,  16+3+4,  16+3+0,  16+0+0);
			manualObj->quad(16+0+12, 16+3+12, 16+3+8,  16+0+8);
			break;
		}
		case CORNER_BOX | NO_THICKNESS:
		{
			Ogre::Real dx = (vmax.x - vmin.x) / 4.0f;
			Ogre::Real dy = (vmax.y - vmin.y) / 4.0f;
			Ogre::Real dz = (vmax.z - vmin.z) / 4.0f;
			
			// A (min.x, min.y, min.z)
			manualObj->position(vmin.x, vmin.y, vmin.z);
			manualObj->position(vmin.x+dx, vmin.y, vmin.z);
			manualObj->position(vmin.x, vmin.y+dy, vmin.z);
			manualObj->position(vmin.x, vmin.y, vmin.z+dz);
			manualObj->line(0, 1);
			manualObj->line(0, 2);
			manualObj->line(0, 3);
			
			// B (max.x, max.y, min.z)
			manualObj->position(vmax.x, vmax.y, vmin.z);
			manualObj->position(vmax.x-dx, vmax.y, vmin.z);
			manualObj->position(vmax.x, vmax.y-dy, vmin.z);
			manualObj->position(vmax.x, vmax.y, vmin.z+dz);
			manualObj->line(4, 5);
			manualObj->line(4, 6);
			manualObj->line(4, 7);
			
			// C (min.x, max.y, max.z)
			manualObj->position(vmin.x, vmax.y, vmax.z);
			manualObj->position(vmin.x+dx, vmax.y, vmax.z);
			manualObj->position(vmin.x, vmax.y-dy, vmax.z);
			manualObj->position(vmin.x, vmax.y, vmax.z-dz);
			manualObj->line(8, 9);
			manualObj->line(8, 10);
			manualObj->line(8, 11);
			
			// D (max.x, min.y, max.z)
			manualObj->position(vmax.x, vmin.y, vmax.z);
			manualObj->position(vmax.x-dx, vmin.y, vmax.z);
			manualObj->position(vmax.x, vmin.y+dy, vmax.z);
			manualObj->position(vmax.x, vmin.y, vmax.z-dz);
			manualObj->line(12, 13);
			manualObj->line(12, 14);
			manualObj->line(12, 15);
			
			// E (min.x, min.y, max.z)
			manualObj->position(vmin.x, vmin.y, vmax.z);
			manualObj->position(vmin.x+dx, vmin.y, vmax.z);
			manualObj->position(vmin.x, vmin.y+dy, vmax.z);
			manualObj->position(vmin.x, vmin.y, vmax.z-dz);
			manualObj->line(16, 17);
			manualObj->line(16, 18);
			manualObj->line(16, 19);
			
			// F (max.x, max.y, max.z)
			manualObj->position(vmax.x, vmax.y, vmax.z);
			manualObj->position(vmax.x-dx, vmax.y, vmax.z);
			manualObj->position(vmax.x, vmax.y-dy, vmax.z);
			manualObj->position(vmax.x, vmax.y, vmax.z-dz);
			manualObj->line(20, 21);
			manualObj->line(20, 22);
			manualObj->line(20, 23);
			
			// G (min.x, max.y, min.z)
			manualObj->position(vmin.x, vmax.y, vmin.z);
			manualObj->position(vmin.x+dx, vmax.y, vmin.z);
			manualObj->position(vmin.x, vmax.y-dy, vmin.z);
			manualObj->position(vmin.x, vmax.y, vmin.z+dz);
			manualObj->line(24, 25);
			manualObj->line(24, 26);
			manualObj->line(24, 27);
			
			// H (max.x, min.y, min.z)
			manualObj->position(vmax.x, vmin.y, vmin.z);
			manualObj->position(vmax.x-dx, vmin.y, vmin.z);
			manualObj->position(vmax.x, vmin.y+dy, vmin.z);
			manualObj->position(vmax.x, vmin.y, vmin.z+dz);
			manualObj->line(28, 29);
			manualObj->line(28, 30);
			manualObj->line(28, 31);
			break;
		}
		case FULL_BOX | NO_THICKNESS:
		default:
		{
			// A (min.x, min.y, min.z)
			manualObj->position(vmin.x, vmin.y, vmin.z);
			manualObj->position(vmax.x, vmin.y, vmin.z);
			manualObj->position(vmin.x, vmax.y, vmin.z);
			manualObj->position(vmin.x, vmin.y, vmax.z);
			manualObj->line(0, 1);
			manualObj->line(0, 2);
			manualObj->line(0, 3);
			
			// B (max.x, max.y, min.z)
			manualObj->position(vmax.x, vmax.y, vmin.z);
			manualObj->position(vmax.x, vmax.y, vmax.z);
			manualObj->position(vmax.x, vmin.y, vmin.z);
			manualObj->position(vmin.x, vmax.y, vmin.z);
			manualObj->line(4, 5);
			manualObj->line(4, 6);
			manualObj->line(4, 7);
			
			// C (min.x, max.y, max.z)
			manualObj->position(vmin.x, vmax.y, vmax.z);
			manualObj->position(vmax.x, vmax.y, vmax.z);
			manualObj->position(vmin.x, vmin.y, vmax.z);
			manualObj->position(vmin.x, vmax.y, vmin.z);
			manualObj->line(8, 9);
			manualObj->line(8, 10);
			manualObj->line(8, 11);
			
			// D (max.x, min.y, max.z)
			manualObj->position(vmax.x, vmin.y, vmax.z);
			manualObj->position(vmax.x, vmin.y, vmin.z);
			manualObj->position(vmax.x, vmax.y, vmax.z);
			manualObj->position(vmin.x, vmin.y, vmax.z);
			manualObj->line(12, 13);
			manualObj->line(12, 14);
			manualObj->line(12, 15);
			break;
		}
	}
	
	manualObj->end();
	manualObj->setRenderQueueGroup(MGE::RenderQueueGroups::UI_3D_V2);
	manualObj->setQueryFlags(0); // set a query flag to exlude from queries (if necessary).
	manualObj->setVisibilityFlags(MGE::VisibilityFlags::UI_3D);
}

/// @}

}
