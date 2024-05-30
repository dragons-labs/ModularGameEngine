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

#include "gui/GuiOnTexture.h"
#include "BaseClasses.h"

#include <OgreBillboardSet.h>

namespace MGE {

/// @addtogroup GUI_Core
/// @{
/// @file

/**
 * @brief Gui in 3D world base class
 */
class GUI3D : MGE::NoCopyableNoMovable {
public:
	/**
	 * @brief constructor - set 3D GUI on Ogre object
	 * 
	 * @param _parent             pointer to scene node which want to attached 3D GUI
	 * @param _name               unique name of 3D GUI (when _autoOrientation == true can use name of parent scene node)
	 * @param _width              width of gui texture object
	 * @param _height             height of gui texture object
	 * @param _offset             offset vector (relative to node position)
	 * @param _inWorldSpace       when true @a _offset is in WORLD coordinate system (axis and unit)
	 *                            eg. positive Y value is always WORLD UP direction and Y value is not scale by node scale
	 * @param _autoOrientation    when true automatically update node orientation to look on camera, when false use @a _orientation
	 * @param _orientation        gui plane orientation (relative to parent), use only when @a _autoOrientation == false
	 */
	GUI3D(
		Ogre::SceneNode* _parent,
		const Ogre::String& _name,
		Ogre::Real _width,
		Ogre::Real _height,
		const Ogre::Vector3& _offset  = Ogre::Vector3::ZERO,
		bool _offsetInWorldSpace = false,
		bool _autoOrientation = true,
		const Ogre::Quaternion& _orientation = Ogre::Quaternion::IDENTITY
	);
	
	/**
	 * @brief return BillboardSet using to display this 3D GUI object
	 */
	inline Ogre::v1::BillboardSet* getBillboardSet() {
		return billboardSet;
	}
	
	/**
	 * @brief return scene node using to display this 3D GUI object
	 */
	inline MGE::GUIOnTexture* getGUI() {
		return guiOnTexture;
	}
	
	/**
	 * @brief create GUI
	 * 
	 * @param[in] _resX             X resolution of GUI texture
	 * @param[in] _resY             Y resolution of GUI texture
	 * @param[in] isInteractive     set to true when this GUI should to take the input
	 */
	void setGUI(int _resX, int _resY, bool isInteractive = false);
	
	/**
	 * @brief destructor
	 */
	~GUI3D();
	
protected:
	/// pointer to billboard set
	Ogre::v1::BillboardSet* billboardSet;
	
	/// pointer to GUIOnTexture object
	MGE::GUIOnTexture* guiOnTexture;
};

/// @}

}
