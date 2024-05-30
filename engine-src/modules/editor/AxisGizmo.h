/*
Copyright (c) 2008-2013 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team
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

#include "MainLoopListener.h"
#include "input/InputSystem.h"

#include <OgreSceneNode.h>

namespace MGE {

/// @addtogroup Modules
/// @{
/// @file

/**
 * @brief Object-oriented axis gizmo class
 */
class AxisGizmo :
	public MGE::MainLoopListener,
	public MGE::InputSystem::Listener
{
public:
	/** The Gizmo operation mode bit mask */
	enum Modes {
		MOVE   = (1 << 1),
		ROTATE = (1 << 2),
		SCALE  = (1 << 3),
		ALL    = MOVE | ROTATE | SCALE
	};
	
	/** The Axis enumeration bit mask */
	enum AXISTYPE {
		AXIS_X          = 1, /**  X Axis */
		AXIS_Y          = 2, /**  Y Axis */
		AXIS_XY         = 3, /** XY Axis */
		AXIS_Z          = 4, /**  Z Axis */
		AXIS_XZ         = 5, /** XZ Axis */
		AXIS_YZ         = 6, /** YZ Axis */
		AXIS_ALL        = 7  /** XYZ Axis */
	};
	
	static constexpr char AxisArray[3] = {AXIS_X, AXIS_Y, AXIS_Z};
	
	/** callback class for gizmo action */
	class Callback {
	public:
		/**
		 * @brief callback function for gizmo action
		 * 
		 * @param gizmoMode           gizmo operation mode (see @ref Modes)
		 * @param transformSpace      gizmo transform space
		 * @param axis                selected axis bitmask
		 * @param node                scene node which has attached gizmo
		 * @param mouseClickPoint     mouse position at operation start
		 * @param mouseCurrentPoint   current mouse position
		 * @param mouseArg            mouse move event
		 * @param endOfOperation      set true on last operation in sequence (mouse release)
		 */
		virtual void gizmoCallback(
			int gizmoMode, Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint, const OIS::MouseEvent& mouseArg,
			bool  endOfOperation
		) = 0;
		
		/**
		 * @brief return move vector (in PARENT space)
		 * 
		 * @param[in]     transformSpace     gizmo transform space
		 * @param[in]     axis               selected axis bitmask
		 * @param[in]     node               scene node which has attached gizmo
		 * @param[in]     ray                click ray (from camera)
		 * @param[in,out] offset             offset betwen click point and node position
		 * @param[in,out] offsetIsValid      when false get zeroOffset (and set to true), when true (moving in progress) use zeroOffset
		 */
		static Ogre::Vector3 getMove(
			Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Ray& ray, Ogre::Vector3& offset, bool& offsetIsValid
		);
		
		/**
		 * @brief return scale (in LOCAL space)
		 * 
		 * @param[in]     transformSpace     gizmo transform space
		 * @param[in]     axis               selected axis bitmask
		 * @param[in]     node               scene node which has attached gizmo
		 * @param         oldScale           scale at operation start
		 * @param         scaleFactor        scaling factor (takes into account mouse move size and scale speed factor)
		 * @param         posScaleFactor     scaling speed factor for scale > 1
		 */
		static Ogre::Vector3 getScale(
			Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Vector3& oldScale, const Ogre::Real& scaleFactor
		);
		
		/**
		 * @brief return scale (in LOCAL space)
		 * 
		 * @param[in]     transformSpace     gizmo transform space
		 * @param[in]     axis               selected axis bitmask
		 * @param[in]     node               scene node which has attached gizmo
		 * @param         mouseClickPoint    mouse position at operation start
		 * @param         mouseCurrentPoint  current mouse position
		 * @param         oldScale           scale at operation start
		 * @param         negScaleFactor     scaling speed factor for scale < 1
		 * @param         posScaleFactor     scaling speed factor for scale > 1
		 */
		static Ogre::Vector3 getScale(
			Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint,
			const Ogre::Vector3& oldScale, float negScaleFactor = 2.0, float posScaleFactor = 4.0
		);
		
		/**
		 * @brief return orientation (in PARENT space)
		 * 
		 * @param[in]     transformSpace     gizmo transform space
		 * @param[in]     axis               selected axis bitmask
		 * @param[in]     node               scene node which has attached gizmo
		 * @param         oldOrientation     orientation at operation start
		 * @param         rotateAngle        rotation angle value
		 */
		static Ogre::Quaternion getOrientation(
			Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Quaternion& oldOrientation, Ogre::Radian rotateAngle
		);
		
		/**
		 * @brief return orientation (in PARENT space)
		 * 
		 * @param[in]     transformSpace     gizmo transform space
		 * @param[in]     axis               selected axis bitmask
		 * @param[in]     node               scene node which has attached gizmo
		 * @param         mouseClickPoint    mouse position at operation start
		 * @param         mouseCurrentPoint  current mouse position
		 * @param         oldOrientation     orientation at operation start
		 * @param         rotateFactor       rotating speed factor
		 */
		static Ogre::Quaternion getOrientation(
			Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint,
			const Ogre::Quaternion& oldOrientation, float rotateFactor = 6.0
		);
		
		/**
		 * @brief calculate and return rotated position (in PARENT space)
		 * 
		 * @param[in]     target_node       scene node to compute rotated position
		 * @param[in]     pivot_position    position (in WORLD space) of rotating pivot point
		 * @param[in]     init_position     original position of @a target_node 
		 * @param[in]     init_orientation  original orientation of @a target_node (corresponding to @a init_position)
		 * @param[in]     new_orientation   new orientation of @a target_node (corresponding to new / returned position of @a target_node)
		 */
		static Ogre::Vector3 calculateRotatedPosition(
			const Ogre::SceneNode* target_node, Ogre::Vector3 pivot_position,
			const Ogre::Vector3& init_position, const Ogre::Quaternion& init_orientation, const Ogre::Quaternion& new_orientation
		);
		
		/// destructor
		virtual ~Callback() {}
	};
	
	/// simple visual callback class
	class VisualCallback : public Callback {
	public:
		/// constructor
		VisualCallback() : lastNode(NULL) { }
		
		/// @copydoc Callback::gizmoCallback
		virtual void gizmoCallback(
			int gizmoMode, Ogre::Node::TransformSpace transformSpace, int axis, Ogre::SceneNode* node,
			const Ogre::Vector2& mouseClickPoint, const Ogre::Vector2& mouseCurrentPoint, const OIS::MouseEvent& mouseArg,
			bool  endOfOperation
		) override;
		
		/// destructor
		virtual ~VisualCallback() {}
		
	protected:
		/// node to updating
		Ogre::SceneNode* lastNode;
		
		/// moving in progress (@ref zeroOffset is valid)
		bool              inMoveMode;
		
		/// offset betwen clicpoint and node position (used for moving)
		Ogre::Vector3     zeroOffset;
		
		/// initial node scale
		Ogre::Vector3     initScale;
		
		/// initial node orientation
		Ogre::Quaternion  initOrientation;
	};
	
	/// Constructor
	AxisGizmo(Ogre::SceneManager* scnMgr, float sizeFactor = 20.0);
	
	/// Destructor
	~AxisGizmo();
	
	
	/**
	 * @brief callback function for call on end of operation (mouse release) to inform about updated node state
	 * 
	 * @param mode            gizmo operation mode (see @ref Modes)
	 * @param transformSpace  gizmo transform space (orientation of gizmo will be copy from node respected @a transformSpace)
	 * @param callback        pointer to callback class
	 */
	void setMode(int mode, Ogre::Node::TransformSpace transformSpace, Callback* callback);
	
	/**
	 * @brief hide gizmo
	 */
	void hide();
	
	/**
	 * @brief show gizmo on @a node
	 * 
	 * @param node                ogre node to attach gizmo and to be target of gizmo operations
	 */
	void show(Ogre::SceneNode* node);
	
	/**
	 * @brief  switch visibility of gizmo
	 * 
	 * @param visible             true => show, false => hide
	 * @param node                ogre node to attach gizmo and to be target of gizmo operations
	 */
	void show(bool visible, Ogre::SceneNode* node);
	
	/**
	 * @brief return gizmo owner scene node
	 */
	inline Ogre::SceneNode* getOwnerNode() {
		return mOwnerNode;
	}
	
	/// return true when @a node is axis gizmo main-node or sub-node
	inline bool isGizmoNode(Ogre::SceneNode* node) {
		return (node == mGizmoNode || node == mGizmoX || node == mGizmoY || node == mGizmoZ || node == mGizmoA);
	}
	
	
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// @copydoc MGE::InputSystem::Listener::mouseMoved
	bool mouseMoved( const Ogre::Vector2& mousePos, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
	
	/// @copydoc MGE::InputSystem::Listener::mousePressed
	bool mousePressed( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture*& _activeTextureObject, CEGUI::Window* fromWindow = NULL ) override;
	
	/// @copydoc MGE::InputSystem::Listener::mouseReleased
	bool mouseReleased( const Ogre::Vector2& mousePos, OIS::MouseButtonID buttonID, const OIS::MouseEvent& arg, MGE::InteractiveTexture* _activeTextureObject ) override;
	
protected:
	/**
	 * Creates gizmo objects
	 */
	void CreateGizmo(Ogre::SceneManager* scnMgr);
	
	/**
	 * Destroys gizmo objects
	 */
	void DestroyGizmo();
	
	/**
	 * Highlights gizmo(s)
	 * 
	 * @param ID gizmo axis(i) flag(s) to highlight  (see @ref AXISTYPE)
	 */
	void HighlightGizmo(int ID);
	
	/**
	 * Attempts to pick X, Y or Z axis gizmo(s) presented when object is moved, scaled etc
	 * 
	 * @param ray ray cast from the mouse  
	 * @param Axis axis of the selected gizmo
	 * 
	 * @return true if intersected with any of gizmo(s), otherwise false
	 */
	bool PickGizmos(const Ogre::Ray& ray, int& Axis);
	
	/**
	 * Sets up move arrow gizmos
	 */
	Ogre::MeshPtr createMoveArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name);
	
	/**
	 * Sets up scale arrow gizmos
	 */
	Ogre::MeshPtr createScaleArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name);
	
	/**
	 * Sets up rotate arrow gizmos
	 */
	Ogre::MeshPtr createRotateArrowMesh(Ogre::SceneManager* manager, const Ogre::String& name);
	
