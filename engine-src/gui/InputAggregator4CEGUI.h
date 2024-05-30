/*
Copyright (c) 2017-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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
#include "input/InputSystem.h"

#include <CEGUI/CEGUI.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OgreVector2.h>

namespace MGE {

/// @addtogroup GUI_Core
/// @{
/// @file

/**
 * @brief Input agregator for CEGUI
 */
class InputAggregator4CEGUI : public InputSystem::InputAggregatorBase {
public:
	/// constructor
	InputAggregator4CEGUI(CEGUI::GUIContext* _gContext);
	
	/// @name %input interface
	/// @{
		/// @copydoc InputSystem::InputAggregatorBase::injectKeyDown
		virtual bool injectKeyDown(const OIS::KeyEvent& arg) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::injectKeyUp
		virtual bool injectKeyUp(const OIS::KeyEvent& arg) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::injectMouseButtonDown
		virtual bool injectMouseButtonDown(OIS::MouseButtonID buttonID, CEGUI::Window* hitWindow) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::injectMouseButtonUp
		virtual bool injectMouseButtonUp(OIS::MouseButtonID buttonID) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::injectMouseMove
		virtual bool injectMouseMove(float x, float y, float z) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::getMousePosition
		/// @note using GUI mouse position to avoid desync CEGUI vs OIS mouse position issue
		virtual Ogre::Vector2 getMousePosition(const OIS::MouseEvent& arg) override;
		
		/// @copydoc InputSystem::InputAggregatorBase::calcViewportRelativePosition
		virtual Ogre::Vector2 calcViewportRelativePosition(Ogre::Vector2 position, const CEGUI::Window* window = nullptr) override;
	/// @}
	
	/**
	 * @brief convert keyboard key code to CEGUI
	 * 
	 * @param key  OIS key scancode
	 */
	static CEGUI::Key::Scan convertKey(int key);
	
	/**
	 * @brief convert mouse button from OIS to CEGUI
	 * 
	 * @param buttonID  OIS mouse button ID
	 */
	static CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);
	
private:
	CEGUI::GUIContext* gContext;
};

/// @}

}
