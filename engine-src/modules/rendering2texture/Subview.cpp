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

#include "modules/rendering2texture/Subview.h"

#include "gui/GuiSystem.h"
#include "input/InputSystem.h"
#include "rendering/CameraSystem.h"
#include "data/utils/OgreSceneObjectInfo.h"

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorChannel.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <OgreViewport.h>

MGE::SubView::SubView (
	const std::string_view& _objectName,
	const pugi::xml_node& _cameraCfg,
	int _xSize, int _ySize,
	MGE::InteractiveTexture::Mode _mode,
	Ogre::SceneManager* _scnMgr, bool _isInteractive, bool _isNotMovable, Ogre::MovableObject* _ogreObject,
	const MGE::LoadingContext* _context
) : 
	MGE::InteractiveTexture("SubView", _objectName, _mode, _scnMgr, _isNotMovable, false, _ogreObject),
	MGE::Unloadable(200)
{
	// create (sub)camera
	camera = new MGE::CameraNode(getCameraName(), _scnMgr);
	camera->restoreFromXML(_cameraCfg, _context);
	
	// create Ogre texture
	renderTexture = createTexture(_xSize, _ySize, _isInteractive, Ogre::TextureFlags::RenderToTexture);
	
	// add viewport with (sub)camera to texture render target and configure it
	camera->setRenderTarget( renderTexture, MGE::VisibilityFlags::DEFAULT_MASK );
	
	hasInput = false;
	hasMouseHover = false;
}

MGE::SubView::SubView (
	const std::string_view& _objectName,
	MGE::CameraNode* _camera,
	int _xSize, int _ySize,
	MGE::InteractiveTexture::Mode _mode,
	Ogre::SceneManager* _scnMgr, bool _isInteractive, bool _isNotMovable, Ogre::MovableObject* _ogreObject
) : 
	MGE::InteractiveTexture("SubView", _objectName, _mode, _scnMgr, _isNotMovable, false, _ogreObject),
	MGE::Unloadable(200)
{
	// set (sub)camera
	camera = _camera;
	
	// create Ogre texture
	renderTexture = createTexture(_xSize, _ySize, _isInteractive, Ogre::TextureFlags::RenderToTexture);
	
	// add viewport with (sub)camera to texture render target and configure it
	camera->setRenderTarget( renderTexture, MGE::VisibilityFlags::DEFAULT_MASK );
	
	hasInput = false;
	hasMouseHover = false;
}

