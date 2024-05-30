/*
Copyright (c) 2015-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/BaseComponent.h"

namespace MGE { struct BaseActor; }
namespace MGE { struct ActorFactory; }

#include <OgreAxisAlignedBox.h>

namespace Ogre {
	class  SceneNode;
	class  MovableObject;
	class  Vector3;
	class  Quaternion;
	class  AxisAlignedBox;
}

#include <forward_list>
#include <stdint.h>

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief interface struct for 3D world game object in MGE
 */
struct World3DObject :
	public MGE::BaseComponent
{
	/** 
	 * @name basic OGRE interface
	 * 
	 * @{
	 */
		/**
		 * @brief return object scene node
		 * 
		 * @note IF getOgreSceneNode() != NULL THEN MGE::BaseActor::get(obj->getOgreSceneNode()) == obj MUST BE TRUE
		 */
		virtual Ogre::SceneNode*   getOgreSceneNode() const = 0;
	/** 
	 * @}
	 * 
	 * 
	 * @name 3D world position and orientation
	 * 
	 * @{
	 */
		/**
		 * @brief return object position in world (not parent) coordinate
		 */
		virtual Ogre::Vector3 getWorldPosition() const;
		
		/**
		 * @brief return object position in world (not parent) coordinate
		 */
		virtual Ogre::Quaternion getWorldOrientation() const;
		
		/**
		 * @brief return object direction (it's local -z) in world (not parent) coordinate
		 */
		virtual Ogre::Vector3 getWorldDirection() const;
		
		
		/**
		 * @brief set object position in world (not parent) coordinate
		 * 
		 * @param[in] position      position Vector
		 */
		virtual void setWorldPosition(const Ogre::Vector3& position);
		
		/**
		 * @brief Set object position on ground (auto update y coordinate) in world (not parent) coordinate.
		 *        y coordinate in input position is used as offset from groud (positive => above, negative => under).
		 * 
		 * @param[in,out] position  destination point, function update Y coordinate
		 */
		virtual void setWorldPositionOnGround(Ogre::Vector3& position);
		
		/**
		 * @brief Find free space near to @a position and set object position on ground (auto update y coordinate) in world (not parent) coordinate.
		 *        y coordinate in input position is not used.
		 * 
		 * @param[in,out] position  destination point, function update Y coordinate
		 */
		virtual void findAndSetFreePositionOnGround(Ogre::Vector3& position);
		
		/**
		 * @brief set object world (not parent) orientation
		 * 
		 * @param[in] orientation   orientation Quaternion
		 */
		virtual void setWorldOrientation(const Ogre::Quaternion& orientation);
		
		/**
		 * @brief set object direction vector (it's local -z)
		 * 
		 * @param[in] direction     rotation vector
		 */
		virtual void setWorldDirection(Ogre::Vector3 direction);
		
		
		/**
		 * @brief update object cached information about transformation and world AABB
		 * 
		 * @param updateAABB    when true update worldAABB of node and (if enabled) child nodes
		 * @param recursive     when true update transformations (and AABBs if enabled) of child nodes
		 * @param updateParent  when true update transformations of parent node
		 */
		virtual void updateCachedTransform(bool updateAABB = true, bool recursive = true, bool updateParent = false);
	/** 
	 * @}
	 * 
	 * 
	 * @name AABB
	 * 
	 * @{
	 */
		/**
		 * @brief return object axis aligned bounding box
		 */
		virtual const Ogre::AxisAlignedBox& getAABB() const = 0;
		
		/**
		 * @brief return scaled and rotated (to world axis) object axis aligned bounding box
		 */
		virtual Ogre::AxisAlignedBox getWorldOrientedAABB() const;
	/** 
	 * @}
	 * 
	 * 
	 * @name move possiblity checking
	 * 
	 * @{
	 */
		/**
		 * @brief check possiblity of crossing from @a start to @a end point
		 * 
		 * @param[in]     start          first point of ray (with correct height)
		 * @param[in]     end            last point of ray (with correct height)
		 * @param[in,out] speedModifier  input value will be multiply by all crossable triggers speed modifiers
		 * @param[out]    squaredLength  squaredLength between @a start and @a end
		 * @param[out]    heightDiff     (not squared) height difference between @a start and @a end
		 * @param[out]    triggers       (when not NULL) list to store BaseActor pointers from crossable trigger
		 * @param[out]    collisionWith  (when not NULL) pointer to store Ogre::MovableObject pointer to collided object
		 * 
		 * @return if (value \< 0) error; if (value \> 0) success
		 *         full list of values see @ref MGE::PathFinder::ReturnCodes
		 */
		virtual int16_t canMove(
			const Ogre::Vector3& start, const Ogre::Vector3& end,
			float& speedModifier, float& squaredLength, float& heightDiff,
			std::forward_list<MGE::BaseActor*>* triggers = 0,
			Ogre::MovableObject** collisionWith = 0
		) const;
	/**
	 * @}
	 */
	
	/// set / update Ogre SceneNode
	virtual void setOgreSceneNode(Ogre::SceneNode* node) = 0;

	/// destructor
	virtual ~World3DObject() { }
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 0x01;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
};

/**
 * @brief interface struct for 3D world game object in MGE
 */
struct World3DObjectImpl :
	public MGE::World3DObject
{
	/// @copydoc MGE::World3DObject::getOgreSceneNode
	virtual Ogre::SceneNode*   getOgreSceneNode() const override;
	
	/// @copydoc MGE::World3DObject::getAABB
	virtual const Ogre::AxisAlignedBox& getAABB() const override;
	
	/// @copydoc MGE::World3DObject::setOgreSceneNode
	/// (and get / calculate local aabb)
	void setOgreSceneNode(Ogre::SceneNode* node) override;
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return MGE::World3DObject::classID;
	}
	
	/// constructor
	World3DObjectImpl(MGE::NamedObject* parent);
	
protected:
	friend struct ActorFactory;
	
	/// destructor
	virtual ~World3DObjectImpl() { }
	
	/// pointer to main ogre scene node
	Ogre::SceneNode*     mainSceneNode;
	
	/// axis aligned bounding box of full object
	Ogre::AxisAlignedBox aabb;
};


/// @}

}
