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

#include "LogSystem.h"
#include "BaseClasses.h"
#include "StringTypedefs.h"
#include "MessagesSystem.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

namespace MGE { class CameraNode; }

#include <OgreRoot.h>
#include <OgreWindow.h>
#include <OgreWindowEventUtilities.h>
#include <OgreLog.h>
#include <OgrePlatformInformation.h>


namespace pugi { class xml_node; }

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

/**
 * @brief 
 * 
 */
class RenderingSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public Ogre::WindowEventListener,
	public MGE::Singleton<RenderingSystem>
{
public:
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/** 
	 * @name OGRE system initialising
	 * @{
	 */
		/**
		 * @brief Create and return pointer to Ogre SceneManager.
		 * 
		 * @param[in] type           numeric type mask or type name of created SceneManager
		 * @param[in] instanceName   name of created instance of SceneManager
		 * 
		 * @see Ogre::Root::createSceneManager()
		 */
		template <typename T = Ogre::SceneTypeMask> Ogre::SceneManager* createSceneManager(
			const T& type,
			const Ogre::String& instanceName,
			size_t numWorkerThreads = 0
		) {
			LOG_INFO("SceneManager", "Create with name=" << instanceName << " type=" << type << " numWorkerThreads=" << numWorkerThreads);
			
			if (numWorkerThreads == 0) {
				numWorkerThreads = std::max<size_t>(1, Ogre::PlatformInformation::getNumLogicalCores());
			}
			Ogre::SceneManager* scnMgr = ogreRoot->createSceneManager(
				type,
				numWorkerThreads,
				instanceName
			);
			
			LOG_INFO("SceneManager", "Successfully created with: name=" << scnMgr->getName() << " type=" << scnMgr->getTypeName() << " numWorkerThreads=" << numWorkerThreads << " (" << scnMgr << ")");
			
			return scnMgr;
		}
		
		/**
		 * @brief Create and init to Ogre SceneManager based on xml config
		 */
		Ogre::SceneManager* initSceneManager(const pugi::xml_node& xmlNode);
		
		/**
		 * @brief Destroy Ogre SceneManager.
		 * 
		 * @param scnMgr  pointer to SceneManager to destroy, after call this function pointer is invalidated
		 * 
		 * @see Ogre::Root::destroySceneManager()
		 */
		void destroySceneManager(Ogre::SceneManager*& scnMgr);
	/**
	 * @}
	 */
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @name Rendering
	 * @{
	 */
		/**
		 * @brief Starts / restarts the automatic rendering cycle
		 * 
		 * @see Ogre::Root::startRendering()
		 */
		inline void startRendering(void) {
			ogreRoot->startRendering();
		}
		
		/**
		 * @brief Render single frame.
		 * 
		 * @see Ogre::Root::renderOneFrame()
		 */
		inline bool renderOneFrame(void) {
			return ogreRoot->renderOneFrame();
		}
		
		/**
		 * @brief Render single frame.
		 * 
		 * @param[in] timeSinceLastFrame - time in seconds from last rendered frame
		 * 
		 * @see Ogre::Root::renderOneFrame()
		 */
		inline bool renderOneFrame(Ogre::Real timeSinceLastFrame) {
			return ogreRoot->renderOneFrame(timeSinceLastFrame);
		}
	/**
	 * @}
	 */
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * @name Utils, etc
	 * @{
	 */
		/**
		 * @brief return pointer to RenderWindow
		 */
		FORCE_INLINE Ogre::Window* getRenderWindow(void) const {
			return renderWindow;
		}
		
		/**
		 * @brief return pointer to loading time SceneManager
		 */
		Ogre::SceneManager* getLoadingSceneManager() {
			return loadingSceneManager;
		}
		
		void createLoadingCamera(Ogre::SceneManager* scnMgr = NULL);
		
		/**
		 * @brief destroy loading time SceneManager (and Camera)
		 */
		void destroyLoadingSceneManager();
		
		/**
		 * @brief destroy loading time Camera
		 */
		void destroyLoadingCamera();
		
	/**
	 * @}
	 */
	
	/**
	 * @brief call renderOneFrame()
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
	
	
	/**
	 * @brief constructor -- create Ogre::Root and Ogre::RenderWindow.
	 * 
	 * @param[in] window_name    name of rendering window
	 * @param[in] plugin_cfg     config file for OGRE plugin configuration
	 * @param[in] ogre_cfg       config file for OGRE rendering configuration
	 */
	RenderingSystem (
		const std::string& window_name,
		const std::string& plugin_cfg,
		const std::string& ogre_cfg
	);
	
	/**
	 * @brief create loading screen (pre-scene rendering)
	 */
	void createLoadingScreen(const pugi::xml_node& xml);
	
	/// destructor -- unregister WindowEventListener, shutting down OGRE
	~RenderingSystem();
	
private:
	/// @name Ogre::WindowEventListener
	/// @{
		/// window resize event
		void windowResized(Ogre::Window* rw) override;
		/// window close event
		void windowClosed(Ogre::Window* rw) override;
	/// @}
	
	/// @brief pointer to Ogre Root
	/// @note This is singleton, so: ogreRoot == Ogre::Root::getSingletonPtr()
	Ogre::Root*                  ogreRoot;
	
	/// @brief pointer to Ogre Window
	/// @note This is auto create render window, so: renderWindow == ogreRoot->getAutoCreatedWindow()
	Ogre::Window*                renderWindow;
	
	/// pointer to loading time scene menager
	Ogre::SceneManager*          loadingSceneManager;
	
	/// pointer to loading time camera
	MGE::CameraNode*             loadingScreenCamera;
	
	#ifndef __DOCUMENTATION_GENERATOR__
	class MyOgreLogger final : public Ogre::LogListener {
	public:
		MyOgreLogger();
		~MyOgreLogger();
		void messageLogged (
			const Ogre::String& message, Ogre::LogMessageLevel lml,
			bool maskDebug, const Ogre::String& logName, bool& skipThisMessage
		) override;
	private:
		Ogre::LogManager* ogreLogManager;
	};
	#endif
	
	/// pointer to class inheriting from Ogre::LogListener, which uses MGE::Utils::LogSystem for writing logs
	MyOgreLogger*     ogreLogger;
};

/// @}

}
