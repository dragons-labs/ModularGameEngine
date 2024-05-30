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

#include "ModuleBase.h"

#include "input/InteractiveTexture.h"

#include <OgreFrameListener.h>

class CefMainArgs;
class MyCefApp;
class BrowserClient;

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief web browser with Chromium Embedded Framework based on interactive texture object
 */
class WebBrowser :
	public Ogre::FrameListener,
	public MGE::Module,
	public MGE::InteractiveTexture,
	public MGE::Unloadable
{
public:
	/**
	 * @brief create CEF web browser, rendering texture and (optional) CEGUI image
	 * 
	 * @param _objectName    base name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param _xSize         x resolution of GUI texture
	 * @param _ySize         y resolution of GUI texture
	 * @param url            initial URL for browser
	 * @param _mode          type of interactive texture (see @ref MGE::InteractiveTexture::Mode)
	 * @param _scnMgr        (need only when _mode == OnOgreObject) pointer to Ogre::SceneManager owned Ogre::Node with Ogre::Entity on witch put texture
	 * @param _isInteractive set to true when this texture should to take the input
	 * @param _isNotMovable  set to true (default is false) when ogre object can't be moved, rotated or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 */
	WebBrowser(
		const std::string_view& _objectName, int _xSize, int _ySize, const std::string_view& url = "about:blank",
		MGE::InteractiveTexture::Mode _mode = MGE::InteractiveTexture::OnOgreObject,
		Ogre::SceneManager* _scnMgr = NULL, bool _isInteractive = true, bool _isNotMovable = false, Ogre::MovableObject* _ogreObject = NULL
	);
	
	/**
	 * @brief create CEF web browser, rendering texture and (optional) CEGUI image based on XML configuration
	 * 
	 * @param[in] xmlNode           XML configuration node
	 * @param[in] context           creation context (provides access to SceneManager, etc)
	 */
	static WebBrowser* create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// destructor
	~WebBrowser();
	
	// get WebBrowser by name
	static WebBrowser* getBrowser(const std::string_view& name);
	
	/// goto @a url
	void loadURL(const std::string_view& url);
	
	/// show HTML from string
	void loadString(const std::string_view& html);
	
	/// go back
	void goBack();
	
	/// go forward
	void goForward();
	
	/// reload current page
	void reload();
	
	/// stop loading
	void stopLoad();
	
	/// return true if browser is currently loading
	bool isLoading();
	
	/// return true if a document has been loaded in the browser
	bool hasDocument();
	
	/// @copydoc MGE::InteractiveTexture::mousePressed
	bool mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) override;
	
	/// @copydoc MGE::InteractiveTexture::mouseMoved
	bool mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) override;
	
	/// @copydoc MGE::InteractiveTexture::mouseReleased
	bool mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg ) override;
	
	/// @copydoc MGE::InteractiveTexture::keyPressed
	bool keyPressed(const OIS::KeyEvent& arg) override;
	
	/// @copydoc MGE::InteractiveTexture::keyReleased
	bool keyReleased(const OIS::KeyEvent& arg) override;
	
	/// Ogre::FrameListener interface
	bool frameStarted(const Ogre::FrameEvent& evt) override;
	
	/// resize window to @a xSize x @a ySize
	void resize(int xSize, int ySize);
	
	/// return parsed URL (support "rpath://" for relative path)
	static std::string parseUrl(const std::string_view& url);
	
	/**
	 * @brief split url into scheme part, path part and query
	 * 
	 * @param[in]  url        full url as string
	 * @param[out] scheme     string to put scheme (before first ":") part of \a url
	 * @param[out] path       string to put path part (between first ":" and first "?") of \a url
	 * @param[out] query      string map to put key-value pairs from query part (after "?") of \a url
	 * @param[in]  needScheme when not empty compare scheme getting from \a url with element of it and when not equal with any return false
	 *                        (do not extract path and query)
	 */
	static bool parseUrl(
		const std::string_view& url, 
		std::string& scheme, std::string& path, std::map<std::string, std::string>& query,
		const std::set<std::string>& needScheme = std::set<std::string>()
	);
	
	/**
	 * @brief decode URL-encoded string (convert %xx to char)
	 */
	static std::string decodeUrl(const std::string_view& url);
	
private:
	friend BrowserClient;
	
	static int           webBrowserObjectCount;
	
	BrowserClient*       currentClient;
	BrowserClient*       mainClient;
	BrowserClient*       dialogClient;
	
	bool sendMouseEvent(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID _buttonID, bool mouseUp);
};

/// @}

}
