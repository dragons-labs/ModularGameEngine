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
#include "BaseClasses.h"
#include "rendering/utils/VisibilityFlags.h"

namespace pugi { class xml_node; }

namespace MGE { struct LoadingContext; }

#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreMath.h>

namespace Ogre {
	class CompositorWorkspace;
}
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

/**
 * @brief Two node multi mode camera class for OGRE
 */
class CameraNode : MGE::NoCopyableNoMovable {
public:
	/** 
	 * @brief constructor - create camera
	 * 
	 * @param[in] name       name of camera and basename of camera nodes
	 * @param     scnMgr     pointer to SceneManager
	 */
	CameraNode(const std::string& name, Ogre::SceneManager* scnMgr);
	
	/// destructor
	~CameraNode();
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	bool storeToXML(pugi::xml_node&& xmlNode, bool onlyRef) const;
	
	/**
	 * @brief add compositor workspace with @a _renderTarget and this camera
	 * @param[in] _renderTarget  The render target texture
	 * @param[in] visibilityMask The visibility mask, see @ref MGE::AudioVideo::VisibilityFlags
	 * @param[in] ZOrder         The relative order with others on the target (allows overlapping viewports i.e. picture-in-picture).
	 * 
	 * @return created viewport
	 */
	void setRenderTarget(
		Ogre::TextureGpu* _renderTarget, uint32_t visibilityMask = MGE::VisibilityFlags::DEFAULT_MASK, int ZOrder=0
	);
	
	/**
	 * @brief set camera mode
	 * 
	 * @param[in] _rotationAllowed  when true camera can be rotated around node
	 * @param[in] _moveAllowed      when true node can be moved
	 * @param[in] _lookOutside      when true camera looks from node to world, when false camera look at node
	 */
	void setMode(bool _rotationAllowed = true, bool _moveAllowed = true, bool _lookOutside = false);
	
	/**
	 * @brief set camera owner
	 * 
	 * @param[in] _owner                 name of owner scene node
	 * @param[in] _autoGetOwnerPosition  when true camera node copy owner position
	 * @param[in] _autoGetOwnerRotation  when true camera node copy owner orientation
	 */
	void setOwner(const std::string_view& _owner, bool _autoGetOwnerPosition = true, bool _autoGetOwnerRotation = true);
	
	/**
	 * @brief set camera owner
	 * 
	 * @param[in] _owner                 pointer to owner scene node
	 * @param[in] _autoGetOwnerPosition  when true camera node copy owner position
	 * @param[in] _autoGetOwnerRotation  when true camera node copy owner orientation
	 */
	void setOwner(const Ogre::SceneNode* _owner = NULL, bool _autoGetOwnerPosition = true, bool _autoGetOwnerRotation = true);
	
	/**
	 * @brief return camera owner if set, otherwise NULL
	 */
	inline const Ogre::SceneNode* getOwner() const {
		return owner;
	}
	
	/**
	 * @brief update camera position and rotation
	 */
	void update();
	
	/// attach movable object to Camera Node
	inline void attachObject(Ogre::MovableObject* obj) {
		node->attachObject(obj);
	}
	/// detach movable object from Camera Node
	inline void detachObject(Ogre::MovableObject* obj) {
		node->detachObject(obj);
	}
	
	/**
	 * @brief write screenshot to file
	 * 
	 * @param[in] name    file name
	 */
	void writeScreenshot(const Ogre::String& name) const;
	
	/**
	 * @brief write screenshot to timestamped file
	 * 
	 * @param[in] prefix  file name prefix
	 * @param[in] suffix  file name suffix
	 * 
	 * @return screenshot filename
	 * 
	 * @note   filename is: prefix_YYYYMMDD_HHMMSS.suffix
	 */
	std::string writeScreenshot(const std::string_view& prefix, const std::string_view& suffix) const;
	
	/** 
	 * @name node position and orientation
	 * 
	 * @brief This is position and orientation of whole camera system
	 * 
	 * @{
	 */
		/**
		 * @brief return position
		 */
		inline Ogre::Vector3 getPosition() const {
			return node->getPosition();
		}
		
		/**
		 * @brief return orientation
		 */
		inline Ogre::Quaternion getOrientation() const {
			return node->getOrientation();
		}
		