MGE::SubView::~SubView () {
	LOG_INFO("destroy SubView");
	delete camera;
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_SubView \<SubView\>

@c \<SubView\> is used for creating sub-view camera with rendering to Ogre texture or CEGUI window.

Attributes independent of mode (Ogre vs CEGUI):
	- @c resX - x (horizontal) resolution of render target texture
	- @c resY - y (vertical) resolution of render target texture
	.
Attributes for CEGUI texture and window target:
	- @c windowName - set base name for @ref MGE::InteractiveTexture,
	  used as part of texture and CEGUI image name
	- @c putOnWindow - name of CEGUI window to set its "Image" property to image with subview texture
	.
Attributes for Ogre texture and 3D world object target:
	- @c nodeName   - set base name for @ref MGE::InteractiveTexture,
	  used as part of texture and Ogre material/datablock name,
	  used ALSO as name of Ogre::MovableObject and its parent Ogre::SceneNode (to find 3D object to put texture on it)

@c \<SubView\> contain one subnode @c \<Camera\> using @ref XMLNode_Camera syntax.

@subsubsection XMLNode_SubView_Example Example
@code{.xml}
<SubView windowName="OgreTest" putOnWindow="WorldInfoWindow/OgreTest" resX="512" resY="512">
	<Camera>
		<Mode>
			<RotationAllowed>1</RotationAllowed>
			<MoveAllowed>1</MoveAllowed>
			<LookOutside>0</LookOutside>
		</Mode>
		<Place>
			<Position> <x>-11.5983</x> <y>0</y> <z>3.52944</z> </Position> 
			<Orientation> <w>1</w> <x>0</x> <y>0</y> <z>0</z> </Orientation>
			<Pitch> <rad>0.785</rad> </Pitch>
		</Place>
	</Camera>
</SubView>
@endcode <br/>
*/

MGE::SubView* MGE::SubView::create(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	int resX = xmlNode.attribute("resX").as_int();
	int resY = xmlNode.attribute("resY").as_int();
	
	auto cameraNode = xmlNode.child("Camera");
	if (!cameraNode) {
		LOG_WARNING("No camera configuration for SubView");
		return nullptr;
	}
	
	pugi::xml_attribute xmlAttrib;
	
	MGE::InteractiveTexture::Mode mode;
	if ( (xmlAttrib = xmlNode.attribute("windowName")) ) {
		mode = MGE::InteractiveTexture::OnGUIWindow;
	} else if ( (xmlAttrib = xmlNode.attribute("nodeName")) ) {
		mode = MGE::InteractiveTexture::OnOgreObject;
	} else {
		LOG_WARNING("Can't determined MGE::InteractiveTexture::Mode for SubView");
		return nullptr;
	}
	
	auto& allCameraNodes = MGE::CameraSystem::getPtr()->allCameraNodes;
	auto name = xmlAttrib.as_string();
	auto camera = allCameraNodes.find( "SubViewCamera"sv + name );
	
	SubView* subView = nullptr;
	if (camera != allCameraNodes.end()) {
		LOG_INFO("Camera for this SubView (" << name << ") exists ... reusing it (this is normal while loading from save)");
		subView = new MGE::SubView( name, camera->second, resX, resY, mode, context->scnMgr );
	} else {
		subView = new MGE::SubView( name, cameraNode, resX, resY, mode, context->scnMgr );
	}
	
	if (mode == MGE::InteractiveTexture::OnGUIWindow)
		subView->putOnGUIWindow( xmlNode.attribute("putOnWindow").as_string() );
	
	return subView;
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(SubView) {
	return MGE::SubView::create(xmlNode, context);
}

bool MGE::SubView::mousePressed(const Ogre::Vector2& mouseTexturePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	LOG_DEBUG("mousePressed on SubView");
	if (!hasInput) {
		hasInput = true;
		activeTextureObject = NULL;
		MGE::CameraSystem::getPtr()->setCurrentCamera(camera);
	}
	hasMouseHover = true;
	MGE::InputSystem::getPtr()->mousePressed(mouseTexturePos, buttonID, arg, activeTextureObject, clickWindow);
	return true;
}

bool MGE::SubView::mouseMoved(const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg) {
	std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	if (res.first) {
		hasMouseHover = true;
		return MGE::InputSystem::getPtr()->mouseMoved(res.second, arg, activeTextureObject);
	} else if (hasMouseHover) {
		MGE::InputSystem::getPtr()->lostInput();
		hasMouseHover = false;
	}
	return false;
}

bool MGE::SubView::mouseReleased(const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg) {
	std::pair<bool, Ogre::Vector2> res = textureHitTest(mousePos);
	if (res.first && MGE::InputSystem::getPtr()->mouseReleased(res.second, buttonID, arg, activeTextureObject))
		return true;
	return false;
}

bool MGE::SubView::keyPressed(const OIS::KeyEvent& arg) {
	return MGE::InputSystem::getPtr()->keyPressed(arg, activeTextureObject);
}

bool MGE::SubView::keyReleased(const OIS::KeyEvent& arg) {
	return MGE::InputSystem::getPtr()->keyReleased(arg, activeTextureObject);
}

bool MGE::SubView::lostInput(MGE::InteractiveTexture* toTexture, bool toGUI) {
	if (!toTexture) {
		MGE::CameraSystem::getPtr()->setCurrentCamera(NULL);
		MGE::InputSystem::getPtr()->lostInput(true);
		hasInput = false;
	}
	return !hasInput;
}

inline std::string MGE::SubView::getCameraName() const {
	return namePrefix + "Camera" + objectName;
}

MGE::CameraNode* MGE::SubView::getCamera() const {
	return camera;
}
