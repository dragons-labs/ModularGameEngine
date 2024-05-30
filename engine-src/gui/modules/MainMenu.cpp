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

#include "gui/modules/MainMenu.h"

#include "with.h"
#include "ConfigParser.h"

#include "gui/GuiSystem.h"
#include "gui/utils/CeguiString.h"

#include "data/property/G11n.h"
#include "gui/utils/CeguiOpenFileDialog.h"

#include "rendering/RenderingSystem.h" // rendering single frame on loading screen mode
#include "physics/TimeSystem.h"    // pause managment
#include "rendering/audio-video/AudioSystem.h"     // pause audio on full pause
#include "data/LoadingSystem.h"   // loading maps / saves, saving, ...

#include <OgreResourceGroupManager.h>
#ifdef USE_OGGVIDEO
#include <OgreVideoManager.h>
#endif

MGE::MainMenu::MainMenu() {
	LOG_INFO("Initialise GUIMainMenu");
	
	menuWin = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("MainMenu.layout");
	MGE::GUISystem::getPtr()->getMainWindow()->addChild(menuWin);
	menuWin->hide();
	
	mainMenu       = menuWin->getChild( "MainMenu" );
	openFileDialog = new MGE::OpenFileDialog( menuWin->getChild( "OpenFileDialog" ) );
	loadMapMenu    = menuWin->getChild( "LoadMapMenu" );
	endGameMenu    = menuWin->getChild( "EndGameMenu" );
	loadingScreen  = menuWin->getChild( "LoadingScreen" );
	fullPaused     = false;
	
	mapsList = static_cast<CEGUI::ListWidget*>(loadMapMenu->getChild( "MapsList" ));
	mapsList->setSortMode(CEGUI::ViewSortMode::Ascending);
	mapsList->setMultiSelectEnabled(false);
	mapsList->subscribeEvent(
		CEGUI::ListWidget::EventSelectionChanged, CEGUI::Event::Subscriber(&MGE::MainMenu::mapsListSelectionChanged, this)
	);
	mapsList->setTextColour( CEGUI::PropertyHelper<CEGUI::ColourRect>::fromString(mapsList->getProperty("TextColour")).getColourAtPoint(0,0) );
	
	CEGUI::Window* subWin;
	CEGUI::Window* button;
	CEGUI::String  tmpString;
	for (size_t i = 0; i < menuWin->getChildCount(); ++i) {
		subWin = menuWin->getChildAtIndex( i );
		LOG_INFO("Parsing elements of: " + STRING_FROM_CEGUI(subWin->getName()));
		
		for (size_t j = 0; j < subWin->getChildCount(); ++j) {
			button = subWin->getChildAtIndex( j );
			LOG_INFO("parsing sub element: " + STRING_FROM_CEGUI(button->getName()));
			
			tmpString = button->getType();
			if (tmpString.substr(tmpString.find("/")+1) == "Button") {
				button->subscribeEvent(
					CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MGE::MainMenu::handleAction, this)
				);
				tmpString = button->getName();
				tmpString = tmpString.substr(tmpString.find(" : ")+3);
				MGE::GUISystem::getPtr()->setTranslatedText(button, tmpString);
			}
		}
	}
	
	MGE::InputSystem::getPtr()->hightPriorityKeyPressedListener.addListener(
		MGE::InputSystem::KeyPressedListenerFunctor(std::bind(&MGE::MainMenu::priorityKeyPressed, this, std::placeholders::_1), reinterpret_cast<uintptr_t>(this)),
		64
	);
	
	MGE::InputSystem::getPtr()->registerListener(
		this,
		-1, -1, -1,
		-1, MGE::InputSystem::Listener::CAMERA_CONTROL, -1
	);
	
	MGE::LoadingSystem::getPtr()->setLoadingScreen(this);
}

