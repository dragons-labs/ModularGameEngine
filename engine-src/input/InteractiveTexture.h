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

#include "config.h"

#include "BaseClasses.h"
#include "StringTypedefs.h"

#include <OgreSceneManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

namespace CEGUI {
	class  Window;
	class  Texture;
	class  BitmapImage;
}
namespace Ogre {
	class HlmsUnlitDatablock;
}

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief base class for interactive texture object
 */
class InteractiveTexture : MGE::NoCopyableNoMovable {
public:
	/// modes of InteractiveTexture
	enum Mode {
		/// texture is on Ogre object  - get texture coordinates using raycasting to the polygon level
		OnOgreObject,
		/// texture is on CEGUI window - use window mouse position
		OnGUIWindow
	};
	
	/**
	 * @brief constructor
	 * 
	 * @param _namePrefix    prefix name for object (used for create other names based on prefix and surfix), name of module using InteractiveTexture
	 * @param _objectName    surfix name for object (used for create other names based on prefix and surfix), name of Ogre::MovableObject, GameObject or CEGUI::Window using this texture
	 * @param _mode          type of interactive texture (see @ref MGE::InteractiveTexture::Mode)
	 * @param _scnMgr        (need only when _mode == OnOgreObject) pointer to Ogre::SceneManager owned Ogre::Node with Ogre::Entity on witch put texture
	 * @param _isNotMovable  set to true (deafault is false) when ogre object can't be moved, roteted or scaled after call this constructor; has effect only in OnOgreObject mode
	 * @param _disableAlpha  set to true (deafault is false) to disable transparency from alpha channel
	 * @param _ogreObject    pointer to Ogre::MovableObject for set texture (when NULL get based on @a _objectName (MovableObject must be attached to SceneNode with this same name for this feature))
	 */
	InteractiveTexture(
		const std::string_view& _namePrefix,
		const std::string_view& _objectName,
		Mode _mode,
		Ogre::SceneManager*
		_scnMgr = NULL,
		bool _isNotMovable = false,
		bool _disableAlpha = false,
		Ogre::MovableObject* _ogreObject = NULL
	);
	
	/// destructor
	~InteractiveTexture();
	
	/**
	 * @brief call on mouse pressed
	 * 
	 * @param mouseTexturePos       relative to texture mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	virtual bool mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) { return true; }
	
	/**
	 * @brief call on mouse moved
	 * 
	 * @param mousePos              mouse position to process in textureHitTest()
	 * @param arg                   OIS mouse event
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 * 
	 * @note for calculation new mouse position use:
	   \code{.cpp}
	     std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	     if(res.first) { xxx(res.second); }
	   \endcode
	 */
	virtual bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg ) { return true; }
	
	/**
	 * @brief call on mouse released
	 * 
	 * @param mousePos              mouse position to process in textureHitTest()
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	virtual bool mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg ) { return true; }
	
	/**
	 * @brief call on key pressed
	 * 
	 * @param arg                   OIS mouse event
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	virtual bool keyPressed(const OIS::KeyEvent& arg) { return true; }
	
	/**
	 * @brief call on key released
	 * 
	 * @param arg                   OIS mouse event
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	virtual bool keyReleased(const OIS::KeyEvent& arg) { return true; }
	
	/**
	 * @brief call on lost input (click outside InteractiveTexture)
	 * 
	 * @param toTexture             pointer to InteractiveTexture object which want to take input (NULL when go to GUI or main)
	 * @param toGUI                 true when GUI want to take input
	 * 
	 * @return true when lost input is allowed for this situation, can be ignored
	 */
	virtual bool lostInput(InteractiveTexture* toTexture, bool toGUI) { return true; }
	
	
	/// return Ogre and CEGUI texture name
	virtual std::string getTextureName() const {
		return namePrefix + "Texture" + objectName;
	}
	
	/// return Ogre material Name
	virtual std::string getMaterialName() const {
		if (mode == OnOgreObject)
			return namePrefix + "Material" + objectName;
		else
			return "";
	}
	
	/// return CEGUI Image name
	virtual std::string getImageName() const {
		if (mode == OnGUIWindow)
			return namePrefix + "Image" + objectName;
		else
			return "";
	}
	
	/**
	 * @brief return object name (Ogre::MovableObject, GameObject or CEGUI::Window)
	 * 
	 * example of usage for CEGUI (see also: @ref putOnGUIWindow):
	 * \code{cpp}
		InteractiveTexture* iTexObj = ...;
		float x = 0.65, y = 0.65, w = 0.35, h = 0.35;
		CEGUI::Window* win = CEGUI::WindowManager::getSingleton().createWindow("InteractiveTexture", iTexObj->getObjectName());
		win->setSize(CEGUI::USize(CEGUI::UDim(w, 0), CEGUI::UDim(h, 0)));
		win->setPosition(CEGUI::UVector2(CEGUI::UDim(x, 0), CEGUI::UDim(y, 0)));
		win->setProperty("Image", iTexObj->getImageName());
		MGE::GUISystem::getPtr()->getMainWindow()->addChild(win);
	 * \endcode
	 */
	inline const std::string& getObjectName() const {
		return objectName;
	}
	
	/// set "Image" attribute of CEGUI window identified by @a winName to this texture image
	void putOnGUIWindow(const std::string_view& winName);
	
