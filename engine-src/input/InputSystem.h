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

#include "ListenerSet.h"
#include "BaseClasses.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OgreVector2.h>

namespace CEGUI { class Window; }

namespace MGE { struct EventMsg; }
namespace MGE { class InteractiveTexture; }

namespace MGE {

/// @addtogroup Input
/// @{
/// @file

/**
 * @brief OIS initialising class.
 */
class InputSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public OIS::KeyListener,
	public OIS::MouseListener,
	public MGE::Singleton<InputSystem> 
{
public:
	/**
	 * @brief   update input status and (if need) call input listeners
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/**
	 * @brief call @ref update
	 * 
	 * @copydoc MGE::MainLoopListener::updateOnFullPause
	 */
	bool updateOnFullPause(float realTimeStep) override { return update(0, realTimeStep); }
	
	
	/// @name get input status
	/// @{
		/**
		 * @brief return OIS mouse state
		 */
		inline const OIS::MouseState& getMouseState() const {
			return mouseInput->getMouseState();
		}
		
		/**
		 * @brief return true if key is down
		 */
		inline bool isKeyDown(OIS::KeyCode key) const {
			return keyboardInput->isKeyDown(key);
		}
		
		/**
		 * @brief return true if modifier is down
		 */
		inline bool isModifierDown(OIS::Keyboard::Modifier mod) const {
			return keyboardInput->isModifierDown(mod);
		}
	/// @}
	
	/// @name OIS::MouseListener
	/// @{
		/// On mouse key press event
		virtual bool mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID buttonID ) override;
		/// On mouse move event
		virtual bool mouseMoved( const OIS::MouseEvent& arg ) override;
		/// On mouse key released event
		virtual bool mouseReleased( const OIS::MouseEvent& arg, OIS::MouseButtonID buttonID ) override;
	/// @}
	
	/// @name OIS::KeyListener
	/// @{
		/// On key press event
		virtual bool keyPressed( const OIS::KeyEvent& arg ) override;
		/// On key released event
		virtual bool keyReleased( const OIS::KeyEvent& arg ) override;
	/// @}
	
	/**
	 * @brief  on mouse pressed on world (non GUI)
	 * 
	 * @param mousePos              relative mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * @param fromWindow            CEGUI window used to do this input action (NULL when main)
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	bool mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* fromWindow = NULL );
	
	/**
	 * @brief  on mouse moved on world (non GUI)
	 * 
	 * @param mousePos              relative mouse position
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject );
	
	/**
	 * @brief  on mouse released on world (non GUI)
	 * 
	 * @param mousePos              relative mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	bool mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject );
	
	/**
	 * @brief  on key pressed on world (non GUI)
	 * 
	 * @param arg                   OIS key event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject );
	
	/**
	 * @brief  on key released on world (non GUI)
	 * 
	 * @param arg                   OIS key event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	bool keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject );
	
	/**
	 * @brief  on lost input (unclick SubView InteractiveTexture) or lost mouse hover (mouse moved outside SubView InteractiveTexture)
	 * 
	 * @param full  true when lost all input (unclick), false when only lost mouse hover
	 */
	virtual void lostInput(bool full = false);
	
	/// base class for mouse input listeners
	class Listener;
	
	/**
	 * @brief register input events listener
	 * 
	 * @param listener     pointer to listener object
	 * @param mousePress   priority for mouse pressed event (use -1 to skip registration for this event)
	 * @param mouseMove    priority for mouse moved event (use -1 to skip registration for this event)
	 * @param mosueRelease priority for mosue released event (use -1 to skip registration for this event)
	 * @param lostInput    priority for lost input event (use -1 to skip registration for this event)
	 * @param keyPressed   priority for key pressed event (use -1 to skip registration for this event)
	 * @param keyReleased  priority for key released event (use -1 to skip registration for this event)
	 */
	void registerListener(Listener* listener, int mousePress, int mouseMove, int mosueRelease, int lostInput, int keyPressed, int keyReleased);
	
	/**
	 * @brief unregister input events listener
	 */
	void unregisterListener(Listener* listener);
	
	/// type of object registered in @ref hightPriorityKeyPressedListener
	/// functor returned `bool` with single `const OIS::KeyEvent&` argument
	typedef MGE::FunctorListenerClassBase<bool, const OIS::KeyEvent&> KeyPressedListenerFunctor;
	
	/// set of hight priority keyboard listeners
	///
	/// these listeners will be executed before any other keyboard listeners
	/// and regardless of the current input context (on gui, on worl, on interactive texture, etc.)
	MGE::ClassListenerSet<KeyPressedListenerFunctor, KeyPressedListenerFunctor> hightPriorityKeyPressedListener;
	
	/// base class for input aggregator (for mouse management and GUI integration)
	class InputAggregatorBase;
	
	/// set input aggregator (previous aggregator will be destroyed)
	void setInputAggregator(InputAggregatorBase *aggregator);
	
	/// get input aggregator
	InputAggregatorBase* getInputAggregator() { return inputAggregator; }
	
	/// return OIS keyboard object
	OIS::Keyboard*  getKeyboard() { return keyboardInput; }
	
	/// return OIS mouse object
	OIS::Mouse*  getMouse() { return mouseInput; }
	
	/// constructor - initialise OIS input system
	InputSystem();
	