		/**
		 * @brief Set node position by relative movment,
		 *        respect limitPositionMax and limitPositionMin.
		 * 
		 * @param[in] vec     new position
		 */
		void setPosition(const Ogre::Vector3& vec);
		
		/**
		 * @brief Modify node position by relative movment,
		 *        respect limitPositionMax and limitPositionMin.
		 * 
		 * @param[in] right    relative right   move at XZ surface (direction indicated by camera orientation)
		 * @param[in] up       relative change height
		 * @param[in] forward  relative forward move at XZ surface (direction indicated by camera orientation)
		 */
		void move(const float right, const float up, const float forward);
	
		/**
		 * @brief set orientation
		 * 
		 * @param[in] q       quaternion representing orientation to set
		 */
		inline void setOrientation (const Ogre::Quaternion& q) {
			node->setOrientation(q);
		}
		/**
		 * @brief rotate around an arbitrary axis
		 * 
		 * @param[in] axis    rotation axis
		 * @param[in] angle   rotation angle
		 */
		inline void rotate (const Ogre::Vector3& axis, const Ogre::Radian& angle) {
			node->rotate(axis, angle);
		}
		/**
		 * @brief rotate using a Quarternion
		 * 
		 * @param[in] q       quaternion representing rotation
		 */
		inline void rotate (const Ogre::Quaternion& q) {
			node->rotate(q);
		}
		
		/**
		 * @brief tell the node whether to yaw around it's own local Y axis or a fixed axis of choice
		 * 
		 * @param[in] useFixed    when true using fixed yaw axis
		 * @param[in] fixedAxis   global axis used to yaw
		 */
		inline void setFixedYawAxis(bool useFixed, const Ogre::Vector3& fixedAxis=Ogre::Vector3::UNIT_Y) {
			node->setFixedYawAxis(useFixed, fixedAxis);
		}
		
		/**
		 * @brief set direction vector (ie it's local -z)
		 * 
		 * @param[in] vec                   new direction verctor
		 * @param[in] relativeTo            direction is related to (local, parent or world) coordinate system
		 * @param[in] localDirectionVector  local front vector
		 */
		inline void setDirection (
			const Ogre::Vector3& vec,
			Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL,
			const Ogre::Vector3& localDirectionVector=Ogre::Vector3::NEGATIVE_UNIT_Z
		) {
			node->setDirection(vec, relativeTo, localDirectionVector);
		}
		/**
		 * @brief set direction vector to look at targetPoint
		 * 
		 * node will be rotated to @a localDirectionVector indicate @a targetPoint
		 * 
		 * @param[in] targetPoint           target point
		 * @param[in] relativeTo            targetPoint is in (local, parent or world) coordinate system
		 * @param[in] localDirectionVector  local front vector
		 */
		inline void lookAt (
			const Ogre::Vector3& targetPoint,
			Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL,
			const Ogre::Vector3& localDirectionVector=Ogre::Vector3::NEGATIVE_UNIT_Z
		) {
			node->lookAt(targetPoint, relativeTo, localDirectionVector);
		}
	/** 
	 * @}
	 * 
	 * @name camera position (zoom and orientation relative to node)
	 * 
	 * @brief this describe relative position of camera to node in spherical coordinate system
	 * 
	 * @{
	 */
		/**
		 * @brief set distance between camera and node (camera zoom)
		 *        respect limitZoomMin and limitZoomMax.
		 * 
		 * @param[in] a       new camera-target distance
		 */
		inline void setDistance(Ogre::Real a) {
			camDistance = Ogre::Math::Clamp(a, limitZoomMin, limitZoomMax);
			needInternalPositionUpdate = true;
		}
		/**
		 * @brief set angle betwen node Z axis and camera Z axis projection on node XZ plane
		 * 
		 * @param[in] a       new yaw value
		 */
		inline void setYaw(Ogre::Radian a) {
			camYaw    = a;
			camYawSin = Ogre::Math::Sin(camYaw);
			camYawCos = Ogre::Math::Cos(camYaw);
			needInternalPositionUpdate = true;
		}
		/**
		 * @brief set angle betwen node XZ plane and camera
		 *        respect limitPitchMin and limitPitchMax.
		 * 
		 * @param[in] a       new pitch value
		 */
		inline void setPitch(Ogre::Radian a) {
			camPitch    = Ogre::Math::Clamp(a, limitPitchMin, limitPitchMax);
			camPitchSin = Ogre::Math::Sin(camPitch);
			camPitchCos = Ogre::Math::Cos(camPitch);
			needInternalPositionUpdate = true;
		}
		