friend class InteractiveTextureManager;
protected:
	/**
	 * @brief create Ogre texture (@ref namePrefix + "Texture" + @ref objectName)
	 *        and depending on the mode create Ogre material (@ref namePrefix + "Material" + @ref objectName)
	 *        or CEGUI texture and image (@ref namePrefix + "Image" + @ref objectName)
	 */
	Ogre::TextureGpu* createTexture(
		int xSize, int ySize,
		bool isInteractive = true,
		int usage = Ogre::TextureFlags::ManualTexture,
		Ogre::PixelFormatGpu format = Ogre::PFG_RGBA8_UNORM
	);
	
	/**
	 * @brief change size of texture
	 */
	Ogre::TextureGpu* resizeTexture(int xSize, int ySize);
	
	/// create Ogre Material (@ref ogreMaterial) and set on Entity with name == objectName
	void createMaterialOnOgreObject(bool isInteractive = true);
	
	/// create CEGUI Image
	void createImageForCEGUIWindow(bool autoscale = false);

	/// do raycasting to the polygon level
	std::pair<bool, Ogre::Vector2> ogreObjectHitTest(const Ogre::Ray& mouseRay);
	
	/// do texture hit test
	std::pair<bool, Ogre::Vector2> textureHitTest(const Ogre::Vector2& mousePos);
	
	/// copy @a data to texture
	void fillTexture(const Ogre::uint8* data);
	
	/// all vertices
	std::vector<Ogre::Vector3> vertices;
	
	/// vertex UV coords
	std::vector<Ogre::Vector2> UVs;
	
	/// vertex indices
	std::vector<int>           indices;
	
	/// true when ogreObject is BillboardSet and need (re)get vertices info in ogreObjectHitTest
	bool                       getBillboardInfoWhenDoTest;
	
	/// window using for (current) interaction with texture
	CEGUI::Window*             clickWindow;
	
	/// texture mode - see @ref Mode
	Mode                       mode;
	
	/// pointer to Ogre::SceneManager with node / movable object @a objectName
	/// can be NULL when mode != OnOgreObject
	Ogre::SceneManager*        scnMgr;
	
	/// texture name prefix (name of module usigng this texture)
	std::string     namePrefix;
	
	/// texture name node (name of game object usigng this texture)
	std::string     objectName;
	
	/// pointer to Ogre::Entity
	/// can be NULL when mode != OnOgreObject
	Ogre::MovableObject*       ogreObject;
	
	/// true when ogre object can't be moved, roteted or scaled
	bool                       isNotMovable;
	
	/// true when do not use alpha channel / transparency
	bool                       disableAlpha;
	
	/// pointer to Ogre texture
	Ogre::TextureGpu*          renderTexture;
	
	/// pointer to Ogre datablock (material)
	Ogre::HlmsUnlitDatablock*  ogreDatablock;
	
	/// pointer to CEGUI texture
	CEGUI::Texture*            guiTexture;
	
	/// pointer to CEGUI image
	CEGUI::BitmapImage*        guiImage;
};


/**
 * @brief handling OIS event for interactive texture object
 */
