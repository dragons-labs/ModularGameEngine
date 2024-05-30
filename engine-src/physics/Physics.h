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

#include "BaseClasses.h"

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "config.h" // for USE_BULLET

namespace MGE { struct SceneObjectInfo; }

#include <OgreAny.h>

#ifdef USE_BULLET
	#include "physics/utils/OgreToBullet.h"
	#include <BtOgrePG.h>
	#include <BtOgreGP.h>
	#include <BtOgreExtras.h>
	class PosixThreadSupport;
#else
	class btDynamicsWorld;
	class btCollisionObject;
	class btAxisSweep3;
#endif

namespace Ogre {
	class Terrain;
	class TerrainGroup;
}

namespace MGE {

/// @addtogroup Physics
/// @{
/// @file

/**
 * @brief Implementation physics (colision and dynamics) with %Bullet (via btOgre)
 */
class Physics :
	public MGE::Module,
	public MGE::Unloadable,
	public MGE::MainLoopListener,
	public MGE::Singleton<Physics>
{
public:
	/**
	 * @brief initialise physics system
	 * 
	 * @note  can be called again to reinitialise physics system
	 * 
	 * @param[in] p1x       min x coordinate
	 * @param[in] p1y       min y coordinate
	 * @param[in] p1z       min z coordinate
	 * @param[in] p2x       max x coordinate
	 * @param[in] p2y       max y coordinate
	 * @param[in] p2z       max z coordinate
	 * @param[in] gravityX  x component of gravitation, default 0
	 * @param[in] gravityY  y component of gravitation, default -9.807
	 * @param[in] gravityZ  z component of gravitation, default 0
	 * @param[in] rayLen    len of ray, default - calculate from min, max world coordinate
	 */
	void configure (
		const float p1x, const float p1y, const float p1z, 
		const float p2x, const float p2y, const float p2z,
		const float gravityX = 0, const float gravityY = -9.807, const float gravityZ = 0,
		const float rayLen = 0
	);
	
	/**
	 * @brief remove (and destroy) all physics object (btCollisionObject, btRigidBody) from DynamicsWorld
	 */
	void clearDynamicsWorld();
	
	/**
	 * @brief   update physics world
	 * 
	 * @copydoc MGE::MainLoopListener::update
	 */
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/**
	 * @brief return pointer to DynamicsWorld
	 */
	btDynamicsWorld* getDynamicsWorld() const {
		return bulletWorld;
	}
		
	/// set ogre terrain (when not set or set to NULL don't use raycast to ogre terrain)
	void setTerrain(Ogre::TerrainGroup* terrain) {
		ogreTerrain = terrain;
	}
	
	/// return ogre terrain
	Ogre::TerrainGroup* getTerrain() const {
		return ogreTerrain;
	}
	
	#if defined(USE_BULLET) && defined(MGE_DEBUG_PHYSICS_DRAW)
	/**
	 * @brief preparing debug mod for bullet (showing debug draw ...).
	 * 
	 * @param[in] scnMgr  pointer to SceneManager using to draw debug lines
	 */
	void createDebugDraw(Ogre::SceneManager* scnMgr);
	
	/**
	 * @brief Enable/disable debug mod for bullet (showing debug draw ...).
	 * 
	 * @param[in] enabled
	 */
	void setDebugMode(bool enabled);
	#endif

	/**
	 * @brief create physics object (btCollisionObject or btRigidBody) for Ogre entity / node
	 * 
	 * @param node               ogre node to add physics
	 * @param movable            Ogre::Item or Ogre::v1::Entity to add physics / get shape information
	 * @param physicsMode        physics mode (colision, full, ...)
	 * @param shapeMode          getting collision shape mode
	 * @param shapeFile          (optional - depends on getting shape mode) file with shape
	 * @param mass               mass of object
	 * @param collisionFlag      set of binary flags determining this object type, see @ref MGE::QueryFlags
	 * @param collisionMask      binary mask determining types of objects with this object will collided
	 *
	 * @return created physics object or NULL (on error)
	 */
	static btCollisionObject* createPhysicsObject(
		Ogre::SceneNode* node, Ogre::MovableObject* movable,
		const std::string_view& physicsMode, const std::string_view& shapeMode,
		const std::string& shapeFile,
		const Ogre::Real& mass, int collisionFlag, int collisionMask
	);
	
	/**
	 * @brief create physics object (btCollisionObject or btRigidBody) for Ogre terrain
	 *
	 * @return created physics object or NULL (on error)
	 */
	static btCollisionObject* createPhysicsObject(Ogre::Terrain * pTerrain);
	
	/**
	 * @brief create physics object (btCollisionObject or btRigidBody) for Ogre terrain
	 *
	 * @return created physics object or NULL (on error)
	 */
	static btCollisionObject* createTerrainPhysics(Ogre::TerrainGroup* terrain, Ogre::Vector3 point);
	
	/**
	 * @brief recursive delete physics object (btCollisionObject or btRigidBody and all btCollisionShape)
	 */
	static void deletePhysicsObject(btCollisionObject* physicsBody);
	
	/**
	 * @brief create object physic based on XML config node
	 * 
	 * Implementation of @ref MGE::SceneLoader::SceneNodesCreateFunction.
	 */
	static void processPhysicsXMLNode(
		const pugi::xml_node&       xmlNode,
		const MGE::LoadingContext*  context,
		const MGE::SceneObjectInfo& parent
	);
	
	/**
	 * @brief set world size based on XML config node
	 * 
	 * @copydetails MGE::LoadingSystem::SceneConfigParseFunction
	 */
	static MGE::Module* processWorldSizeXMLNode(
		const pugi::xml_node& xmlNode,
		const MGE::LoadingContext* context
	);
	
	/**
	 * @brief create terrain based on XML config node
	 * 
	 * @copydetails MGE::LoadingSystem::SceneConfigParseFunction
	 */
	static MGE::Module* processTerrainXMLNode(
		const pugi::xml_node& xmlNode,
		const MGE::LoadingContext* context
	);
	
	/**
	 * @brief create bullet subsystem with bullet world based on XML config node
	 * 
	 * @copydetails MGE::LoadingSystem::SceneConfigParseFunction
	 */
	static MGE::Module* createBullet(
		const pugi::xml_node& xmlNode,
		const MGE::LoadingContext* context
	);
	
	///  @copydoc MGE::SaveableToXML::unload
	bool unload() override;
	
	/// Struct to wrap btCollisionObject before put in "phy" user bindings.
	/// Need to call @ref deletePhysicsObject on physics object while removing scene object.
	struct AnyHolder : MGE::NoCopyableNoMovable {
		/// Pointer to scene object physics object.
		btCollisionObject* physicsBody;
		
		/// Constructor.
		AnyHolder(btCollisionObject* x) : physicsBody(x) {}

		/// Destructor.
		/// Will be called by Ogre::Any destructor on destroy Ogre::Node (with "phy" binding).
		~AnyHolder();
		
		/// write to output stream (need by Ogre::Any)
		friend inline std::ostream& operator<<(std::ostream& s, const AnyHolder& obj) { // need by Any
			s << "Physics::AnyHolder " << obj.physicsBody << "\n";
			return s;
		}
	};
	
	/**
	 * @brief constructor -  create physics system
	 * 
	 * @note  after creating system required initializing via @ref configure()
	 */
	Physics();
	
private:
	/// destructor
	~Physics();
	
	/// create bullet
	void* createBullet(Ogre::SceneManager* scnMgr);
	
	/// destroy bullet
	void destroyBullet();
	
	/// pointer to Ogre Terrain
	Ogre::TerrainGroup* ogreTerrain;
	
	/// pointer to Bullet Dynamics world
	btDynamicsWorld* bulletWorld;
	
	/// transform updater from Ogre to Bullet
	MGE::OgreToBullet ogre2bullet;
	
	#ifdef USE_BULLET
		btAxisSweep3* bulletBroadphase;
		btDefaultCollisionConfiguration* bulletCollisionConfig;
		
		btCollisionDispatcher* bulletDispatcher;
		btSequentialImpulseConstraintSolver* bulletSolver;
		
		#ifdef BULLET_USE_POSIX_THREADS
		PosixThreadSupport* threadSupportDispatcher;
		PosixThreadSupport* threadSupportSolver;
		#endif
		
		#ifdef MGE_DEBUG_PHYSICS_DRAW
		BtOgre::DebugDrawer* debugDraw;
		#endif
	#endif
	
};

/// @}

}