MGE::MainMenu::~MainMenu() {
	LOG_INFO("Destroy MainMenu");
	MGE::GUISystem::getPtr()->getMainWindow()->removeChild(menuWin);
	delete openFileDialog;
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_MainMenu \<MainMenu\>

@c \<MainMenu\> is used for setup <b>Main Menu</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(MainMenu) {
	return new MGE::MainMenu();
}


void MGE::MainMenu::setLoadingScreenImage(const std::string_view& imageName, const std::string_view& imageGroup) {
	if (!CEGUI::ImageManager::getSingleton().isDefined(STRING_TO_CEGUI(imageName))) {
		CEGUI::ImageManager::getSingleton().addBitmapImageFromFile(
			STRING_TO_CEGUI(imageName),
			STRING_TO_CEGUI(imageName),
			STRING_TO_CEGUI(imageGroup)
		);
	}
	loadingScreen->setProperty("Image", STRING_TO_CEGUI(imageName));
}

void MGE::MainMenu::setLoadingScreenProgress(float progress, const std::string_view& info) {
	static_cast<CEGUI::ProgressBar*>(loadingScreen->getChild("Progress"))->setProgress(progress);
	loadingScreen->getChild("Info")->setText(STRING_TO_CEGUI(info));
	MGE::RenderingSystem::getPtr()->renderOneFrame();
}

void MGE::MainMenu::showLoadingScreen() {
	show(LOADING_SCREEN);
}

void MGE::MainMenu::hideLoadingScreen() {
	hide(true);
}


void MGE::MainMenu::show(MenuMode mode, const std::string_view& title, const std::string_view& text, const std::string_view& argb_clor) {
	LOG_INFO("Main menu: showing");
	
	bool checkPause = !menuWin->isVisible();
	
	switch(mode) {
		case STANDARD:
			mainMenu->show();
			endGameMenu->hide();
			loadingScreen->hide();
			break;
		case END_GAME:
			endGameMenu->getChild( "Title" )->setText(STRING_TO_CEGUI(title));
			endGameMenu->getChild( "Text"  )->setText(STRING_TO_CEGUI(text));
			endGameMenu->getChild( "Title" )->setProperty("TextColour", STRING_TO_CEGUI(argb_clor));
			mainMenu->hide();
			endGameMenu->show();
			loadingScreen->hide();
			break;
		case LOADING_SCREEN:
			mainMenu->hide();
			endGameMenu->hide();
			loadingScreen->show();
			break;
		case SUB_DIALOG:
			LOG_WARNING("call show with mode == SUB_DIALOG");
			return;
	}
	
	if (checkPause) { // only on first show check pause status
		if (MGE::LoadingSystem::getPtr()->getSceneLoadState() == MGE::LoadingSystem::GAME) {
			setFullPause(true);
		} else {
			pausedOnShow = false;
		}
	}
	
	if (MGE::LoadingSystem::getPtr()->getSceneLoadState() == MGE::LoadingSystem::NO_SCENE || MGE::LoadingSystem::getPtr()->getSceneLoadState() == MGE::LoadingSystem::IN_PROGRESS) {
		menuWin->setProperty("BackgroundColours", menuWin->getUserString("BackgroundNoTransparent"));
		menuWin->setProperty("FrameColours",      menuWin->getUserString("BackgroundNoTransparent"));
		
		mainMenu->getChild( "MainMenu : Continue" )->setProperty("Disabled", "True");
	} else {
		menuWin->setProperty("BackgroundColours", menuWin->getUserString("BackgroundTransparent"));
		menuWin->setProperty("FrameColours",      menuWin->getUserString("BackgroundTransparent"));
		
		mainMenu->getChild( "MainMenu : Continue" )->setProperty("Disabled", "False");
	}
	
	loadMapMenu->hide();
	openFileDialog->hide();
	
	menuWin->show();
	menuWin->activate();
	
	currMode = mode;
	
	if (mode == LOADING_SCREEN) {
		MGE::RenderingSystem::getPtr()->renderOneFrame();
	}
}

void MGE::MainMenu::injectEsc() {
	if (!menuWin->isVisible()) {
		show();
		return;
	}
	
	switch(currMode) {
		case STANDARD:
			if ( !mainMenu->getChild( "MainMenu : Continue" )->isDisabled() )
				hide();
			break;
		case END_GAME:
			mainMenu->getChild( "MainMenu : Continue" )->setProperty("Disabled", "True");
			mainMenu->show();
			endGameMenu->hide();
			break;
		case LOADING_SCREEN:
			break;
		case SUB_DIALOG:
			loadMapMenu->hide();
			openFileDialog->hide();
			mainMenu->show();
			currMode = STANDARD;
			break;
	}
}

void MGE::MainMenu::hide(bool after_reload) {
	menuWin->hide();
	
	if (!after_reload) {
		setFullPause(false);
	}
	
	LOG_INFO("Main menu: hidden");
}

void MGE::MainMenu::toggleVisibility() {
	if (menuWin->isVisible()) {
		hide();
	} else {
		show();
	}
}

bool MGE::MainMenu::priorityKeyPressed(const OIS::KeyEvent& arg) {
	if (arg.key == OIS::KC_SCROLL && MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift)) { // ScrollLock + Shift  ==> show / hide main menu
		toggleVisibility();
		return true;
	}
	
	return false;
}