		/**
		 * @brief increment distance between camera and node (camera zoom)
		 *        respect limitZoomMin and limitZoomMax.
		 * 
		 * @param[in] a       distans to add to current camera-target distance
		 */
		inline void incDistance(Ogre::Real a) {
			setDistance(camDistance + a);
		}
		/**
		 * @brief increment angle betwen node Z axis and camera Z axis projection on node XZ plane
		 * 
		 * @param[in] a       angle to add to current yaw value
		 */
		inline void incYaw(Ogre::Radian a) {
			setYaw(camYaw + a);
		}
		/**
		 * @brief increment angle betwen node XZ plane and camera
		 *        respect limitPitchMin and limitPitchMax.
		 * 
		 * @param[in] a       angle to add to current pitch value
		 */
		inline void incPitch(Ogre::Radian a) {
			setPitch(camPitch + a);
		}
		
		/**
		 * @brief return camera zoom info
		 */
		inline Ogre::Real getZoom() const {
			return camDistance;
		}
		/**
		 * @brief return angle betwen node Z axis and camera Z axis projection on node XZ plane
		 */
		inline Ogre::Radian getYaw() const {
			return camYaw;
		}
		/**
		 * @brief return angle betwen node XZ plane and camera
		 */
		inline Ogre::Radian getPitch() const {
			return camPitch;
		}
	/** 
	 * @}
	 * 
	 * @name ogre camera stuff
	 * 
	 * @{
	 */
		/**
		 * @brief return camera name
		 */
		inline const Ogre::String& getName() const {
			return camera->getName();
		}
		/**
		 * @brief return camera
		 */
		inline Ogre::Camera* getCamera() {
			return camera;
		}
		/// @copydoc getCamera
		inline const Ogre::Camera* getCamera() const {
			return camera;
		}
		/**
		 * @brief get camera scene manager
		 */
		inline Ogre::SceneManager* getSceneManager() const {
			return camera->getSceneManager();
		}
		
		/**
		 * @brief get camera render target texture
		 */
		inline Ogre::TextureGpu* getRenderTarget() const {
			return renderTarget;
		}
		
		/**
		 * @brief add value to camera related visibilityMask
		 * 
		 * @param val value to add
		 */
		void addToVisibilityMask(uint32_t val);
		
		/**
		 * @brief remove value from camera related visibilityMask
		 * 
		 * @param val value to add
		 */
		void remFromVisibilityMask(uint32_t val);
		
		/**
		 * @brief get compositor workspace created for this camera
		 */
		Ogre::CompositorWorkspace* getWorkspace() const {
			return workspace;
		}
		
		/**
		 * @brief get ray from camera to screen position
		 * 
		 * @param[in] screenx - x screen position
		 * @param[in] screeny - y screen position
		 */
		inline Ogre::Ray getCameraRay(float screenx, float screeny) const {
			return camera->getCameraToViewportRay(screenx, screeny);
		}
		
		/**
		 * @brief increment Field of View angle
		 *        respect limitFOVMin and limitFOVMax.
		 * 
		 * @param[in] a       new FOV value
		 */
		inline void setFOV(Ogre::Radian a) {
			camera->setFOVy(Ogre::Math::Clamp(a, limitFOVMin, limitFOVMax));
		}
		/**
		 * @brief increment Field of View angle
		 *        respect limitFOVMin and limitFOVMax.
		 * 
		 * @param[in] a       angle to add to current new pitch value
		 */
		inline void incFOV(Ogre::Radian a) {
			setFOV( a + camera->getFOVy() );
		}
		/**
		 * @brief return Field of View angle
		 */
		inline const Ogre::Radian& getFOV() {
			return camera->getFOVy();
		}
	/** 
	 * @}
	 * 
	 * @name Camera limits
	 * 
	 * @{
	 */
		/// max position of node
		Ogre::Vector3 limitPositionMax;
		/// min position of node
		Ogre::Vector3 limitPositionMin;
		/// max camera zoom (distance betwen node and camera)
		Ogre::Real    limitZoomMax;
		/// min camera zoom (distance betwen node and camera)
		Ogre::Real    limitZoomMin;
		/// max camera pitch (angle betwen node XZ plane and camera)
		Ogre::Radian  limitPitchMax;
		/// min camera pitch (angle betwen node XZ plane and camera)
		Ogre::Radian  limitPitchMin;
		/// max camera Field of View
		Ogre::Radian  limitFOVMax;
		/// min camera Field of View
		Ogre::Radian  limitFOVMin;
	/** 
	 * @}
	 * 
	 * @name Camera control params
	 * 
	 * @{
	 */
		/// size of camera move step for keyboard
		Ogre::Real    kbdMoveStep;
		/// size of camera zoom step for keyboard
		Ogre::Real    kbdZoomStep;
		/// size of camera Field of View step for keyboard
		Ogre::Radian  kbdFOVStep;
		/// size of camera rotate step for keyboard
		Ogre::Radian  kbdRotateStep;
		
