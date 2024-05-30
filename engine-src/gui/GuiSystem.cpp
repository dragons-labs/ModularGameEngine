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

#include "gui/GuiSystem.h"

#include "ConfigParser.h"

#include "gui/GuiOnTexture.h"
#include "gui/utils/CeguiClipboard.h"
#include "gui/InputAggregator4CEGUI.h"
#include "gui/utils/CeguiString.h"

#include "input/InputSystem.h"
#include "rendering/RenderingSystem.h"
#include "data/property/G11n.h"

#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <CEGUI/RendererModules/Ogre/WindowTarget.h>

MGE::GUISystem::~GUISystem(void) {
	LOG_INFO("Destroy GUISystem");
	
	Ogre::Root::getSingletonPtr()->removeFrameListener(this);
	
	delete clipboardProvider;
	
	CEGUI::WindowManager::getSingleton().destroyWindow(gMainWindow);
	gRenderer->destroySystem();
	delete ceguiLogger;
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_GUISystem \<GUISystem\>

@c \<GUISystem\> is used for used for configure <b>GUI system (CEGUI)</b>, have following (optional) subnodes:
	- @c \<SchemeName\> - name of CEGUI Scheme to use (default "MGE_GUI")
	- @c \<ImagesetName\> - name of CEGUI ImageSet to use for default cursor (default "MGE_GUI_ImageSet")
	- @c \<DefaultFont\> - name of default font (default "DefaultFont")
	- @c \<SchemeResourceGroup\> - name of ResourceGroup for Schemes (default "MGE_GUI_layouts")
	- @c \<ImagesetResourceGroup\> - name of ResourceGroup for ImageSets (default "MGE_GUI_layouts")
	- @c \<WidgetLookResourceGroup\> - name of ResourceGroup for WidgetLooks (default "MGE_GUI_layouts")
	- @c \<WindowResourceGroup\> - name of ResourceGroup for Windows (default "MGE_GUI_layouts")
	- @c \<FontResourceGroup\> - name of ResourceGroup for Fonts (default "MGE_GUI_fonts")
*/

MGE::GUISystem::GUISystem (const pugi::xml_node& xmlNode) {
	LOG_HEADER("Initialise CEGUI system");
	ceguiLogger = new MyCeguiLogger();
	
	LOG_INFO("Init Ogre render system in CEGUI");
	gRenderer = &CEGUI::OgreRenderer::bootstrapSystem( MGE::RenderingSystem::getPtr()->getRenderWindow() );
	gRenderer->setRenderingMode(CEGUI::OgreRenderer::RenderingModes::Disabled); // we use own frameRenderingQueued()
	Ogre::Root::getSingletonPtr()->addFrameListener(this);
	
	LOG_INFO("Configure resource group");
	CEGUI::Font::setDefaultResourceGroup(
		xmlNode.child("FontResourceGroup").text().as_string("MGE_GUI_fonts")
	);
	CEGUI::Scheme::setDefaultResourceGroup(
		xmlNode.child("SchemeResourceGroup").text().as_string("MGE_GUI_layouts")
	);
	CEGUI::ImageManager::setImagesetDefaultResourceGroup(
		xmlNode.child("ImagesetResourceGroup").text().as_string("MGE_GUI_layouts")
	);
	CEGUI::WidgetLookManager::setDefaultResourceGroup(
		xmlNode.child("WidgetLookResourceGroup").text().as_string("MGE_GUI_layouts")
	);
	CEGUI::WindowManager::setDefaultResourceGroup(
		xmlNode.child("WindowResourceGroup").text().as_string("MGE_GUI_layouts")
	);
	
	LOG_INFO("Create default GUIContext");
	gMainContext = &CEGUI::System::getSingleton().createGUIContext(gRenderer->getDefaultRenderTarget());
	
	LOG_INFO("Create input aggregator for default GUIContext");
	auto inputAggregator = new MGE::InputAggregator4CEGUI(gMainContext);
	MGE::InputSystem::getPtr()->setInputAggregator(inputAggregator);
	
	LOG_INFO("Setting default input semantics for default GUIContext");
	gMainContext->initDefaultInputSemantics();
	
	LOG_INFO("Setting scheme, cursor and font");
	CEGUI::SchemeManager::getSingleton().createFromFile(
		xmlNode.child("SchemeName").text().as_string("MGE_GUI") + ".scheme"sv
	);
	gMainContext->setDefaultCursorImage(
		xmlNode.child("ImagesetName").text().as_string("MGE_GUI_ImageSet") + "/MouseArrow"sv
	);
	gMainContext->setDefaultFont(
		xmlNode.child("DefaultFont").text().as_string("DefaultFont")
	);
	CEGUI::System::getSingleton().setDefaultFontName(
		xmlNode.child("DefaultFont").text().as_string("DefaultFont")
	);
	
	LOG_INFO("Create main window");
	gMainWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Sheet");
	gMainWindow->setCursorPassThroughEnabled(true);
	gMainContext->setRootWindow(gMainWindow);
	gMainContext->setDefaultTooltipType("Tooltip");
	
	LOG_INFO("Fix mouse position");
	const OIS::MouseState state = MGE::InputSystem::getPtr()->getMouseState();
	gMainContext->injectMousePosition(state.X.abs, state.X.abs);
	setMouseVisible(true);
	
	LOG_INFO("Init clipboard support");
	if (MGE::CeguiNativeClipboard::supported()) {
		clipboardProvider = new MGE::CeguiNativeClipboard( MGE::RenderingSystem::getPtr()->getRenderWindow() );
		CEGUI::System::getSingleton().getClipboard()->setNativeProvider( clipboardProvider );
	} else {
		clipboardProvider = NULL;
	}
}

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(GUISystem) {
	return new MGE::GUISystem(xmlNode);
}


bool MGE::GUISystem::frameRenderingQueued(const Ogre::FrameEvent& evt) {
	CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
	
	gRenderer->beginRendering();
	
	gMainContext->injectTimePulse(evt.timeSinceLastFrame);
	gMainContext->draw();
	
	gRenderer->endRendering();
	
	for (auto& iter : gExtraContexts) {
		iter->getContext()->injectTimePulse(evt.timeSinceLastFrame);
		iter->redraw();
	}
	
	CEGUI::WindowManager::getSingleton().cleanDeadPool();
	return true;
}

CEGUI::Window* MGE::GUISystem::findGUIWindow(const Ogre::Vector2& position, CEGUI::Window* parent)  {
	CEGUI::Window* findWindow = parent->getChildAtPosition(
		glm::vec2(position.x, position.y)
	);
	if (findWindow) {
		LOG_DEBUG("window" <<
			" path: "      << findWindow->getNamePath() <<
			" name: "      << findWindow->getName() <<
			" type: "      << findWindow->getType() <<
			" left_up.x: " << findWindow->getClipRect().getPosition().x <<
			" left_up.y: " << findWindow->getClipRect().getPosition().y <<
			" width: "     << findWindow->getInnerRectClipper().getWidth() <<
			" height: "    << findWindow->getInnerRectClipper().getHeight()
		);
	}
	return findWindow;
}

CEGUI::Window* MGE::GUISystem::createGUIWindow(const CEGUI::String& layout, const std::string_view& moduleName, CEGUI::Window* parent) {
	LOG_INFO("Create window for " + moduleName + " based on: " + layout);
	
	CEGUI::Window* win = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(layout);
	if (parent == NULL) {
		parent  = gMainWindow;
	}
	parent->addChild(win);
	
	return win;
}

void MGE::GUISystem::setTranslatedText(CEGUI::Window* win, const CEGUI::String& altStr, const std::string_view& prefix) {
	try {
		win->setText(
			win->getUserString( CEGUI::String(prefix + MGE::G11n::getLang()) )
		);
	} catch (CEGUI::UnknownObjectException) {
		win->setText( altStr );
	}
}

void MGE::GUISystem::setMouseVisible(bool vis) {
	gMainContext->setCursorVisible(vis);
}

void MGE::GUISystem::registerContext(MGE::GUIOnTexture* gc) {
	gExtraContexts.insert(gc);
}

void MGE::GUISystem::unregisterContext(MGE::GUIOnTexture* gc) {
	gExtraContexts.erase(gc);
}


MGE::GUISystem::MyCeguiLogger::MyCeguiLogger() {
	LOG_INFO("Create CEGUI logger");
}

void MGE::GUISystem::MyCeguiLogger::logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level) {
	MGE::Log::LogLevel newLevel;
	switch(level) {
		case CEGUI::LoggingLevel::Error:
			newLevel = MGE::Log::Error;
			break;
		case CEGUI::LoggingLevel::Warning:
			newLevel = MGE::Log::Warning;
			break;
		case CEGUI::LoggingLevel::Standard:
			newLevel = MGE::Log::Info;
			break;
		case CEGUI::LoggingLevel::Informative:
			newLevel = MGE::Log::Verbose;
			break;
		case CEGUI::LoggingLevel::Insane:
			newLevel = MGE::Log::Debug;
			break;
	}
	MGE_LOG.logMultiLine(STRING_FROM_CEGUI(message), newLevel, "CEGUI");
}