bool MGE::MainMenu::keyPressed( const OIS::KeyEvent& arg, MGE::InteractiveTexture* /*_activeTextureObject*/ ) {
	switch(arg.key) {
		case OIS::KC_ESCAPE:                                                 // Esc                 ==> show main menu
			injectEsc();
			return true;
		case OIS::KC_PAUSE:
			if (MGE::InputSystem::getPtr()->isModifierDown(OIS::Keyboard::Shift) ||                      // Pause + Shift       ==> full pause / full un-pause game (by EscMenu)
				isFullPaused()
			) {
				setFullPause(
					!isFullPaused()
				);
			} else {                                                         // Pause               ==> pause / un-pause game
				MGE::TimeSystem::getPtr()->switchPause();
			}
			return true;
		default:
			return false;
	}
}

void MGE::MainMenu::setFullPause(bool pause) {
	if (pause && !fullPaused) {
		if (!MGE::TimeSystem::getPtr()->gameIsPaused()) {
			MGE::TimeSystem::getPtr()->pause(0);
			pausedOnShow = true;
		} else {
			pausedOnShow = false;
		}
		MGE::TimeSystem::getPtr()->realtimeTimer->pause();
		WITH_NOT_NULL(MGE::AudioSystem::getPtr())->pauseAllSounds();
		#ifdef USE_OGGVIDEO
		auto videoMgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		if(videoMgr) videoMgr->pauseAllVideoClips();
		#endif
		fullPaused = true;
		LOG_INFO("Full Pause ON");
	} else if (!pause && fullPaused) {
		if(pausedOnShow)
			MGE::TimeSystem::getPtr()->unpause();
		
		MGE::TimeSystem::getPtr()->realtimeTimer->unpause();
		WITH_NOT_NULL(MGE::AudioSystem::getPtr())->resumeAllPausedSounds();
		#ifdef USE_OGGVIDEO
		auto videoMgr = static_cast<Ogre::OgreVideoManager*>(Ogre::OgreVideoManager::getSingletonPtr());
		if(videoMgr) videoMgr->unpauseAllVideoClips();
		#endif
		fullPaused = false;
		LOG_INFO("Full Pause OFF");
	}
}

bool MGE::MainMenu::isVisible() const {
	return menuWin->isVisible();
}

bool MGE::MainMenu::isFullPaused() const {
	return fullPaused;
}

bool MGE::MainMenu::mapsListSelectionChanged(const CEGUI::EventArgs& args) {
	MapEntryItem* item = static_cast<MapEntryItem*>( mapsList->getFirstSelectedItem() );
	
	if (item) {
		pugi::xml_document xmlFile;
		auto xmlRootNode = XMLUtils::openXMLFile(xmlFile, item->fileName.c_str(), "Mission");
		
		loadMapMenu->getChild( "MapInfo" )->setText(
			MGE::G11n::getLocaleStringFromXML(xmlRootNode, "Description")
		);
	}
	return true;
}

void  MGE::MainMenu::showLoadMapMenu(const std::string& group) {
	LOG_INFO("Loading modules list");
	
	mainMenu->hide();
	loadMapMenu->show();
	
	mapsList->clearList();
	
	Ogre::FileInfoListPtr filesInfo = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo(group, "*.xml"); 
	for (auto& fi : *filesInfo) {
		std::string path = fi.archive->getName() + "/" + fi.filename;
		LOG_INFO("Find module config file: " + path);
		
		pugi::xml_document xmlFile;
		auto xmlRootNode = XMLUtils::openXMLFile(xmlFile, path.c_str(), "Mission");
		
		CEGUI::StandardItem* item = new MapEntryItem(
			MGE::G11n::getLocaleStringFromXML(xmlRootNode, "Name"),
			path
  		);
		mapsList->addItem( item );
	}
}


