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

#include "gui/GuiOnTexture.h"

#include "LogSystem.h"
#include "gui/GuiSystem.h"
#include "gui/InputAggregator4CEGUI.h"

#include <CEGUI/RendererModules/Ogre/Texture.h>
#include <CEGUI/RendererModules/Ogre/TextureTarget.h>

#include <Hlms/Unlit/OgreHlmsUnlitDatablock.h>

MGE::GUIOnTexture::GUIOnTexture(
	const std::string_view& _objectName, int _xSize, int _ySize, Ogre::SceneManager* _scnMgr, bool _isInteractive, bool _isNotMovable, Ogre::MovableObject* _ogreObject
) :
	MGE::InteractiveTexture("3DGUI",  _objectName, OnOgreObject, _scnMgr, _isNotMovable, false, _ogreObject), guiContext(NULL)
{
	// create render target and texture
	renderTextureTarget = MGE::GUISystem::getPtr()->getRenderer()->createTextureTarget(
		false
	);
	renderTextureTarget->declareRenderSize( CEGUI::Sizef(_xSize, _ySize) );
	renderTexture = static_cast<CEGUI::OgreTexture&>(renderTextureTarget->getTexture()).getOgreTexture();
	
	// create, set on object and configure (transparency) Ogre material
	createMaterialOnOgreObject(_isInteractive);
	
	// register listener for interactive mode
	if (_isInteractive) {
		MGE::InteractiveTextureManager::getPtr()->addTextureListener(getObjectName(), this);
		// remove in ~MGE::InteractiveTexture();
	}
	
	// create gui context and root window
	guiContext = &(CEGUI::System::getSingleton().createGUIContext(*renderTextureTarget));
	rootWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Sheet");
	guiContext->setRootWindow(rootWindow);
	
	// register gui context for drawing
	MGE::GUISystem::getPtr()->registerContext(this);
}

void MGE::GUIOnTexture::redraw() {
	if (guiContext->isDirty()) {
		LOG_DEBUG("GUIOnTexture::redraw");
		MGE::GUISystem::getPtr()->getRenderer()->beginRendering();
		renderTextureTarget->clear();
		guiContext->draw();
		MGE::GUISystem::getPtr()->getRenderer()->endRendering();
	}
}

MGE::GUIOnTexture::~GUIOnTexture() {
	if (guiContext) {
		MGE::GUISystem::getPtr()->unregisterContext(this);
		CEGUI::WindowManager::getSingleton().destroyWindow(rootWindow);
		CEGUI::System::getSingleton().destroyGUIContext(*guiContext);
	}
}

bool MGE::GUIOnTexture::mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	const CEGUI::Sizef& size = guiContext->getSurfaceSize();
	Ogre::Vector2 mousePosition(mouseTexturePos.x * size.d_width, mouseTexturePos.y * size.d_height);
	
	guiContext->injectMousePosition( mousePosition.x, mousePosition.y );
	guiContext->markAsDirty();
	return guiContext->injectMouseButtonDown( MGE::InputAggregator4CEGUI::convertButton( buttonID ) );
}

bool MGE::GUIOnTexture::mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) {
	bool ret = false;
	std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	if(res.first) {
		const CEGUI::Sizef& size = guiContext->getSurfaceSize();
		ret = guiContext->injectMousePosition( res.second.x * size.d_width, res.second.y * size.d_height );
		guiContext->markAsDirty();
	}
	return ret || guiContext->injectMouseWheelChange( arg.state.Z.rel );
}

bool MGE::GUIOnTexture::mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	guiContext->markAsDirty();
	return guiContext->injectMouseButtonUp( MGE::InputAggregator4CEGUI::convertButton( buttonID ) );
}

bool MGE::GUIOnTexture::keyPressed(const OIS::KeyEvent& arg) {
	guiContext->markAsDirty();
	bool r1 = guiContext->injectKeyDown( MGE::InputAggregator4CEGUI::convertKey(arg.key) );
	bool r2 = guiContext->injectChar(arg.text); // when OIS TextTranslation == Unicode (default) this is Unicode char number,
	                                            // conversion from UTF-8 (linux) or UTF-16 (windows) is done internally by OIS
	return r1 || r2;
}

bool MGE::GUIOnTexture::keyReleased(const OIS::KeyEvent& arg) {
	guiContext->markAsDirty();
	return guiContext->injectKeyUp( MGE::InputAggregator4CEGUI::convertKey(arg.key) );
}
