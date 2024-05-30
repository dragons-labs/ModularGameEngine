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

#include "input/InputSystem.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "MessagesSystem.h"
#include "with.h"

#include "Engine.h"
#include "rendering/RenderingSystem.h"
#include "rendering/WindowEventMessage.h"

#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"

#include "input/InteractiveTexture.h"

MGE::InputSystem::InputSystem() {
	LOG_HEADER("Initialise OIS input system");
	
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	
	auto ogreGraphics = static_cast<MGE::RenderingSystem*>(MGE::Engine::getPtr()->getModuleThrow("RenderingSystem"));
	ogreGraphics->getRenderWindow()->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	LOG_INFO("using window: " + windowHndStr.str());
	
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	inputManager = OIS::InputManager::createInputSystem( pl );
	
	keyboardInput = static_cast<OIS::Keyboard*>(inputManager->createInputObject( OIS::OISKeyboard, true ));
	mouseInput = static_cast<OIS::Mouse*>(inputManager->createInputObject( OIS::OISMouse, true ));
	
	keyboardInput->setTextTranslation(OIS::Keyboard::Unicode);
	keyboardInput->setEventCallback(this);
	mouseInput->setEventCallback(this);
	
	activeTextureObject = NULL;
	
	// fix mouse
	onWindowResized();
	
	// put mouse pointer on center
	OIS::MouseState& ms = const_cast<OIS::MouseState &>( mouseInput->getMouseState() );
	ms.X.abs = ms.width/2;
	ms.Y.abs = ms.height/2;
	
	inputAggregator = new InputAggregatorBase();
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, INPUT_ACTIONS);
	
	MGE::Engine::getPtr()->getMessagesSystem()->registerReceiver(
		MGE::WindowEventMsg::MsgType,
		std::bind(&MGE::InputSystem::onWindowEvent, this, std::placeholders::_1, std::placeholders::_2),
		this
	);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_Input \<InputSystem\>

@c \<InputSystem\> is used for setup <b>InputSystem</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(InputSystem) {
	return new MGE::InputSystem();
}


void MGE::InputSystem::onWindowResized() {
	unsigned int width, height;
	int left, top;
	OIS::MouseState& ms = const_cast<OIS::MouseState &>( mouseInput->getMouseState() );
	MGE::RenderingSystem::getPtr()->getRenderWindow()->getMetrics(width, height, left, top);
	ms.width = width;
	ms.height = height;
}

void MGE::InputSystem::onWindowEvent(const MGE::EventMsg* eventMsg, void* /*regOwnerID*/) {
	auto msg = static_cast<const MGE::WindowEventMsg*>(eventMsg);
	if (msg->subType == MGE::WindowEventMsg::Closed) {
		delete this;
	} else if (msg->subType == MGE::WindowEventMsg::Resized) {
		onWindowResized();
	}
}

MGE::InputSystem::~InputSystem(void) {
	LOG_INFO("Destroy Input");
	
	if( inputManager ) {
		inputManager->destroyInputObject( mouseInput );
		inputManager->destroyInputObject( keyboardInput );
		
		OIS::InputManager::destroyInputSystem(inputManager);
	}
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
}

bool MGE::InputSystem::update(float gameTimeStep, float realTimeStep) {
	keyboardInput->capture();
	mouseInput->capture();
	return true;
}

void  MGE::InputSystem::registerListener(Listener* listener, int mousePress, int mouseMove, int mosueRelease, int lostInput, int keyPressed, int keyReleased) {
	if (mousePress >= 0)
		mousePressedListeners.addListener(listener, mousePress);
	
	if (mouseMove >= 0)
		mouseMovedListeners.addListener(listener, mouseMove);
	
	if (mosueRelease >= 0)
		mouseReleasedListeners.addListener(listener, mosueRelease);
	
	if (lostInput >= 0)
		lostInputListeners.addListener(listener, lostInput);
	
	if (keyPressed >= 0)
		keyPressedListeners.addListener(listener, keyPressed);
	
	if (keyReleased >= 0)
		keyReleasedListeners.addListener(listener, keyReleased);
}