	/// destructor - destroy OIS input system
	~InputSystem();
	
protected:
	/// callback function for "WindowEvent" messages
	void onWindowEvent(const MGE::EventMsg* eventMsg, void* regOwnerID);
	
	/// update (fix) mouse after window set size
	void onWindowResized();
	
	/// set of keyboard listeners
	MGE::ClassPtrListenerSet<Listener, int> keyPressedListeners;
	
	/// set of keyboard listeners
	MGE::ClassPtrListenerSet<Listener, int> keyReleasedListeners;
	
	/// set of lost input listeners
	MGE::ClassPtrListenerSet<Listener, int> lostInputListeners;
	
	/// set of mouse listeners
	MGE::ClassPtrListenerSet<Listener, int> mousePressedListeners;
	
	/// set of mouse listeners
	MGE::ClassPtrListenerSet<Listener, int> mouseMovedListeners;
	
	/// set of mouse listeners
	MGE::ClassPtrListenerSet<Listener, int> mouseReleasedListeners;
	
	/// pointer to gui input aggregator (used also for mouse position calculations)
	InputAggregatorBase*      inputAggregator;
	
	/// pointer to OIS InputManager
	OIS::InputManager*        inputManager;
	/// pointer to OIS Keyboard
	OIS::Keyboard*            keyboardInput;
	/// pointer to OIS Mouse
	OIS::Mouse*               mouseInput;
	/// pointer to active (clicked - selected in last mouse click) MGE::InteractiveTexture object, is NULL when no active MGE::InteractiveTexture object
	MGE::InteractiveTexture* activeTextureObject;
	
};

class InputSystem::Listener {
public:
	enum ExecutionOrder {
		CAMERA_CONTROL = 16,
		SELECTION_CONTINUE = 32,
		SELECTION_INIT = 4096
	};
	
	/**
	 * @brief  on mouse pressed
	 * 
	 * @param mousePos              relative mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * @param fromWindow            CEGUI window used to do this input action (NULL when main)
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	virtual bool mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* fromWindow = NULL ) { return false; }
	
	/**
	 * @brief  on mouse moved
	 * 
	 * @param mousePos              relative mouse position
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	virtual bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) { return false; }
	
	/**
	 * @brief  on mouse released
	 * 
	 * @param mousePos              relative mouse position
	 * @param buttonID              OIS mouse button ID
	 * @param arg                   OIS mouse event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	virtual bool mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) { return false; }
	
	/**
	 * @brief  on key pressed
	 * 
	 * @param arg                   OIS key event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	virtual bool keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) { return false; }
	
	/**
	 * @brief  on key released
	 * 
	 * @param arg                   OIS key event
	 * @param _activeTextureObject  pointer to MGE::InteractiveTexture pointer, input value is used to compare for detection change of active MGE::InteractiveTexture, function update MGE::InteractiveTexture pointer if need
	 * 
	 * @return true if event put to MGE::InteractiveTexture, and MGE::InteractiveTexture handle it
	 */
	virtual bool keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) { return false; }
	
	/**
	 * @brief  on lost input
	 * 
	 * @param full  true when lost all input (unclick), false when only lost mouse hover
	 */
	virtual bool lostInput(bool full = false) { return false; }
	
	/// destructor
	virtual ~Listener() {}
};


class InputSystem::InputAggregatorBase {
public:
	/**
	 * @brief send key down event from OIS event listener to GUI
	 * 
	 * @param arg       OIS KeyEvent detail/description
	 */
	virtual bool injectKeyDown(const OIS::KeyEvent& arg) { return false; }
	
	/**
	 * @brief send key up event from OIS event listener to GUI
	 * 
	 * @param arg       OIS KeyEvent detail/description
	 */
	virtual bool injectKeyUp(const OIS::KeyEvent& arg) { return false; }
	
	/**
	 * @brief send mouse key down event from OIS event listener to GUI
	 * 
	 * @param buttonID  OIS mouse button ID
	 */
	virtual bool injectMouseButtonDown(OIS::MouseButtonID buttonID, CEGUI::Window* hitWindow) { return false; }
	
	/**
	 * @brief send mouse key up event from OIS event listener to GUI
	 * 
	 * @param buttonID  OIS mouse button ID
	 */
	virtual bool injectMouseButtonUp(OIS::MouseButtonID buttonID) { return false; }
	
	/**
	 * @brief send mouse move (include wheel move) event from OIS event listener to GUI
	 * 
	 * @param x  x axis mouse move size
	 * @param y  y axis mouse move size
	 * @param z  z axis (wheel) mouse move size
	 */
	virtual bool injectMouseMove(float x, float y, float z) { return false; }
	
	/**
	 * @brief Return current mouse position
	 * 
	 * @param arg  OIS mouse event that may be (or may be not) used to calculate returned mouse position
	 */
	virtual Ogre::Vector2 getMousePosition(const OIS::MouseEvent& arg);
	
	/**
	 * @brief Return the specific position as window resolution/size independant values (range from 0.0f to 1.0f),
	 *        if window is null use main application window
	 * 
	 * @param[in] position    screen position to convert
	 * @param[in] window      pointer to window, using to calculate resolution/size independant position
	 */
	virtual Ogre::Vector2 calcViewportRelativePosition(Ogre::Vector2 position, const CEGUI::Window* window = nullptr);
	
	/// destructor
	virtual ~InputAggregatorBase() {}
};

/// @}

}