		/// multiplier to moving/zooming speed when press right shift
		Ogre::Real    shiftMultiplier;
		/// multiplier to zoom modificator for moving/zooming speed
		Ogre::Real    zoomMultiplier;
		
		/// size of camera move step for mouse
		Ogre::Real    mouseMoveStep;
		/// size of camera zoom step for mouse
		Ogre::Real    mouseZoomStep;
		/// size of camera Field of View step for mouse
		Ogre::Radian  mouseFOVStep;
		/// size of camera rotate step for mouse
		Ogre::Radian  mouseRotateStep;
		
		/// size of screen margins (relative to screen size), placing mouse cursor inside the margins area causes camera move
		Ogre::Real    mouseMaginSize;
		/// size of camera move step for mouse on screen margin
		Ogre::Real    mouseMaginStep;
	/** 
	 * @}
	 */
	
protected:
	/** 
	 * @name Camera objects
	 * 
	 * @{
	 */
		/// target node - camera is attached to this node
		/// and always look from (zoom relative) distance at/from this node
		Ogre::SceneNode*            node;
		
		/// ogre camera object
		Ogre::Camera*               camera;
		
		/// render target used by this camera
		Ogre::TextureGpu*           renderTarget;
		
		/// workspace used by this camera
		Ogre::CompositorWorkspace*  workspace;
	
	/** 
	 * @}
	 * 
	 * @name Camera orientation and zoom settings
	 * 
	 * @brief this describe relative position of camera to node in spherical coordinate system
	 * 
	 * @{
	 */
		/// distance betwen node and camera (camera zoom)
		Ogre::Real   camDistance;
		/// angle betwen node Z axis and camera Z axis projection on node XZ plane
		Ogre::Radian camYaw;
		/// angle betwen node XZ plane and camera
		Ogre::Radian camPitch;
		
		/// sin of @ref camYaw, updated when camYaw is updated
		Ogre::Real   camYawSin;
		/// cos of @ref camYaw, updated when camYaw is updated
		Ogre::Real   camYawCos;
		/// sin of @ref  camPitch, updated when camPitch is updated
		Ogre::Real   camPitchSin;
		/// cos of @ref  camPitch, updated when camPitch is updated
		Ogre::Real   camPitchCos;
		
		/// information in camDistance, camYaw or camPitch was changed, so we need
		/// recalculate and set (x,y,z) camera position (relative to node) and rotete camera to look at/from node
		bool needInternalPositionUpdate;
	/** 
	 * @}
	 * 
	 * @name Camera mode settings
	 * 
	 * @{
	 */
		/// camera owner scene node
		const Ogre::SceneNode* owner;
		
		/// look direction switch: when -1 look at node, when 1 look from node
		enum { LOOK_AT = -1, LOOK_FROM = 1 } lookDirection;
		
		/// allow camera rotation  relative to node
		bool rotationAllowed;
		
		/// allow node moving
		bool moveAllowed;
		
		/// value of moveAllowed when don't have owner
		bool moveAllowedNoOwner;
		
		/// copy owner position to node on update()
		/// when true owner can't be NULL
		bool autoGetOwnerPosition;
		
		/// copy owner orientation to node on update()
		/// when true owner can't be NULL
		bool autoGetOwnerRotation;
	/** 
	 * @}
	 */
	
	std::set<Ogre::CompositorPassSceneDef*>  scenePassDefs;
};

/// @}

}