private:
	Ogre::SceneNode*             mGizmoNode;                /**< Gizmo widget handle */
	Ogre::SceneNode*             mGizmoX;                   /**< X axis widget node handle */
	Ogre::SceneNode*             mGizmoY;                   /**< Y axis widget node handle */
	Ogre::SceneNode*             mGizmoZ;                   /**< Z axis widget node handle */
	Ogre::SceneNode*             mGizmoA;                   /**< All axis widget node handle */
	Ogre::Item*                  mGizmoEntities[13];        /**< Gizmo handles */
	std::vector<Ogre::Vector3>   vertices[5];               /**< Gizmo axis, plane and sphere mesh info - vertices */
	std::vector<int>             indices[5];                /**< Gizmo axis, plane and sphere mesh info - indices */
	
	int                          mGizmoMode;                /**< Gizmo operation mode */
	Ogre::Node::TransformSpace   mTransformSpace;           /**< Gizmo transform space */
	Callback*                    mGizmoCallback;            /**< pointer to callback class */
	Ogre::SceneNode*             mOwnerNode;                /**< Gizmo owner ogre scene node */
	
	int                          mCurrentAxis;              /**< current axis set */
	Ogre::Vector2                mMouseClickPoint;          /**< initial click on gizmo mouse position */
	Ogre::Vector3                mLastGizmoCamDist;         /**< last distance from camera - used to determinate need of update */
	
	float                        mGizmoSizeFactor;          /**< size factor */
	
	static constexpr float radius = 0.22f;
	static constexpr float accuracy = 8;
};

/// @}

}
