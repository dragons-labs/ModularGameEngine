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

#pragma   once

#include "BaseClasses.h"
#include "StringTypedefs.h"
#include "ModuleBase.h"

#include "data/utils/OgreSceneObjectInfo.h"

namespace Ogre { class SceneManager; class SceneNode; }
namespace pugi { class xml_node; class xml_document; }
namespace MGE { class LoadingScreen; }

namespace MGE {

/// @addtogroup LoadingSystem
/// @{
/// @file

/**
 * @brief Implementation "load and save system" for write save file, and loading configure and save file.
 * 
 * This load and save system:
 *   - is initial point for:
 *     - creating scene (from config or save file)
 *     - writing scene state to save file
 *     - writing scene to .scene.xml file (for editor mode)
 *   - provide some utils function for loading resources, parsing .scene.xml files, etc
 *   - keeps info about state of loading the scene (load state value, source file path, etc)
 *   - load vs restore pipeline:
 *     - on save restore (@ref loadSave):
 *       - first @ref loadMapConfig will be called with `preloadOnly == true`, in order of XML nodes:
 *         - use class registered in @ref MGE::ConfigParser::configParserListeners via @ref MGE_REGISTER_MODULE and @ref MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG
 *         - use class registered in @ref MGE::SceneLoader::sceneNodesCreateListeners via @ref MGE_REGISTER_SCENE_ELEMENT
 *         - load "StateFile" via @ref loadSave
 *         - load "ConfigScripts" via @ref loadScripts
 *       - next @ref MGE::StoreRestoreSystem::restoreFromXML will be called
 *         - use class registered in @ref MGE::StoreRestoreSystem::restoreListeners via @ref MGE::StoreRestoreSystem::addSaveListener)
 *     - on load only (not restoring from save):
 *       - only @ref loadMapConfig will be called with `preloadOnly == false` (for details see above)
 *   - uses / calls listeners for:
 *     - loading from xml config
 *       - @ref MGE::ConfigParser::configParserListeners (registration via @ref MGE_REGISTER_MODULE and @ref MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG)
 *         for parsing main map / mission config and top level xml elements in .scene.xml files including from map/mission config
 *       - @ref MGE::SceneLoader::sceneNodesCreateListeners (registration via @ref MGE_REGISTER_SCENE_ELEMENT)
 *         for parsing \<nodes\> in .scene.xml
 *       - function will be called for each occurrence of xml with registred tag name (at multiple level of xml files, it can be called recursively)
 *       - the order of calls in the order of occurrence xml elements in main config file and .scene.xml files
 *     - clear scene
 *       - @ref MGE::StoreRestoreSystem::unloadListeners (int -- Listener derivered object) call @ref MGE::UnloadableInterface::unload
 *       - function will be called once for each registered listener object
 *       - the order of calls in the order of integer number used to registered listener (but multiple object can ust the same value)
 *     - saving scene / game state to xml save file:
 *       - @ref MGE::StoreRestoreSystem::saveListeners (int -- Listener derivered object) call @ref MGE::SaveableToXMLInterface::storeToXML
 *       - function will be called once for each registered listener object
 *       - the order of calls in the order of integer number used to registered listener (but multiple object can ust the same value)
 *     - restoring scene / game state from xml save file:
 *       - @ref MGE::StoreRestoreSystem::restoreListeners (xml tag name -- Listener derivered object) call @ref MGE::SaveableToXMLInterface::restoreFromXML
 *       - function will be called for each occurrence of xml tag with registered name (at top level of save file, but can be called internally from other places too)
 *       - the order of calls in the order of occurrence xml elements in xml save files
 *   - @ref loadSave can be used to load state file (aka "fake save file" - save file without a specified map config file to restore),
 *     in this case save is apply to current scene (clearScene and loading listeners are not called)
 */
class LoadingSystem :
	public MGE::Module,
	public MGE::Singleton<LoadingSystem>
{
public:
	/// states of scene loading
	enum SceneLoadStates {
		/// scene is not loaded
		NO_SCENE = 0,
		/// scene is loading / unloading
		IN_PROGRESS,
		/// scene is loaded in game mode
		GAME,
		/// scene is loaded in editor mode
		EDITOR,
	};
	
	/**
	 * @brief create new game scene based on config file
	 * 
	 * @param mapConfigFilePath     Map config file path.
	 * @param preloadOnly           When true: in next step will be loaded save and can skip some part of loading.
	 * @param mainDotSceneFilePath  Dot scene config file path to read main scene setting (scene manager),
	 *                              when empty get this file path from content of @a mapConfigFilePath.
	 * @param loadType              Loading type (GAME or EDITOR).
	 *                              If @a preloadOnly == false when will be set as @ref sceneLoadState after loaded finished.
	 */
	void loadMapConfig( const std::string& filePath, bool preloadOnly = false, std::string mainDotSceneFilePath = MGE::EMPTY_STRING, SceneLoadStates loadType = GAME );
	
	/**
	 * @brief load Game from file
	 * 
	 * @param filePath        file to load xml
	 * @param _isRealSaveFile  when true: read from @a filePath name of map config file (and load map from it)
	 *                         false is used to load stateFile from map config file
	 */
	void loadSave( const std::string& filePath, bool _isRealSaveFile = true );
	
	/**
	 * @brief load scene from .scene xml string
	 * 
	 * @param xmlNode     Dot scene config root XML node ("scene").
	 * @param context     Structure with info about restoring/loading context.  If null, then use defualt LoadingSystem context.
	 * @param parent      Parent scene node. If null, then use root scene node.
	 */
	void loadDotScene(
		const pugi::xml_node& xmlNode,
		const MGE::LoadingContext* context = nullptr,
		Ogre::SceneNode* parent = nullptr
	);
	
	/**
	 * @brief load scene from .scene file
	 * 
	 * @param filePath    Path to dot scene xml file.
	 * @param context     Structure with info about restoring/loading context.  If null, then use defualt LoadingSystem context.
	 * @param parent      Parent scene node. If null, then use root scene node.
	 * @param xmlDoc      If not null, then put pointer to pugi::xml_document used to read and parse @a filePath .scene file (instead delete it).
	 */
	void loadDotSceneFile(
		const std::string& filePath,
		const MGE::LoadingContext* context = nullptr,
		Ogre::SceneNode* parent = nullptr,
		pugi::xml_document** xmlDoc = nullptr
	);
	
	/**
	 * @brief load scene from .scene xml string
	 * 
	 * @param xmlStr      Dot scene content as string.
	 * @param context     Structure with info about restoring/loading context.  If null, then use defualt LoadingSystem context.
	 * @param parent      Parent scene node. If null, then use root scene node.
	 */
	void loadDotSceneXML(
		const std::string& xmlStr,
		const MGE::LoadingContext* context = nullptr,
		Ogre::SceneNode* parent = nullptr
	);
	
	/**
	 * @brief clear scene
	 */
	void clearScene();
	
	/**
	 * @brief load Game from file
	 * 
	 * @param _mapFile         path to .scene xml file
	 */
	void loadEditor( const std::string& _mapFile );
	
	/**
	 * @brief save Game to file
	 * 
	 * @param filePath        file to save xml
	 * 
	 * @return True when save successful, false otherwise (game not loaded, write error, ...).
	 */
	bool writeSave( const std::string& filePath );
	
	/**
	 * @brief save edited scene to file
	 * 
	 * @param filePath        file to save xml
	 * 
	 * @return True when save successful, false otherwise (scene not loaded, write error, ...).
	 */
	bool writeScene( const std::string& filePath );
	
	/**
	 * @brief return sugested save name for current game state
	 */
	std::string getSaveName();
	
	/**
	 * @brief return map config file path or .scene file path
	 * 
	 * @li @ref sceneLoadState == GAME    =>  path to config file used to create current game scene
	 * @li @ref sceneLoadState == EDITOR  =>  path to .scene file to load in editor
	 */
	const std::string& getLoadingFilePath() const {
		return configFile;
	}
	
	/**
	 * @brief return true when game scene is loaded
	 */
	inline SceneLoadStates getSceneLoadState() const {
		return sceneLoadState;
	}
	
	/**
	 * @brief return Ogre SceneManager used to create current scene
	 */
	Ogre::SceneManager* getGameSceneManager() const {
		return loadingContext.scnMgr;
	}
	
	/**
	 * @brief set context menu (for right click selection) implementation object
	 */
	inline void setLoadingScreen(MGE::LoadingScreen* ls) {
		loadingScreen = ls;
	}
	
	/// constructor
	LoadingSystem();
	
	/**
	 * @brief load scripts from Ogre ResourceGroup
	 * 
	 * @param[in] group       name of ResourceGroup containing python scripts
	 * @param[in] filter      load only maching to filter files (default "*.py")
	 * 
	 * function recursive load all .py file from ResourceGroup
	 */
	void loadScriptsFromResourceGroup(const Ogre::String& group = "Scripts", const Ogre::String& filter = "*.py");
	
protected:
	/// destructor
	~LoadingSystem();
	
	/// Loading context - Ogre Scene Manager and other (not scene graph / not parent related) info used for loaded scene elements
	MGE::LoadingContext  loadingContext;
	
	/**
	 * @brief map config file path or .scene file path
	 * 
	 * @li @ref sceneLoadState == GAME    =>  path to config file used to create current game scene
	 * @li @ref sceneLoadState == EDITOR  =>  path to .scene file to load in editor
	 */
	std::string          configFile;
	
	/// name of scene for creating save name (unused when sceneLoadState != GAME)
	std::string          sceneName;
	
	/// pointer to xml document parser used for saving edited .scene file
	pugi::xml_document*  editedDotSceneXMLParser;
	
	/// indicator of scene loaded state
	SceneLoadStates      sceneLoadState;
	
	/// Loading screen with progress bar
	MGE::LoadingScreen*  loadingScreen;
	
	/// load script from mission / map file config entry
	void loadScripts(const pugi::xml_node& xmlNode);
	
	/// do post loading stuff ... e.g. set sceneLoadState
	void finishLoading(SceneLoadStates loadType);
};

/// @}

}
