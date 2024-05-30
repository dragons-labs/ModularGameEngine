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

#pragma   once

#include "gui/Gui3D.h"

#include <CEGUI/CEGUI.h>

namespace MGE {

/// @addtogroup GUI_Modules
/// @{
/// @file

/**
 * @brief 3D Gui progress bar
 */
class ProgressBar3D : public MGE::GUI3D {
public:
	/**
	 * @brief constructor
	 * 
	 * @param _parent        scene node to attach progressbar
	 * @param _nodeName      unique name of progressbar (can't use name of parent scene node)
	 * @param _offset        offset betwen @a _parent and progressbar
	 * @param _isNotMovable  set true for enable optimalizations for non movement @a _parent scene node
	 */
	ProgressBar3D(Ogre::SceneNode* _parent, const Ogre::String& _nodeName, const Ogre::Vector3& _offset = Ogre::Vector3(0, 3.9, 0), bool _isNotMovable = false);
	
	/// destructor
	~ProgressBar3D();
	
	/**
	 * @brief set progress bar value (0.0 - 1.0)
	 */
	inline void setProgress(float progress) {
		pbWin->setProgress(progress);
	}
	
	/**
	 * @brief set progress bar value (0.0 - 1.0) and colour (32bit ARGB value)
	 */
	void setProgress(float progress, CEGUI::argb_t newColourARGB);
	
	/**
	 * @brief return progress bar numeric value
	 */
	inline float getValue() {
		return pbWin->getProgress();
	}
	
	/**
	 * @brief return progress bar colour
	 */
	inline CEGUI::argb_t getColour() {
		return colourARGB;
	}
	
private:
	/// pointer to CEGUI ProgressBar element
	CEGUI::Window*      bgWin;
	CEGUI::ProgressBar* pbWin;
	
	/// colour of progress bar
	CEGUI::argb_t       colourARGB;
};

/// @}

}
