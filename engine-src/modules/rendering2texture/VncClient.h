/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "StoreRestoreSystem.h"

#include "ModuleBase.h"

#include "input/InteractiveTexture.h"
#include "modules/utils/AsioSyn.h"

#include <thread>
#include <OgreFrameListener.h>

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief VNC (remote framebuffer) client based on interactive texture object
 * 
 * @note
 *   * Tested with VLC server "tigervnc-standalone-server" 1.9.0 with `vncserver` command line options: `-geometry 1024x768 -depth 24 -SecurityTypes None`
 *   * RFB protocol reference:
 *      * RFC 6143 (https://tools.ietf.org/html/rfc6143)
 *      * http://www.realvnc.com/docs/rfbproto.pdf
 *      * MIT license python code vncdotool (https://github.com/sibson/vncdotool)
 */
class VNCclient :
	public MGE::InteractiveTexture,
	public MGE::Unloadable,
	public MGE::Module,
	public Ogre::FrameListener,
	public MGE::AsioSyn
{
public:
	/// Ogre::FrameListener interface
	bool frameStarted(const Ogre::FrameEvent& evt) override;
	
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
	
	// pause refreshing texture
	void pause() { isPaused=true; }
	
	// unpause refreshing texture
	void unpause() { isPaused=false; }
	
	/**
	 * @brief create VNC client, rendering texture and (optional) CEGUI image
	 * 
	 * @param _objectName    base name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param host           host to connect
	 * @param display        display (on @a host) to use
	 * @param _mode          type of interactive texture (see @ref MGE::InteractiveTexture::Mode)
	 * @param _scnMgr        (need only when _mode == OnOgreObject) pointer to Ogre::SceneManager owned Ogre::Node with Ogre::Entity on witch put texture
	 * @param _isInteractive set to true when this texture should to take the input
	 * @param _isNotMovable  set to true (deafault is false) when ogre object can't be moved, roteted or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _disableAlpha  set to true (deafault is true) to disable transparency from alpha channel
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 */
	VNCclient(
		const std::string_view& _objectName, const std::string& host, int display,
		MGE::InteractiveTexture::Mode _mode = MGE::InteractiveTexture::OnGUIWindow,
		Ogre::SceneManager* _scnMgr = NULL, bool _isInteractive = true, bool _isNotMovable = false, bool _disableAlpha = true, Ogre::MovableObject* _ogreObject = NULL
	);
	
	/**
	 * @brief create VNC client, rendering texture and (optional) CEGUI image based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 * @param[in] context           creation context (provides access to SceneManager, etc)
	 */
	static VNCclient* create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// destructor
	~VNCclient();
	
private:
	char*                  screenBuf;
	bool                   screenBufNeedRedraw;
	std::size_t            screenBufSize;
	std::size_t            screenLineSize;
	
	std::thread*           networkListener;
	std::thread*           networkSender;
	char                   vncRequestMode;
	
	Ogre::Vector2          lastMouseTexturePos;
	bool                   haveInput;
	bool                   haveCursor;
	bool                   isPaused;
	
	void parseServerMessage(const boost::system::error_code& ec, std::size_t transferredBytes, char* messageBuffor);
	[[ noreturn ]] void rfbListener();
	[[ noreturn ]] void rfbSender();
	
	void sendFramebufferUpdateRequest(char incremental, bool doPool);
	void sendMouseEvent(const OIS::MouseEvent& arg, uint8_t buttonMask = 0);
	void sendKeyEvent(const OIS::KeyEvent& arg, bool isDown);
	
	uint32_t getX11KeySym(OIS::KeyCode key, uint32_t text, bool isDown);
};

/// @}

}