void MGE::InputSystem::unregisterListener(Listener* listener) {
	mousePressedListeners.remListener(listener);
	mouseMovedListeners.remListener(listener);
	mouseReleasedListeners.remListener(listener);
	lostInputListeners.remListener(listener);
	keyPressedListeners.remListener(listener);
	keyReleasedListeners.remListener(listener);
}

bool MGE::InputSystem::mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID buttonID ) {
	Ogre::Vector2   mouseScreenPos = inputAggregator->getMousePosition(arg);
	CEGUI::Window*  hitWindow      = nullptr;
	bool            guiTexture     = false;
	
	WITH_NOT_NULL(MGE::GUISystem::getPtr(), gui) {
		hitWindow = MGE::GUISystem::findGUIWindow(mouseScreenPos, gui->getMainWindow());
	}
	if (inputAggregator->injectMouseButtonDown( buttonID, hitWindow )) {
		// in most case hitWindow != NULL (because injectMouseButtonDown return true, so mouseScreenPos is on window)
		// but when we have unwraped Combobox it's NOT true
		if (hitWindow && hitWindow->getType() == "InteractiveTexture") {
			guiTexture = true;
		} else {
			MGE::InteractiveTextureManager::unset(activeTextureObject);
			return true;
		}
	}
	
	// calculate relative to hit window mouse position
	Ogre::Vector2 mouseWindowPos = inputAggregator->calcViewportRelativePosition(mouseScreenPos, hitWindow);
	
	if (guiTexture) {
		MGE::InteractiveTextureManager::getPtr()->mousePressedOnGUI(mouseWindowPos, buttonID, arg, activeTextureObject, hitWindow);
		return true;
	}
	
	// no GUI hit, so current window is NULL ... call next step with relative mouse position
	return mousePressed(
		mouseWindowPos,
		buttonID, arg, activeTextureObject, hitWindow
	);
}

bool MGE::InputSystem::mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* fromWindow ) {
	if (MGE::InteractiveTextureManager::getPtr()->mousePressedOnWorld(mousePos, buttonID, arg, _activeTextureObject, fromWindow)) {
		return true;
	}
	
	if (mousePressedListeners.callFirst(&MGE::InputSystem::Listener::mousePressed, mousePos, buttonID, arg, _activeTextureObject, fromWindow)) {
		return true;
	}
	
	return true;
}

bool MGE::InputSystem::mouseMoved( const OIS::MouseEvent& arg ) {
	// update CEGUI
	bool retA = inputAggregator->injectMouseMove( arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel );
	
	// get new position from CEGUI
	Ogre::Vector2 mousePos = inputAggregator->getMousePosition(arg);
	
	// check interactive texture
	bool retB = MGE::InteractiveTextureManager::mouseMoved(mousePos, arg, activeTextureObject, MGE::InteractiveTexture::OnGUIWindow);
	if (retA || retB)
		return true;
	
	// no GUI hit, so current window is NULL ... call next step with relative mouse position
	return mouseMoved(
		inputAggregator->calcViewportRelativePosition(mousePos, NULL),
		arg, activeTextureObject
	);
}

bool MGE::InputSystem::mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (MGE::InteractiveTextureManager::mouseMoved(mousePos, arg, _activeTextureObject, MGE::InteractiveTexture::OnOgreObject))
		return true;
	
	mouseMovedListeners.callFirst(&MGE::InputSystem::Listener::mouseMoved, mousePos, arg, _activeTextureObject);
	
	return true;
}

