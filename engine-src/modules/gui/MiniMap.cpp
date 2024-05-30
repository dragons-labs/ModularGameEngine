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

#include "modules/gui/MiniMap.h"

#include "LogSystem.h"
#include "ConfigParser.h"
#include "XmlUtils.h"
#include "data/property/XmlUtils_Ogre.h"

#include "rendering/CameraSystem.h"
#include "gui/InputAggregator4CEGUI.h"

#include <CEGUI/RendererModules/Ogre/Texture.h>

const int MGE::MiniMap::overlayScale = 1;
// theoretically we can use a smaller texture (minimap size divided by overlayScale) and use a smaller value of size
// parametr in putPoint, but due to interpolation method of textures used in CEGUI (and no methods to change it for single
// texture?) we don't do this

MGE::MiniMap::MiniMap(
	MGE::GenericWindows::BaseWindow* baseWin,
	const CEGUI::String& image, const CEGUI::String& imageGroup,
	const Ogre::Vector2& upperLeftCornerPositionIn3D,
	const Ogre::Vector2& sizeIn3D
) :
	MGE::GenericWindows::BaseWindowOwner(baseWin),
	MGE::Unloadable(200)
{
	LOG_INFO("Initialise GUIMiniMap based on: " + image + " from: " + imageGroup + " resources group");
	
	objectsInfoProvider     = nullptr;
	
	// background minimap
	minimap = getWindow()->getChild("MiniMap");
	CEGUI::ImageManager::getSingleton().addBitmapImageFromFile( "BackgroundMiniMap", image, imageGroup );
	
	miniMapSizeIn3D         = sizeIn3D;
	miniMapOffsetX          = upperLeftCornerPositionIn3D.x;
	miniMapOffsetY          = upperLeftCornerPositionIn3D.y;
	minimap->subscribeEvent(
		CEGUI::Window::EventSized, CEGUI::Event::Subscriber(&MGE::MiniMap::handleSized, this)
	);
	recalculateScale();
	
	LOG_INFO("miniMapSizeIn3D=" << miniMapSizeIn3D << " miniMapOffset=" << upperLeftCornerPositionIn3D << " scaleX=" << toOverlayMiniMapScaleX << " scaleY=" << toOverlayMiniMapScaleY);
	
	minimap->setProperty("Image", "BackgroundMiniMap");
	minimap->subscribeEvent(
		CEGUI::Window::EventClick,
		CEGUI::Event::Subscriber(&MGE::MiniMap::handleClick, this)
	);
	
	// overlay minimap
	minimap = minimap->getChild("OverlayMap");
	overlayTexture = &(MGE::GUISystem::getPtr()->getRenderer()->createTexture("OverlayMniMap"));
	
	overlayTextureSize.d_width  = minimap->getPixelSize().d_width  / overlayScale;
	overlayTextureSize.d_height = minimap->getPixelSize().d_height / overlayScale;
	overlayTextureBufferSize    = overlayTextureSize.d_width * overlayTextureSize.d_height;
	
	CEGUI::BitmapImage& image2 = static_cast<CEGUI::BitmapImage&>(CEGUI::ImageManager::getSingleton().create("BitmapImage", "OverlayMniMap"));
	image2.setTexture(overlayTexture);
	image2.setImageArea( CEGUI::Rectf(
		0.0f,
		0.0f,
		overlayTextureSize.d_width,
		overlayTextureSize.d_height
	) );
	image2.setAutoScaled( CEGUI::AutoScaledMode::Both );
	
	uint16_t overlayTextureBuffer[overlayTextureBufferSize];
	for (int i=0; i<overlayTextureBufferSize; ++i) overlayTextureBuffer[i] = 0x0000; // ARGB
	overlayTexture->loadFromMemory(
		overlayTextureBuffer, overlayTextureSize,
		CEGUI::Texture::PixelFormat::Rgba4444
	);
	
	minimap->setProperty("Image", "OverlayMniMap");
	
	isVisible = false;
	getWindow()->getChild("MiniMap")->subscribeEvent(
		CEGUI::Window::EventShown, CEGUI::Event::Subscriber(&MGE::MiniMap::onShow, this)
	);
	getWindow()->getChild("MiniMap")->subscribeEvent(
		CEGUI::Window::EventHidden, CEGUI::Event::Subscriber(&MGE::MiniMap::onHide, this)
	);
	
	MGE::Engine::getPtr()->mainLoopListeners.addListener(this, POST_RENDER_GUI);
}

