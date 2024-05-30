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

#include "ModuleBase.h"

#include "input/InteractiveTexture.h"
#include "rendering/CameraNode.h"

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief create window for subcamera and create subcamera
 */
class SubView :
	public MGE::InteractiveTexture,
	public MGE::Unloadable,
	public MGE::Module
{
public:
	/// return subcamera name
	std::string getCameraName() const;
	
	/// return subcamera
	MGE::CameraNode* getCamera() const;
	
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
	
	/// @copydoc MGE::InteractiveTexture::lostInput
	virtual bool lostInput(MGE::InteractiveTexture* toTexture, bool toGUI) override;
	
	/**
	 * @brief create subcamera, rendering texture and (optional) CEGUI image
	 * 
	 * @param _objectName    base name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param _cameraCfg     xml archive object, with pointer to xml node with camera configuration
	 * @param _xSize         x resolution of GUI texture
	 * @param _ySize         y resolution of GUI texture
	 * @param _mode          type of interactive texture (see @ref MGE::InteractiveTexture::Mode)
	 * @param _scnMgr        (not NULL) pointer to SceneManager for creating camera
	 * @param _isInteractive set to true when this texture should to take the input
	 * @param _isNotMovable  set to true (deafault is false) when ogre object can't be moved, roteted or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 * @param _context       creation context (passed to MGE::CameraNode::restoreFromXML, can be null)
	 */
	SubView(
		const std::string_view& _objectName,
		const pugi::xml_node& _cameraCfg,
		int _xSize, int _ySize,
		MGE::InteractiveTexture::Mode _mode,
		Ogre::SceneManager* _scnMgr, bool _isInteractive = true, bool _isNotMovable = false, Ogre::MovableObject* _ogreObject = NULL,
		const MGE::LoadingContext* _context = nullptr
	);
	
	/**
	 * @brief create subview for existed camera
	 * 
	 * @param _objectName    base name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param _camera        camera to use
	 * @param _xSize         x resolution of GUI texture
	 * @param _ySize         y resolution of GUI texture
	 * @param _mode          type of interactive texture (see @ref MGE::InteractiveTexture::Mode)
	 * @param _scnMgr        (not NULL) pointer to SceneManager for creating camera
	 * @param _isInteractive set to true when this texture should to take the input
	 * @param _isNotMovable  set to true (deafault is false) when ogre object can't be moved, roteted or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 */
	SubView (
		const std::string_view& _objectName,
		MGE::CameraNode* _camera,
		int _xSize, int _ySize,
		MGE::InteractiveTexture::Mode _mode,
		Ogre::SceneManager* _scnMgr, bool _isInteractive = true, bool _isNotMovable = false, Ogre::MovableObject* _ogreObject = NULL
	);
	
	/**
	 * @brief create subcamera, rendering texture and (optional) CEGUI image based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 * @param[in] context           creation context (provides access to SceneManager, etc)
	 */
	static SubView* create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// destructor
	~SubView();
	
private:
	MGE::CameraNode* camera;
	MGE::InteractiveTexture* activeTextureObject;
	bool hasInput;
	bool hasMouseHover;
};

/// @}

}
