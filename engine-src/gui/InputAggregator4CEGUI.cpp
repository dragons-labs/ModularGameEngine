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

#include "gui/InputAggregator4CEGUI.h"

#include "LogSystem.h"

#include "config.h"

#if defined MGE_DEBUG_LEVEL and MGE_DEBUG_LEVEL > 1
#define DEBUG2_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG2_LOG(a)
#endif

#ifdef MGE_DEBUG_MOUSE_VIEWPORT_POSITION
#define DEBUG_MOUSE_VIEWPORT_POSITION_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG_MOUSE_VIEWPORT_POSITION_LOG(a)
#endif


/*--------------------- OIS to CEGUI convert functions ---------------------*/

CEGUI::Key::Scan MGE::InputAggregator4CEGUI::convertKey(int key) { // `int key` because we convert missing OIS key numbers
	switch (key) {
		case 0x60:
			return CEGUI::Key::Scan::NumpadEnter;
		default:
			return static_cast<CEGUI::Key::Scan>(key);
	}
}

CEGUI::MouseButton MGE::InputAggregator4CEGUI::convertButton(OIS::MouseButtonID buttonID) {
	switch (buttonID) {
		case OIS::MB_Left:
			return CEGUI::MouseButton::Left;
		case OIS::MB_Right:
			return CEGUI::MouseButton::Right;
		case OIS::MB_Middle:
			return CEGUI::MouseButton::Middle;
		default:
			LOG_DEBUG("unsupported mouse button in CEGUI");
			return CEGUI::MouseButton::Left;
	}
}


/*--------------------- OIS to CEGUI "inject" functions ---------------------*/

bool MGE::InputAggregator4CEGUI::injectKeyDown(const OIS::KeyEvent& arg) {
	bool r1 = gContext->injectKeyDown(convertKey(arg.key));
	bool r2 = gContext->injectChar(arg.text); // when OIS TextTranslation == Unicode (default) this is Unicode char number,
	                                          // conversion from UTF-8 (linux) or UTF-16 (windows) is done internally by OIS
	return r1 || r2;
}

bool MGE::InputAggregator4CEGUI::injectKeyUp(const OIS::KeyEvent& arg) {
	return gContext->injectKeyUp(convertKey(arg.key));
}

bool MGE::InputAggregator4CEGUI::injectMouseButtonDown(OIS::MouseButtonID buttonID, CEGUI::Window* window) {
	return gContext->injectMouseButtonDown(convertButton( buttonID ));
}

bool MGE::InputAggregator4CEGUI::injectMouseButtonUp(OIS::MouseButtonID buttonID) {
	return gContext->injectMouseButtonUp(convertButton( buttonID ));
}

bool MGE::InputAggregator4CEGUI::injectMouseMove(float x, float y, float z) {
	// fix wheel move
	if (z > 0)
		z = 1;
	else if (z < 0)
		z = -1;
	
	// inject move
	bool r1 = gContext->injectMouseMove( x, y );
	bool r2 = gContext->injectMouseWheelChange( z );
	
	return r1 || r2;
}

/*--------------------- CEGUI utils functions ---------------------*/

Ogre::Vector2 MGE::InputAggregator4CEGUI::getMousePosition(const OIS::MouseEvent& /*arg*/) {
	auto pos = gContext->getCursorPosition();
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("getMousePosition (GUI) " << pos.x << " " << pos.y);
	return Ogre::Vector2(pos.x, pos.y);
}

Ogre::Vector2 MGE::InputAggregator4CEGUI::calcViewportRelativePosition(Ogre::Vector2 position, const CEGUI::Window* window) {
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("mouse screen position: " << position << " window: " << window);
	
	if (!window)
		window = gContext->getRootWindow();
	
	position.x = (position.x - window->getClipRect().getPosition().x) / window->getInnerRectClipper().getWidth();;
	position.y = (position.y - window->getClipRect().getPosition().y) / window->getInnerRectClipper().getHeight();
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("mouse window relative position: " << position);
	
	return position;
}

/*--------------------- constructor ---------------------*/

MGE::InputAggregator4CEGUI::InputAggregator4CEGUI(CEGUI::GUIContext* _gContext) {
	gContext =_gContext;
}