MGE::MiniMap::~MiniMap() {
	LOG_INFO("destroy MiniMap");
	
	CEGUI::ImageManager::getSingleton().destroy("OverlayMniMap");
	MGE::GUISystem::getPtr()->getRenderer()->destroyTexture("OverlayMniMap");
	CEGUI::ImageManager::getSingleton().destroy("BackgroundMiniMap");
	MGE::GUISystem::getPtr()->getRenderer()->destroyTexture("BackgroundMiniMap");
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
	
	// window->remClient() is in (automatic called) BaseWindowOwner destructor ... and can destroy baseWin too
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_MiniMap \<MiniMap\>

@c \<MiniMap\> is used for enabled and configure GUI (sub)widow with mini map of action scene. It have required subnodes:
	- @ref XMLNode_BaseWin
	- @c \<File\> with attributes:
		- @c name - minimap image filename
		- @c group - minimap image resources group
		.
		and subnodes:
		- @c WorldPosition_of_UpperLeftCorner - @ref XML_Vector2 with 3D world X,Z coordinate of left upper corner of minimap image
		- @c WorldSize - @ref XML_Vector2 with size of mini map (offset from left upper corner to rigth lower corner of minimap image) in game 3D world units
	.
see too: @ref MGE::MiniMap::MiniMap
*/

MGE::MiniMap* MGE::MiniMap::create(const pugi::xml_node& xmlNode) {
	LOG_INFO("Load / create MiniMap based on config xml node");
	
	MGE::GenericWindows::BaseWindow* baseWin = MGE::GenericWindows::Factory::getPtr()->get(xmlNode);
	if(!baseWin) {
		throw std::logic_error("Could not create base window for MiniMap");
	}
	
	auto xmlSubNode = xmlNode.child("File");
	if (xmlSubNode) {
		return new MGE::MiniMap(
			baseWin,
			xmlSubNode.attribute("name").as_string(),
			xmlSubNode.attribute("group").as_string("Map_Scene"),
			MGE::XMLUtils::getValue(xmlNode.child("WorldPosition_of_UpperLeftCorner"), Ogre::Vector2::ZERO),
			MGE::XMLUtils::getValue(xmlNode.child("WorldSize"), Ogre::Vector2::UNIT_SCALE)
		);
	} else {
		throw std::logic_error("No correct config for MiniMap");
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(MiniMap) {
	return MGE::MiniMap::create(xmlNode);
}

bool MGE::MiniMap::handleSized(const CEGUI::EventArgs& args) {
	recalculateScale();
	return true;
}

bool MGE::MiniMap::onShow(const CEGUI::EventArgs& args) {
	isVisible = true;
	return true;
}

bool MGE::MiniMap::onHide(const CEGUI::EventArgs& args) {
	isVisible = false;
	return true;
}

void MGE::MiniMap::show(const CEGUI::String& name) {
	if (name.empty())
		window->show("MiniMap");
	else
		window->show(name);
}

void MGE::MiniMap::recalculateScale() {
	toOverlayMiniMapScaleX  = minimap->getPixelSize().d_width  / miniMapSizeIn3D.x;
	toOverlayMiniMapScaleY  = minimap->getPixelSize().d_height / miniMapSizeIn3D.y;
	fromMiniMapScaleX       = miniMapSizeIn3D.x / minimap->getPixelSize().d_width;
	fromMiniMapScaleY       = miniMapSizeIn3D.y / minimap->getPixelSize().d_height;
}

Ogre::Vector3 MGE::MiniMap::minimapToWorld(const glm::vec2& pos) {
	return Ogre::Vector3(
		miniMapOffsetX + pos.x * fromMiniMapScaleX,
		0,
		miniMapOffsetY + pos.y * fromMiniMapScaleY
	);
}

Ogre::Vector2 MGE::MiniMap::worldToOverlayMinimap(const Ogre::Vector3& pos) {
	return Ogre::Vector2(
		(pos.x - miniMapOffsetX) * toOverlayMiniMapScaleX,
		(pos.z - miniMapOffsetY) * toOverlayMiniMapScaleY
	);
}

void MGE::MiniMap::putPoint(uint16_t* buf, int bufWidth, int bufHeight, int x, int y, uint8_t size, uint16_t argb_color) {
	x = x - size/2;
	y = y - size/2;
	
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x+size >= bufWidth)  x = bufWidth - size;
	if (y+size >= bufHeight) y = bufHeight - size;
	
	for (int i=x; i<x+size; ++i)
		for (int j=y; j<y+size; ++j)
			buf[i + bufWidth * j] = argb_color;
}

void MGE::MiniMap::putCross(uint16_t* buf, int bufWidth, int bufHeight, int x, int y, uint8_t size, uint16_t argb_color) {
	if (x < size) x = size;
	if (y < size) y = size;
	if (x+size >= bufWidth)  x = bufWidth - size - 1;
	if (y+size >= bufHeight) y = bufHeight - size - 1;
	
	for (int i=x-size; i<=x+size; ++i)
		buf[i + bufWidth * y] = argb_color;
	for (int i=y-size; i<=y+size; ++i)
		buf[x + bufWidth * i] = argb_color;
}

bool MGE::MiniMap::update(float gameTimeStep, float realTimeStep) {
	if (!isVisible)
		return false;
	
	if (!objectsInfoProvider){
		LOG_ERROR("Using MiniMap without set objectsInfoProvider");
		return false;
	}
	
	// init minimap overlay buffer
	uint16_t overlayTextureBuffer[overlayTextureBufferSize];
	for (int i=0; i<overlayTextureBufferSize; ++i) overlayTextureBuffer[i] = 0x0000;
	
	// puts information to overlay buffer
	const uint16_t* buf = NULL;
	int             width = 0, height = 0;
	Ogre::Vector3   worldPosition;
	
	objectsInfoProvider->resetMinimapInfo();
	while (
		// 1. get object symbol and world position
		objectsInfoProvider->getNextMinimapInfo(buf, width, height, worldPosition)
	) {
		if (buf == NULL)
			continue;
		
		// 2. get actor position at minimap
		Ogre::Vector2 mmPos = worldToOverlayMinimap( worldPosition );
		
		// 3. calculate position left upper corner of actor symbol
		mmPos.x -= width/2.0;
		mmPos.y -= height/2.0;
		
		// 4. check / correct position left upper corner of actor symbol
		if (mmPos.y < 0)
			 mmPos.y = 0;
		else if (mmPos.y + height >= overlayTextureSize.d_height)
			mmPos.y = overlayTextureSize.d_height - height - 1;
		
		if (mmPos.x < 0)
			 mmPos.x = 0;
		else if (mmPos.x + width >= overlayTextureSize.d_width)
			mmPos.x = overlayTextureSize.d_width - width - 1;
		
		/// copy symbol to minimap
		uint16_t* start = overlayTextureBuffer + static_cast<int>(mmPos.y) * static_cast<int>(overlayTextureSize.d_width) + static_cast<int>(mmPos.x);
		
		for (int line=0; line<height; ++line) {
			memcpy(
				start + line * static_cast<int>(overlayTextureSize.d_width),
				buf   + line * width,
				height * 2
			);
		}
	}
	
	// debug:
	// putPoint(overlayTextureBuffer, overlayTextureSize.d_width, overlayTextureSize.d_height, 22, 22, 7, 0x9f93);
	
	// update texture from overlay buffer
	overlayTexture->loadFromMemory(
		overlayTextureBuffer, overlayTextureSize,
		CEGUI::Texture::PixelFormat::Rgba4444
	);
	minimap->invalidate();
	
	return true;
}

bool MGE::MiniMap::handleClick(const CEGUI::EventArgs& args) {
	auto mbargs = static_cast<const CEGUI::MouseButtonEventArgs&>(args);
	
	if (mbargs.d_button == CEGUI::MouseButton::Left) {
		LOG_INFO("MiniMap: center camera on click");
		Ogre::Vector3  worldPos = minimapToWorld(
			CEGUI::CoordConverter::screenToWindow(*(mbargs.window), mbargs.window->getGUIContextPtr()->getCursorPosition())
		);
		MGE::CameraSystem::getPtr()->getCurrentCamera()->setPosition(worldPos);
		
		return true;
	}
	
	return false;
}
