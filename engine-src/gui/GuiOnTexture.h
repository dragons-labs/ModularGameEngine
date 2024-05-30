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

#include "input/InteractiveTexture.h"

namespace CEGUI { class Window; class GUIContext; class TextureTarget; }

namespace MGE {

/// @addtogroup GUI_Core
/// @{
/// @file

/**
 * @brief Gui on Ogre texture base class
 */
class GUIOnTexture : public MGE::InteractiveTexture {
public:
	/**
	 * @brief constructor
	 * 
	 * @param _objectName    base name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param _xSize         x resolution of GUI texture
	 * @param _ySize         y resolution of GUI texture
	 * @param _scnMgr        pointer to Ogre::SceneManager owned Ogre::Node with Ogre::Entity on witch put texture
	 * @param _isInteractive set to true when this texture should to take the input
	 * @param _isNotMovable  set to true (default is false) when ogre object can't be moved, rotated or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 */
	GUIOnTexture(const std::string_view& _objectName, int _xSize, int _ySize, Ogre::SceneManager* _scnMgr, bool _isInteractive = true, bool _isNotMovable = false, Ogre::MovableObject* _ogreObject = NULL);
	
	/**
	 * @brief destructor
	 */
	virtual ~GUIOnTexture();
	
	/**
	 * @brief redraw (update) GUI texture (only when guiContext is dirty)
	 */
	void redraw();
	
	/// @copydoc MGE::InteractiveTextureManager::getTextureName
	std::string getTextureName() const override {
		return renderTexture->getNameStr();
	}
	
	/**
	 * @brief return root CEGUI windows for this 3D GUI object
	 */
	inline CEGUI::Window* getRootWindow() {
		return rootWindow;
	}
	
	/**
	 * @brief return CEGUI context for this 3D GUI object
	 */
	inline CEGUI::GUIContext* getContext() {
		return guiContext;
	}
	
	/// @name MGE::InteractiveTexture %input interface
	/// @{
		/// @copydoc MGE::InteractiveTexture::mousePressed
		virtual bool mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) override;
		
		/// @copydoc MGE::InteractiveTexture::mouseMoved
		virtual bool mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) override;
		
		/// @copydoc MGE::InteractiveTexture::mouseReleased
		virtual bool mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) override;
		
		/// @copydoc MGE::InteractiveTexture::keyPressed
		virtual bool keyPressed(const OIS::KeyEvent& arg) override;
		
		/// @copydoc MGE::InteractiveTexture::keyReleased
		virtual bool keyReleased(const OIS::KeyEvent& arg) override;
	/// @}
	
protected:
	/// render target for GUI
	CEGUI::TextureTarget*        renderTextureTarget;
	
	/// pointer to 3DGUI GUI Context
	CEGUI::GUIContext*           guiContext;
	
	/// pointer to 3DGUI GUI root window
	CEGUI::Window*               rootWindow;
};

/// @}

}