bool MGE::InputSystem::mouseReleased( const OIS::MouseEvent& arg, OIS::MouseButtonID buttonID ) {
	Ogre::Vector2 mousePos = inputAggregator->getMousePosition(arg);
	
	if (MGE::InteractiveTextureManager::mouseReleased(mousePos, buttonID, arg, activeTextureObject, MGE::InteractiveTexture::OnGUIWindow))
		return true;
	
	if (inputAggregator->injectMouseButtonUp(buttonID))
		return true;
	
	// no GUI hit, so current window is NULL ... call next step with relative mouse position
	return mouseReleased(
		inputAggregator->calcViewportRelativePosition(mousePos, NULL),
		buttonID, arg, activeTextureObject
	);
}

bool MGE::InputSystem::mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (MGE::InteractiveTextureManager::mouseReleased(mousePos, buttonID, arg, _activeTextureObject, MGE::InteractiveTexture::OnOgreObject))
		return true;
	
	mouseReleasedListeners.callFirst(&MGE::InputSystem::Listener::mouseReleased, mousePos, buttonID, arg, _activeTextureObject);
	
	return true;
}

bool MGE::InputSystem::keyPressed( const OIS::KeyEvent& arg ) {
	hightPriorityKeyPressedListener.callFirst(&KeyPressedListenerFunctor::call, arg);
	
	if (MGE::InteractiveTextureManager::keyPressed(arg, activeTextureObject, MGE::InteractiveTexture::OnGUIWindow))
		return true;
	
	if (inputAggregator->injectKeyDown( arg ))
		return true;
	
	return keyPressed(arg, activeTextureObject);
}

bool MGE::InputSystem::keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (MGE::InteractiveTextureManager::keyPressed(arg, _activeTextureObject, MGE::InteractiveTexture::OnOgreObject))
		return true;
	
	keyPressedListeners.callFirst(&MGE::InputSystem::Listener::keyPressed, arg, _activeTextureObject);
	return true;
}

bool MGE::InputSystem::keyReleased( const OIS::KeyEvent& arg ) {
	if (MGE::InteractiveTextureManager::keyReleased(arg, activeTextureObject, MGE::InteractiveTexture::OnGUIWindow))
		return true;
	
	if (inputAggregator->injectKeyUp( arg ))
		return true;
	
	return keyReleased(arg,  activeTextureObject);
}

bool MGE::InputSystem::keyReleased( const OIS::KeyEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) {
	if (MGE::InteractiveTextureManager::keyReleased(arg, _activeTextureObject, MGE::InteractiveTexture::OnOgreObject))
		return true;
	
	keyReleasedListeners.callFirst(&MGE::InputSystem::Listener::keyReleased, arg, _activeTextureObject);
	
	return true;
}

void MGE::InputSystem::lostInput(bool full) {
	lostInputListeners.callAll(&MGE::InputSystem::Listener::lostInput, full);
}


void MGE::InputSystem::setInputAggregator(InputAggregatorBase *aggregator) {
	delete inputAggregator;
	inputAggregator = aggregator;
}

#ifdef MGE_DEBUG_MOUSE_VIEWPORT_POSITION
#define DEBUG_MOUSE_VIEWPORT_POSITION_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG_MOUSE_VIEWPORT_POSITION_LOG(a)
#endif

Ogre::Vector2 MGE::InputSystem::InputAggregatorBase::getMousePosition(const OIS::MouseEvent& arg) {
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("getMousePosition " << arg.state.X.abs << " " << arg.state.Y.abs);
	return Ogre::Vector2(arg.state.X.abs, arg.state.Y.abs);
}

Ogre::Vector2 MGE::InputSystem::InputAggregatorBase::calcViewportRelativePosition(Ogre::Vector2 position, const CEGUI::Window* window) {
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("mouse screen position: " << position << " window: " << window);
	
	const OIS::MouseState& ms = MGE::InputSystem::getPtr()->getMouseState();
	position.x = position.x / ms.width;
	position.y = position.y / ms.height;
	
	DEBUG_MOUSE_VIEWPORT_POSITION_LOG("mouse screen relative position: " << position);
	
	return position;
}
