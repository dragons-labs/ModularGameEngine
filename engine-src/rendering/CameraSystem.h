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

#include "config.h"
#include "StoreRestoreSystem.h"
#include "BaseClasses.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "rendering/CameraNode.h"

#include <unordered_map>

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

/**
 * @brief %Camera manager and RTS-style controls
 */
class CameraSystem MGE_CLASS_FINAL :
	public MGE::Module,
	public MGE::Singleton<CameraSystem>,
	public MGE::SaveableToXML<CameraSystem>,
	public MGE::MainLoopListener
{
public:
	/**
	 * @brief Update all cameras
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	virtual bool update(float gameTimeStep, float realTimeStep) override;
	
	/**
	 * @brief Set current camera (camera which we will control)
	 * 
	 * @param      newCamera   pointer to MGE::Camera, when NULL reset to default camera
	 * @param[in]  audio       attach to camera Sound Listener for 3D audio (default true)
	 */
	void setCurrentCamera(MGE::CameraNode* newCamera = NULL, bool audio = true);
	
	/**
	 * @brief Return currently controlled camera
	 */
	inline MGE::CameraNode* getCurrentCamera() const {
		return currentCamera;
	}
	
	/**
	 * @brief Return SceneManager associated with currently controlled camera
	 */
	inline Ogre::SceneManager* getCurrentSceneManager() const {
		return currentCamera->getSceneManager();
	}
	
	/**
	 * @brief Set default camera (camera will be used when call setCurrentCamera with NULL)
	 * 
	 * @param      newCamera   pointer to new default camera
	 */
	inline void setDefaultCamera(MGE::CameraNode* newCamera) {
		defaultCamera = newCamera;
	}
	
	/// map name <-> pointer of all camera nodes
	std::unordered_map<std::string, MGE::CameraNode*, MGE::string_hash, std::equal_to<>> allCameraNodes;
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "CameraSystem";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
	/// constructor
	CameraSystem();
	
	/// destructor
	~CameraSystem();
	
private:
	/// pointer to current camera
	MGE::CameraNode* currentCamera;
	
	/// pointer to default camera
	MGE::CameraNode* defaultCamera;
};

/// @}

}
