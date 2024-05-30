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

// #include "utils/enums.h"
#include "data/utils/OgreUtils.h"

#include <OgreSceneManager.h>
#include <OgreManualObject2.h>
#include <OgreTechnique.h>
#include <OgreRenderQueue.h>

namespace MGE {

/// @addtogroup VisualMarkers
/// @{
/// @file

/**
 * @brief Simple polygonal-chain 3D graphics (eg for showing selected area or path).
 */
class SimplePolygonalChain {
private:
	std::string                colorName;
	Ogre::ManualObject*        manualObj;
	
public:
	/**
	 * @brief constructor
	 * 
	 * @param[in]  color            color of graphics object
	 * @param[in]  scnMgr           pointer to scene manager used to create graphics object
	 * @param[in]  visibilityFlag   visibility flag
	 * @param[out] _points          when not null used to store PolygonalChain points
	 * @param[in]  _linesThickness  thickness of lines (used only when !=0)
	 */
	SimplePolygonalChain(
		const Ogre::ColourValue& color, Ogre::SceneManager* scnMgr, int visibilityFlag,
		std::list<Ogre::Vector3>* _points = NULL,
		float _linesThickness = 0
	);
	
	/// destructor
	~SimplePolygonalChain();
	
	/**
	 * @brief redraw polygonal chain whith current lists of points
	 */
	void update();
	
	/**
	 * @brief add point to polygonal chain
	 * 
	 * @param[in] p   - point to add
	 */
	void addPoint(const Ogre::Vector3& p) {
		points->push_back(p + Ogre::Vector3(0, 0.25, 0));
	}
	
	/**
	 * @brief remove last addes point from polygonal chain
	 */
	void deleteLastPointAndUpdate() {
		points->pop_back();
		update();
	}
	
	/**
	 * @brief add point to polygonal chain and redraw it
	 * 
	 * @param[in] p   - point to add
	 */
	void addPointAndUpdate(const Ogre::Vector3& p) {
		addPoint(p);
		update();
	}
	
private:
	std::list<Ogre::Vector3>*  points;
	Ogre::SceneNode*           sceneNode;
	float                      linesThickness;
	bool                       hasOwnPoints;
	
	static Ogre::Vector3 perpendicularVector(const Ogre::Vector3& vect);
	static void addTriangle(Ogre::ManualObject* manualObj, const Ogre::Vector3& point, Ogre::Vector3 dir, float size);
};

/// @}

}