bool MGE::MainMenu::handleAction(const CEGUI::EventArgs& args) {
	const CEGUI::WindowEventArgs& wargs = static_cast<const CEGUI::WindowEventArgs&>(args);
	const CEGUI::String tmpString = wargs.window->getName();
	
	static const std::string mapsConfigGroup = MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("MapsConfigGroupName").text().as_string("MGE_MapsMainConfigs");
	static const std::string defaultSaveDirPath = MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("SaveDirectrory").text().as_string("./saves");
	static const std::string autoSaveDirectory = MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("AutoSaveDirectrory").text().as_string("./saves/autosave");
	static const std::string defaultSceneFileDirPath = MGE::ConfigParser::getPtr()->getMainConfig("LoadAndSave").child("DefaultSceneFilesDirectory").text().as_string("./resources/GameConfig/Maps/");
	
	if (tmpString == "MainMenu : Exit") {
		MGE::LoadingSystem::getPtr()->writeSave(autoSaveDirectory + "/ExitGame.xml");
		MGE::Engine::getPtr()->shutDown();
	} else if (tmpString == "MainMenu : Start Game") {
		currMode = SUB_DIALOG;
		showLoadMapMenu(mapsConfigGroup);
	} else if (tmpString == "MainMenu : Save / Load") {
		mainMenu->hide();
		currMode = SUB_DIALOG;
		editorLoadSaveMode = false;
		if (MGE::LoadingSystem::getPtr()->getSceneLoadState() == MGE::LoadingSystem::GAME)
			openFileDialog->show(defaultSaveDirPath, "saves://", MGE::LoadingSystem::getPtr()->getSaveName(), true);
		else
			openFileDialog->show(defaultSaveDirPath, "saves://");
	} else if (tmpString == "MainMenu : Editor") {
		mainMenu->hide();
		currMode = SUB_DIALOG;
		editorLoadSaveMode = true;
		if (MGE::LoadingSystem::getPtr()->getSceneLoadState() == MGE::LoadingSystem::EDITOR)
			openFileDialog->show(".", "",  MGE::LoadingSystem::getPtr()->getLoadingFilePath(), true);
		else
			openFileDialog->show(".", "", defaultSceneFileDirPath, false);
	} else if (tmpString == "LoadMapMenu : Load") {
		MapEntryItem* item = static_cast<MapEntryItem*>( mapsList->getFirstSelectedItem() );
		if (item) {
			MGE::LoadingSystem::getPtr()->writeSave(autoSaveDirectory + "/LoadNewGame.xml");
			MGE::LoadingSystem::getPtr()->loadMapConfig( item->fileName );
		}
	} else if (tmpString == "OpenFileDialog : Load") {
		std::string fullPath = openFileDialog->getSelectedFile();
		LOG_INFO("GUIMainMenu: prepare to loading: " + fullPath);
		
		if (!fullPath.empty() && std::filesystem::is_regular_file(fullPath)) {
			if(!editorLoadSaveMode) {
				std::string autosavePath = autoSaveDirectory + "/LoadSavedGame.xml";
				if (
					std::filesystem::exists(std::filesystem::path(autosavePath)) &&
					std::filesystem::equivalent(std::filesystem::path(autosavePath), std::filesystem::path(fullPath))
				) {
					MGE::LoadingSystem::getPtr()->writeSave(autoSaveDirectory + "/LoadSavedGame2.xml");
				} else {
					MGE::LoadingSystem::getPtr()->writeSave(autosavePath);
				}
				MGE::LoadingSystem::getPtr()->loadSave(fullPath);
			} else {
				MGE::LoadingSystem::getPtr()->loadEditor(fullPath);
			}
		} else {
			LOG_WARNING("\"" + fullPath + "\" is not a FILE");
		}
	} else if (tmpString == "OpenFileDialog : Save") {
		std::string fullPath = openFileDialog->createSavePath();
		LOG_INFO("GUIMainMenu: prepare to saving to: " + fullPath);
		
		std::filesystem::path pFile(fullPath);
		if (std::filesystem::is_directory(pFile)) {
			LOG_WARNING("\"" + fullPath + "\" is not a FILE");
		} else if (std::filesystem::is_regular_file(pFile)) {
			std::string tmpPath;
			
			if(!editorLoadSaveMode) {
				tmpPath = autoSaveDirectory + "/Overwrite.xml";
			} else {
				tmpPath = fullPath + "~";
			}
			
			if (fullPath != tmpPath) {
				LOG_INFO("Save overwrite protection - rename: " + fullPath + " to: " + tmpPath);
				std::filesystem::rename(pFile, std::filesystem::path(tmpPath));
			}
		}
		
		if(!editorLoadSaveMode) {
			MGE::LoadingSystem::getPtr()->writeSave(fullPath);
		} else {
			MGE::LoadingSystem::getPtr()->writeScene(fullPath);
		}
		
		openFileDialog->reload();
	} else if (tmpString == "MainMenu : Continue" || tmpString == "LoadMapMenu : Back" || tmpString == "OpenFileDialog : Back" || tmpString == "EndGameMenu : Back") {
		injectEsc();
	} else {
		std::string buttonName( STRING_FROM_CEGUI(tmpString) );
		runMainMenuActionListeners.callAllWithKey( buttonName, &Listener::runMainMenuAction, buttonName );
	}
	
	return true;
}

MGE::MainMenu::MapEntryItem::MapEntryItem(const CEGUI::String& text, const std::string& file) :
	CEGUI::StandardItem(text, 0),
	fileName(file)
{ }

bool MGE::MainMenu::MapEntryItem::operator==(const CEGUI::GenericItem& other) const {
	const MapEntryItem* myOther = dynamic_cast<const MapEntryItem*>(&other);
	if (myOther && fileName != myOther->fileName)
		return false;
	return CEGUI::GenericItem::operator==(other);
}