class InteractiveTextureManager MGE_CLASS_FINAL :
	public MGE::TrivialSingleton<InteractiveTextureManager>
{
public:
	/**
	 * @brief do raycast and check if results contains object with interactive texture,
	 *        calculate click position, set activeTextureObject
	 *        and call mousePressed on activeTextureObject
	 * 
	 * @param mouseWindowPos        relative to @a fromWindow mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param activeTextureObject   reference to InteractiveTexture pointer, input value is used to compare for detection change of active InteractiveTexture, function update InteractiveTexture pointer if need
	 * @param fromWindow            CEGUI window used to do this input action (NULL when main)
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	bool mousePressedOnWorld(
		const Ogre::Vector2& mouseWindowPos,
		OIS::MouseButtonID buttonID,
		const OIS::MouseEvent& arg,
		InteractiveTexture*& activeTextureObject,
		CEGUI::Window* fromWindow = NULL
	);
	
	/**
	 * @brief check if clicked GUI window contains object with interactive texture,
	 *        calculate click position, set activeTextureObject
	 *        and call mousePressed on activeTextureObject
	 * 
	 * @param mouseWindowPos        relative to @a window mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param activeTextureObject   reference to InteractiveTexture pointer, input value is used to compare for detection change of active InteractiveTexture, function update InteractiveTexture pointer if need
	 * @param window                CEGUI window used to do this input action (NULL when main)
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	bool mousePressedOnGUI(
		const Ogre::Vector2& mouseWindowPos,
		OIS::MouseButtonID buttonID,
		const OIS::MouseEvent& arg,
		InteractiveTexture*& activeTextureObject,
		CEGUI::Window* window = NULL
	);
	
	/**
	 * @brief unset @a activeTextureObject (before unset call lostInput() on it)
	 * 
	 * @param activeTextureObject   reference to InteractiveTexture pointer, to unset
	 * @param toGUI                 true when input go to GUI
	 */
	static void unset(InteractiveTexture*& activeTextureObject, bool toGUI = false);
	
	/**
	 * @brief call mouseMoved on activeTextureObject
	 * 
	 * @param mousePos              mouse position to process in textureHitTest()
	 * @param arg                   OIS mouse event
	 * @param activeTextureObject   reference to InteractiveTexture, used to to verify necessary call mouseMoved
	 * @param mode                  type of interactive texture to process
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	static inline bool mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, InteractiveTexture* activeTextureObject, MGE::InteractiveTexture::Mode mode) {
		if (activeTextureObject && mode == activeTextureObject->mode) {
			return activeTextureObject->mouseMoved(mousePos, arg);
		} else {
			return false;
		}
	}
	
	/**
	 * @brief call mouseReleased on activeTextureObject
	 * 
	 * @param mousePos              mouse position to process in textureHitTest()
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param activeTextureObject   reference to InteractiveTexture, used to to verify necessary call mouseMoved
	 * @param mode                  type of interactive texture to process
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	static inline bool mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, InteractiveTexture* activeTextureObject, MGE::InteractiveTexture::Mode mode) {
		if (activeTextureObject && mode == activeTextureObject->mode) {
			return activeTextureObject->mouseReleased(mousePos, buttonID, arg);
		} else {
			return false;
		}
	}
	
	/**
	 * @brief call keyPressed on activeTextureObject
	 * 
	 * @param arg                   OIS key event
	 * @param activeTextureObject   reference to InteractiveTexture, used to to verify necessary call mouseMoved
	 * @param mode                  type of interactive texture to process
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	static inline bool keyPressed(const OIS::KeyEvent& arg, InteractiveTexture* activeTextureObject, MGE::InteractiveTexture::Mode mode) {
		if (activeTextureObject && mode == activeTextureObject->mode) {
			return activeTextureObject->keyPressed(arg);
		} else {
			return false;
		}
	}
	
	/**
	 * @brief call keyReleased on activeTextureObject
	 * 
	 * @param arg                   OIS key event
	 * @param activeTextureObject   reference to InteractiveTexture, used to to verify necessary call mouseMoved
	 * @param mode                  type of interactive texture to process
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	static inline bool keyReleased(const OIS::KeyEvent& arg, InteractiveTexture* activeTextureObject, MGE::InteractiveTexture::Mode mode) {
		if (activeTextureObject && mode == activeTextureObject->mode) {
			return activeTextureObject->keyReleased(arg);
		} else {
			return false;
		}
	}
	
	/**
	 * @brief register texture listener
	 * 
	 * @param objectName    key in listeners map, used to compare with clicked object name
	 * @param textureObject pointer to InteractiveTexture
	 */
	void addTextureListener(
		const std::string_view& objectName,
		MGE::InteractiveTexture* textureObject
	);
	
	/**
	 * @brief unregister texture listener
	 * 
	 * @param objectName    key in listeners map
	 */
	void remTextureListener(const std::string_view& objectName);
	
	/**
	 * @brief return registred texture listener by object name
	 * 
	 * @param objectName    key in listeners map
	 */
	MGE::InteractiveTexture* getTextureListener(const std::string_view& objectName);
	
protected:
	/// map of all InteractiveTexture objects
	///
	/// @li key is Ogre::Node name == Ogre::MovableObject name
	/// @li value is pointer to InteractiveTexture object
	std::map<std::string, MGE::InteractiveTexture*, std::less<>> listeners;
	
	/**
	 * @brief proccess hit for use in mousePressedOn* functions
	 * 
	 * @param newTextureObject      pointer to new InteractiveTexture object, NULL when no hit
	 * @param position              position on texture
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param activeTextureObject   reference to InteractiveTexture pointer, input value is used to compare for detection change of active InteractiveTexture, function update InteractiveTexture pointer if need
	 * @param window                CEGUI window used to do this input action (NULL when main)
	 * @param info                  info string for log
	 * 
	 * @return true if event put to InteractiveTexture, and InteractiveTexture handle it
	 */
	bool _processHit(
		InteractiveTexture* newTextureObject,
		const Ogre::Vector2& position, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg,
		InteractiveTexture*& activeTextureObject, CEGUI::Window* window, MGE::null_end_string info
	);
	
protected:
	friend class TrivialSingleton;
	InteractiveTextureManager()  = default;
	~InteractiveTextureManager() = default;
};

/// @}

}
