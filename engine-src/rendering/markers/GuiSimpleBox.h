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

#pragma   once
#include "config.h"

#include <OgreSceneManager.h>
#include <OgreManualObject2.h>
#include <OgreMovableObject.h>
#include <OgreTechnique.h>
#include <OgreRenderQueue.h>

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief Simple 2D (screen coordinates) box graphics (eg for showing selection objects).
 */
class SimpleBox {
private:
	Ogre::SceneNode*           sceneNode;
	Ogre::ManualObject*        manualObj;
	std::string                colorName;
	float                      lineWidthX;
	float                      lineWidthY;
	
public:
	/**
	 * @brief constructor
	 * 
	 * @param[in] color            color of 2D box
	 * @param[in] scnMgr           pointer to scene manager used to create graphics object
	 * @param[in] visibilityFlag   visibility flag
	 * @param[in] linesThickness   thickness of lines (used only when !=0)
	 */
	SimpleBox(const Ogre::ColourValue& color, Ogre::SceneManager* scnMgr, Ogre::uint32 visibilityFlag, float linesThickness);
	
	/// destructor
	virtual ~SimpleBox(void) {
		Ogre::SceneManager* scnMgr = sceneNode->getCreator();
		
		sceneNode->detachObject(manualObj);
		scnMgr->destroyManualObject(manualObj);
		
		scnMgr->getRootSceneNode()->removeChild(sceneNode);
		scnMgr->destroySceneNode(sceneNode);
	}
	
	/**
	 * @brief sets the actual corners of the box and redraw box
	 * 
	 * @param[in] left
	 * @param[in] top
	 * @param[in] right
	 * @param[in] bottom
	 */
	void setCorners(float left, float top, float right, float bottom);
	
	/**
	 * @brief sets the actual corners of the box and redraw box
	 * 
	 * @param[in] topLeft
	 * @param[in] bottomRight
	 */
	inline void setCorners(const Ogre::Vector2& topLeft, const Ogre::Vector2& bottomRight) {
		setCorners(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
	}
};

/// @}

}
