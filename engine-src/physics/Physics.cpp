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

Inspired by:
	→ BtOgre "demo" (https://github.com/nikki93/btogre/tree/master/demo) (Zlib licensed)
	→ Bullet "MultiThreadedDemo" (https://github.com/bulletphysics/bullet3/tree/master/examples/MultiThreadedDemo) (Zlib licensed)
*/

#include "physics/Physics.h"

#include "ConfigParser.h"
#include "SceneLoader.h"
#include "StoreRestoreSystem.h"

#include "data/structs/BaseActor.h"
#include "physics/utils/HexagonalGrid.h"
// #include "system/ScriptsSystem.h"
#include "physics/Raycast.h"
#include "physics/PathFinder.h"
#include "physics/utils/WorldSizeInfo.h"
#include "data/utils/OgreSceneObjectInfo.h"

#ifdef USE_BULLET
	#include <BulletCollision/CollisionDispatch/btSimulationIslandManager.h>
	#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
	#ifdef USE_BULLET_IMPORTER
		#include <BulletWorldImporter/btBulletWorldImporter.h>
	#endif
	
	#ifdef BULLET_USE_POSIX_THREADS
		#include <BulletMultiThreaded/SpuCollisionTaskProcess.h>
		#include <BulletMultiThreaded/SpuGatheringCollisionDispatcher.h>
		#include <BulletMultiThreaded/btParallelConstraintSolver.h>
	#endif
#endif

#include <OgreEntity.h>
#include <pybind11/gil.h>

MGE::Physics::AnyHolder::~AnyHolder() {
	LOG_DEBUG("delete physics object from any"); /// @todo TODO.4: TEST is this called while destroying node?
	deletePhysicsObject(physicsBody);
}

/**
@page XMLSyntax_MainConfig

@subsection XMLNode_Physics \<Physics\>

@c \<Physics\> is used for setup <b>Physics</b>. This node do not contain any subnodes nor attributes.
*/

MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(Physics) {
	return new MGE::Physics();
}


MGE::Physics::Physics::Physics() :
	MGE::Unloadable(900)
	, ogreTerrain(NULL)
	, bulletWorld(NULL)
	#ifdef USE_BULLET
	, bulletBroadphase(NULL), bulletCollisionConfig(NULL), bulletDispatcher(NULL), bulletSolver(NULL)
	#ifdef MGE_DEBUG_PHYSICS_DRAW
	, debugDraw(NULL)
	#endif
	#endif
{
	LOG_HEADER("Create physics system");
	
	// register map config and .scene file xml tags listener
	#ifdef USE_BULLET
	MGE::ConfigParser::getPtr()->addConfigParseListener("Bullet", MGE::Physics::Physics::createBullet);
	#endif
	MGE::ConfigParser::getPtr()->addConfigParseListener("worldSize", MGE::Physics::Physics::processWorldSizeXMLNode);
	MGE::ConfigParser::getPtr()->addConfigParseListener("Terrain", MGE::Physics::Physics::processTerrainXMLNode);
	
	// register .scene file xml tags listener
	#ifdef USE_BULLET
	MGE::SceneLoader::getPtr()->addSceneNodesCreateListener(
		"physics", reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::Physics::Physics::processPhysicsXMLNode)
	);
	#endif
}

MGE::Physics::Physics::~Physics(void) {
	unload();
	
	MGE::ConfigParser::getPtr()->remConfigParseListener(MGE::Physics::Physics::processWorldSizeXMLNode);
	MGE::ConfigParser::getPtr()->remConfigParseListener(MGE::Physics::Physics::processTerrainXMLNode);
	#ifdef USE_BULLET
	MGE::ConfigParser::getPtr()->remConfigParseListener(MGE::Physics::Physics::createBullet);
	MGE::SceneLoader::getPtr()->remSceneNodesCreateListener(
		reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(MGE::Physics::Physics::processPhysicsXMLNode)
	);
	#endif
}

bool MGE::Physics::Physics::unload() {
	LOG_INFO("Unload physics system");
	
	if (ogreTerrain) {
		LOG_INFO("Destroy Terrain");
		
		/// @todo TODO.9: [Terrain] delete terrain
		ogreTerrain = NULL;
	}
	
	destroyBullet();
	ogre2bullet.clearAll();
	return true;
}


void* MGE::Physics::Physics::createBullet(Ogre::SceneManager* scnMgr) {
#ifdef USE_BULLET
	LOG_INFO("Create Bullet physics system");
	
	bulletCollisionConfig = new btDefaultCollisionConfiguration();
	
	#if BULLET_MAX_DISPATCHER_THREADS > 0
		#ifdef BULLET_USE_POSIX_THREADS
			PosixThreadSupport::ThreadConstructionInfo dispatcherConstructionInfo(
				"collision", processCollisionTask, createCollisionLocalStoreMemory, BULLET_MAX_DISPATCHER_THREADS);
			threadSupportDispatcher = new PosixThreadSupport(dispatcherConstructionInfo);
			bulletDispatcher = new SpuGatheringCollisionDispatcher(threadSupportDispatcher, BULLET_MAX_DISPATCHER_THREADS, bulletCollisionConfig);
		#else
			#warning "BULLET_MAX_DISPATCHER_THREADS > 0 but not supported type of BULLET_MULTITHREADS"
			#undef    BULLET_MAX_DISPATCHER_THREADS
			#define   BULLET_MAX_DISPATCHER_THREADS 0
		#endif
	#endif
	
	#if BULLET_MAX_DISPATCHER_THREADS == 0
		bulletDispatcher = new btCollisionDispatcher(bulletCollisionConfig);
	#endif
	
	#if BULLET_MAX_SOLVER_THREADS > 0
		#ifdef BULLET_USE_POSIX_THREADS
			PosixThreadSupport::ThreadConstructionInfo solverConstructionInfo(
				"solver", SolverThreadFunc, SolverlsMemoryFunc, BULLET_MAX_SOLVER_THREADS);
			threadSupportSolver = new PosixThreadSupport(solverConstructionInfo);
			bulletSolver = new btParallelConstraintSolver(threadSupportSolver);
			bulletDispatcher->setDispatcherFlags(btCollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);
		#else
			#warning "BULLET_MAX_SOLVER_THREADS > 0 but not supported type of BULLET_MULTITHREADS"
			#undef    BULLET_MAX_SOLVER_THREADS
			#define   BULLET_MAX_SOLVER_THREADS 0
		#endif
	#endif
			
	#if BULLET_MAX_SOLVER_THREADS == 0
		bulletSolver = new btSequentialImpulseConstraintSolver();
	#endif
	
	return bulletCollisionConfig;
#else
	return nullptr;
#endif
}

void MGE::Physics::Physics::destroyBullet() {
#ifdef USE_BULLET
	LOG_INFO("Destroy Bullet physics system");
	if (bulletWorld) {
		clearDynamicsWorld();
		
		delete bulletWorld;
		bulletWorld = NULL;
		
		delete bulletBroadphase;
		bulletBroadphase = NULL;
	}
	
	if (bulletSolver) {
		delete bulletSolver;
		bulletSolver = NULL;
		#if BULLET_MAX_SOLVER_THREADS > 0
		delete threadSupportSolver;
		threadSupportSolver = NULL;
		#endif
	}
	
	if (bulletDispatcher) {
		delete bulletDispatcher;
		bulletDispatcher = NULL;
		#if BULLET_MAX_DISPATCHER_THREADS > 0
		delete threadSupportDispatcher;
		threadSupportDispatcher = NULL;
		#endif
	}
	
	if (bulletCollisionConfig) {
		delete bulletCollisionConfig;
		bulletCollisionConfig = NULL;
	}
	
	MGE::Engine::getPtr()->mainLoopListeners.remListener(this);
#endif
}


void MGE::Physics::Physics::configure(
		const float p1x, const float p1y, const float p1z, 
		const float p2x, const float p2y, const float p2z,
		const float gravityX, const float gravityY, const float gravityZ,
		const float rayLen
) {
	LOG_INFO("Configure physics system");
	
	MGE::WorldSizeInfo::worldMax = Ogre::Vector3(p2x, p2y, p2z);
	MGE::WorldSizeInfo::worldMin = Ogre::Vector3(p1x, p1y, p1z);
	
	if (rayLen > 0)
		MGE::WorldSizeInfo::rayLenght = rayLen;
	else
		MGE::WorldSizeInfo::rayLenght = MGE::WorldSizeInfo::worldMax.distance(MGE::WorldSizeInfo::worldMin);
	
	#ifdef USE_BULLET
	delete bulletWorld;
	delete bulletBroadphase;
	
	if (bulletCollisionConfig) {
		LOG_INFO("Configure Bullet physics system");
		
		bulletBroadphase = new btAxisSweep3(BtOgre::Convert::toBullet(MGE::WorldSizeInfo::worldMin), BtOgre::Convert::toBullet(MGE::WorldSizeInfo::worldMax));
		btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(bulletDispatcher, bulletBroadphase, bulletSolver, bulletCollisionConfig);
		world->getSimulationIslandManager()->setSplitIslands(false);
		world->getSolverInfo().m_numIterations = 4;
		world->getSolverInfo().m_solverMode = SOLVER_SIMD+SOLVER_USE_WARMSTARTING;//+SOLVER_RANDMIZE_ORDER;
		
		bulletWorld = world;
		bulletWorld->getDispatchInfo().m_enableSPU = true;
		bulletWorld->setGravity(btVector3(gravityX, gravityY, gravityZ));
		
		MGE::Engine::getPtr()->mainLoopListeners.addListener(this, PHYSICS_ACTIONS);
	}
	#endif
}

void MGE::Physics::Physics::clearDynamicsWorld(void) {
#ifdef USE_BULLET
	if (!bulletWorld)
		return;
	
	for (int i=bulletWorld->getNumCollisionObjects()-1; i>=0 ;i--) {
		btCollisionObject* obj = bulletWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		if (body && body->getCollisionShape()) {
			delete body->getCollisionShape();
		}
		bulletWorld->removeCollisionObject( obj );
		delete obj;
	}
#endif
}

bool MGE::Physics::Physics::update(float gameTimeStep, float realTimeStep) {
#ifdef USE_BULLET
	pybind11::gil_scoped_release();
	ogre2bullet.updateAll();
	
	if (bulletWorld && gameTimeStep != 0)
		bulletWorld->stepSimulation(gameTimeStep, 10);
	
	#ifdef MGE_DEBUG_PHYSICS_DRAW
	if ( debugDraw->getDebugMode() ) {
		bulletWorld->debugDrawWorld();
		debugDraw->step();
	}
	#endif
	return true;
#else
	return false;
#endif
}

btCollisionObject* MGE::Physics::Physics::createTerrainPhysics(Ogre::TerrainGroup* terrain, Ogre::Vector3 point) {
	LOG_INFO("Set physic to terrain for terrain at " << point);
	
	#if 0 /// @todo TODO.9: [Terrain] support create terrain physics
	long x,y;
	terrain->convertWorldPositionToTerrainSlot(point, &x, &y);
	
	return MGE::Physics::Physics::createPhysicsObject(
		terrain->getTerrain(x,y)
	);
	#endif
	return NULL;
}

btCollisionObject* MGE::Physics::Physics::createPhysicsObject(Ogre::Terrain * pTerrain) {
#ifdef USE_BULLET
#if 0 /// @todo TODO.9: [Terrain] support create terrain physics
	if (MGE::Physics::Physics::getPtr()->getDynamicsWorld()) {
		int terrainSideNodesNumber = pTerrain->getSize();
		float terrainLengthBetwenNodes = pTerrain->getWorldSize()/(terrainSideNodesNumber-1);
		float terrainHeightCenter = (pTerrain->getMaxHeight()-pTerrain->getMinHeight())/2.0;
		LOG_INFO( "Terrain"
			<< " sideNodesNumber="   << terrainSideNodesNumber
			<< " lengthBetwenNodes=" << terrainLengthBetwenNodes
			<< " minHeight="         << pTerrain->getMinHeight()
			<< " maxHeight="         << pTerrain->getMaxHeight()
			<< " heightCenter="      << terrainHeightCenter
			<< " position="          << pTerrain->getPosition()
		);
		
		// prepare height info for bullet based on Ogre Terrain
		float* heightfieldTerrainData= new float[terrainSideNodesNumber * terrainSideNodesNumber];
		for(int i=0; i<terrainSideNodesNumber; ++i) {
			memcpy(
				heightfieldTerrainData + terrainSideNodesNumber*i,
				pTerrain->getHeightData() + terrainSideNodesNumber * (terrainSideNodesNumber - i - 1),
				sizeof(float)*(terrainSideNodesNumber)
			);
		}
		
		// create terrain colision shape
		btHeightfieldTerrainShape* heightfieldTerrainShape = new btHeightfieldTerrainShape(
			terrainSideNodesNumber, terrainSideNodesNumber, heightfieldTerrainData, 1,
			pTerrain->getMinHeight(), pTerrain->getMaxHeight(), 1, PHY_FLOAT, true
		);
		
		heightfieldTerrainShape->setUseDiamondSubdivision(true);
		heightfieldTerrainShape->setLocalScaling(btVector3(
			terrainLengthBetwenNodes, 1, terrainLengthBetwenNodes
		));
		
		// create terrain btRigidBody
		btRigidBody* groundBody = new btRigidBody(0, new btDefaultMotionState(), heightfieldTerrainShape);
		groundBody->getWorldTransform().setOrigin( BtOgre::Convert::toBullet(
			pTerrain->getPosition() + Ogre::Vector3(0, terrainHeightCenter, 0)
		) );
		
		groundBody->setCollisionFlags(MGE::QueryFlags::GROUND);
		MGE::Physics::Physics::getPtr()->getDynamicsWorld()->addRigidBody(groundBody);
		
		return groundBody;
	}
#endif
#endif
	return NULL;
}

btCollisionObject* MGE::Physics::Physics::createPhysicsObject(
	Ogre::SceneNode* node, Ogre::MovableObject* movable,
	const std::string_view& physicsMode, const std::string_view& shapeMode,
	const std::string& shapeFile,
	const Ogre::Real& mass, int collisionFlag, int collisionMask
) {
#ifdef USE_BULLET
	LOG_INFO("createPhysicsObject for node=" << node << " and movable=" << movable <<
		" shapeMode=" << shapeMode << " shapeFile=" << shapeFile <<
		" physicsMode=" << physicsMode << " mass=" << mass <<
		" collisionFlag=" << std::hex << std::showbase << collisionFlag <<
		" collisionMask=" << std::hex << std::showbase << collisionMask
	);
	
	if (MGE::Physics::Physics::getPtr()->getDynamicsWorld()) {
		Ogre::Vector3       shapeOffset = Ogre::Vector3::ZERO;
		btCollisionShape*   shape = NULL;
		btCollisionObject*  physicsBody = NULL;
		
		if (shapeMode == "file") {
			#ifdef USE_BULLET_IMPORTER
			btBulletWorldImporter importer(NULL);
			importer.loadFile( GET_NULLEND_STRING(shapeFile) );
			
			if (importer.getNumCollisionShapes() == 1) {
				shape = importer.getCollisionShapeByIndex(0);
			} else {
				importer.deleteAllData(); // only here, in normally case we should not delete imported shape
				throw "file should have exactly one CollisionShape, have: " + Ogre::StringConverter::toString(importer.getNumCollisionShapes());
			}
			#else
				throw "shapeMode == \"file\" require build with BulletWorldImporter";
			#endif
		} else {
			BtOgre::StaticMeshToShapeConverter*  converter = NULL;
			
			if (movable->getMovableType() == Ogre::ItemFactory::FACTORY_TYPE_NAME) {
				converter = new BtOgre::StaticMeshToShapeConverter(static_cast<Ogre::Item*>(movable));
			} else if  (movable->getMovableType() == Ogre::v1::EntityFactory::FACTORY_TYPE_NAME) {
				converter = new BtOgre::StaticMeshToShapeConverter(static_cast<Ogre::v1::Entity*>(movable));
			} else {
				throw "movable in createPhysicsObject should be Ogre::Item or Ogre::v1::Entity not " + movable->getMovableType();
			}
			
			if (shapeMode == "trimesh") {
				shape = converter->createTrimesh();
			} else if (shapeMode == "convex") {
				shape = converter->createConvex();
			} else if (shapeMode == "capsule") {
				shape = converter->createCapsule();
				shapeOffset = converter->getCenterOffset();
			} else if (shapeMode == "cylinder") {
				shape = converter->createCylinder();
				shapeOffset = converter->getCenterOffset();
			} else if (shapeMode == "sphere") {
				shape = converter->createSphere();
				shapeOffset = converter->getCenterOffset();
			} else if (shapeMode == "box") {
				shape = converter->createBox();
				shapeOffset = converter->getCenterOffset();
			}
			
			LOG_INFO(" use shapeOffset = " << shapeOffset);
			delete converter;
		}
		
		// calculate Bullet position&rotation transform matrix
		btTransform offsetTransform(
			btQuaternion::getIdentity(),
			BtOgre::Convert::toBullet(shapeOffset * node->getScale())
		);
		btTransform transform(
			BtOgre::Convert::toBullet( node->getOrientation() ),
			BtOgre::Convert::toBullet( node->getPosition() )
		);
		transform *= offsetTransform;
		
		// create btRigidBody or btCollisionObject
		if (physicsMode == "full") {
			BtOgre::RigidBodyState* state;
			btVector3              inertia;
			
			// calculate inertia
			shape->calculateLocalInertia(mass, inertia);
			
			// create BtOgre MotionState (connects Ogre and Bullet).
			state = new BtOgre::RigidBodyState(node, transform, offsetTransform);
			state->setOffset(-shapeOffset * node->getScale());
			
			// create the Body
			physicsBody = new btRigidBody(mass, state, shape, inertia);
			//physicsBody->setCollisionFlags(physicsBody->getCollisionFlags() | collisionFlag | btCollisionObject::CF_KINEMATIC_OBJECT);
			
			// add the Body
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->addRigidBody( static_cast<btRigidBody*>(physicsBody), short(collisionFlag |= btCollisionObject::CF_KINEMATIC_OBJECT), short(collisionMask) );
		} else if (physicsMode == "collision") {
			physicsBody = new btCollisionObject();
			physicsBody->setCollisionShape(shape);
			physicsBody->setWorldTransform(transform);
			//physicsBody->setCollisionFlags(physicsBody->getCollisionFlags() | collisionFlag | btCollisionObject::CF_STATIC_OBJECT);
			
			// add the Body
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->addCollisionObject( physicsBody, short(collisionFlag |= btCollisionObject::CF_STATIC_OBJECT), short(collisionMask) );
		} else {
			LOG_WARNING("unknown physicsMode: " + physicsMode);
		}
		
		node->getUserObjectBindings().setUserAny( "phy", Ogre::Any( std::make_shared<Physics::AnyHolder>(physicsBody) ) );
			// wrap physicsBody in PhysicsAnyHolder to call deletePhysicsObject on it when remove
			// wrap also in shared_ptr because Ogre::Any create copies of holding object and we want call AnyHolder destructor only once (when destroy last Any holding this AnyHolder/physicsBody)
		MGE::Physics::Physics::getPtr()->ogre2bullet.addObj(physicsBody, node, shapeOffset);
		if (movable) {
			physicsBody->setUserPointer(movable);
			// do NOT add bindings to movable to prevent double deletePhysicsObject call on physicsBody
		}
		
		return physicsBody;
	}
#endif
	return NULL;
}

void MGE::Physics::Physics::deletePhysicsObject(btCollisionObject* physicsBody) {
#ifdef USE_BULLET
	if (MGE::Physics::Physics::getPtr()->getDynamicsWorld()) {
		LOG_DEBUG("delete physics object - start");
		
		if (physicsBody->getInternalType() == btCollisionObject::CO_RIGID_BODY) {
			btRigidBody* rigidBody = static_cast<btRigidBody*>(physicsBody);
			if ( rigidBody->getMotionState() )
				delete rigidBody->getMotionState();
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->removeRigidBody(rigidBody);
		} else {
			MGE::Physics::Physics::getPtr()->getDynamicsWorld()->removeCollisionObject(physicsBody);
		}
		
		btCollisionShape* physicsShape = physicsBody->getCollisionShape();
		if (physicsShape) {
			if (physicsShape->isCompound()) {
				btCompoundShape* compoundShape = static_cast<btCompoundShape*>(physicsShape);
				int numChild = compoundShape->getNumChildShapes();
				for (int i = 0; i < numChild; ++i)
					delete compoundShape->getChildShape(i);
				delete compoundShape;
			} else {
				delete physicsShape;
			}
		}
		
		MGE::Physics::Physics::getPtr()->ogre2bullet.remObj(physicsBody);
		
		delete physicsBody;
		LOG_DEBUG("delete physics object - finish");
	}
#endif
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_Bullet \<Bullet\>

@c \<Bullet\> is used for enabled Bullet physics engine. It don't have any attributes nor subnodes.
*/

MGE::Module* MGE::Physics::Physics::createBullet(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	MGE::Physics::Physics::getPtr()->createBullet(context->scnMgr);
	return nullptr;
}

/**
@page XMLSyntax_SceneConfig

@subsection XMLNode_Physics \<physics\>

@c \<physics\> is used for creating Bullet physic object, should be subnode of @ref XMLNode_Item or @ref XMLNode_Entity and has the following attributes:
  - @c physicsMode one of the following string value:
    - @c collision   create Bullet CollisionObject, only collision, don't move this object by physics
    - @c full        create Bullet RigidBody physics
  - @c shapeMode   source for object shape for physics, one of the following string value:
    - @c box         convert from Ogre mesh as box (aabb)
    - @c trimesh     convert from Ogre mesh as trimesh
    - @c convex      convert from Ogre mesh as convex
    - @c file        read from @a shapeFile file
  - @c shapeFile    file whith object shape for Bullet, used only when @a shapeMode == file
  - @c mass         mass of object, used only when @a physicsMode == full
  - @c collisionFlags  set of binary flags determining this object collision type(s), see @ref MGE::QueryFlags
  - @c collisionMask   binary mask determining types of objects with this object will collided
*/
void MGE::Physics::Physics::processPhysicsXMLNode(
	const pugi::xml_node& xmlNode,
	const MGE::LoadingContext* context,
	const MGE::SceneObjectInfo& parent
) {
#ifdef USE_BULLET
	LOG_INFO("Create physics");
	std::string_view physicsMode      = xmlNode.attribute("physicsMode").as_string();
	std::string_view shapeMode        = xmlNode.attribute("shapeMode").as_string();
	std::string shapeFile             = xmlNode.attribute("shapeFile").as_string();
	Ogre::Real         mass           = xmlNode.attribute("mass").as_float();
	int                collisionFlags = xmlNode.attribute("collisionFlags").as_int((parent.movable->getQueryFlags() != MGE::QueryFlags::OGRE_OBJECT) ? 0 : short(btBroadphaseProxy::DefaultFilter));
	int                collisionMask  = xmlNode.attribute("collisionMask").as_int(short(btBroadphaseProxy::AllFilter));
	
	btCollisionObject* phyObj = MGE::Physics::Physics::createPhysicsObject(
		parent.node, static_cast<Ogre::v1::Entity*>(parent.movable),
		physicsMode, shapeMode, shapeFile,
		mass, collisionFlags | parent.movable->getQueryFlags(), collisionMask
	);
	
	LOG_DEBUG("physics for object: " << parent.node << " is: " << phyObj);
#endif
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_WorldSize \<worldSize\>

@c \<worldSize\> is used to set world size (for raycasting, etc) and gravity force. It have subnodes:
	- @c \<min\>         3D coordinate of minimal word corner
	- @c \<max\>         3D coordinate of maximal word corner
	- @c \<gravity\>     gravity force vector
	- @c \<searchGrid\>  parameters for hexagonal grid used for 3D world pathfinding, have attributes:
		- @c size        grid size (distance between hexagon) used to init @ref MGE::HexagonalGridPoint
		- @c pathFinderLimit  number of open-nodes iteration in MGE::PathFinder::findPath
		- @c freeSpeceSearchLimit  number of iteration in @ref MGE::RayCast::findFreePosition
*/

MGE::Module* MGE::Physics::Physics::processWorldSizeXMLNode(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	Ogre::Vector3 min = MGE::XMLUtils::getValue(xmlNode.child("min"),  Ogre::Vector3::ZERO);
	Ogre::Vector3 max = MGE::XMLUtils::getValue(xmlNode.child("max"),  Ogre::Vector3::ZERO);
	Ogre::Vector3 gravity = MGE::XMLUtils::getValue(xmlNode.child("gravity"), Ogre::Vector3::ZERO);
	
	MGE::Physics::Physics::getPtr()->configure(
		min.x, min.y, min.z,
		max.x, max.y, max.z,
		gravity.x, gravity.y, gravity.z
	);
	
	MGE::HexagonalGridPoint::init(
		xmlNode.child("searchGrid").attribute("size").as_float(0.3)
	);
	MGE::PathFinder::iterationLimit = xmlNode.child("searchGrid").attribute("pathFinderLimit").as_int(MGE::PathFinder::iterationLimit);
	MGE::RayCast::defaultIterationLimit = xmlNode.child("searchGrid").attribute("freeSpeceSearchLimit").as_int(MGE::RayCast::defaultIterationLimit);
	
	#if defined(USE_BULLET) && defined(MGE_DEBUG_PHYSICS_DRAW)
	MGE::Physics::Physics::getPtr()->createDebugDraw( context->scnMgr );
	#endif
	return reinterpret_cast<MGE::Module*>(1);
}

/**
@page XMLSyntax_MapAndSceneConfig

@subsection XMLNode_Terrain \<Terrain\>

@c \<Terrain\> is used for enabled and configure terrain.
	Currently no terrain support in Ogre 2.1.
*/

MGE::Module* MGE::Physics::Physics::processTerrainXMLNode(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
#if 0  /// @todo TODO.9: [Terrain] no terrain support in Ogre >= 2.1
	LOG_INFO("Create terrain");
	
	Ogre::TerrainGlobalOptions* mTerrainGlobalOptions = OGRE_NEW Ogre::TerrainGlobalOptions();
	mTerrainGlobalOptions->setCastsDynamicShadows(true);
	
	Ogre::Light* l = scnMgr->createLight();
	l->setType(Ogre::Light::LT_DIRECTIONAL);
	l->setDiffuseColour(Ogre::ColourValue(1.0, 1.0, 1.0));
	l->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
	
	Ogre::Vector3 lightdir(0, -0.3, 0.75);
	lightdir.normalise();

	mTerrainGlobalOptions->setMaxPixelError( xmlNode.attribute("tuningMaxPixelError").as_Ogre::Real(0) );
	mTerrainGlobalOptions->setCompositeMapDistance( xmlNode.attribute("tuningCompositeMapDistance").as_Ogre::Real(0) );
	mTerrainGlobalOptions->setLightMapDirection(lightdir);
	mTerrainGlobalOptions->setCompositeMapDiffuse(l->getDiffuseColour());
	mTerrainGlobalOptions->setCompositeMapAmbient(scnMgr->getAmbientLight());
	mTerrainGlobalOptions->setQueryFlags(MGE::QueryFlags::GROUND);
	
	int mapSize = xmlNode.attribute("mapSize").as_int(0);
	float worldSize = xmlNode.attribute("worldSize").as_Ogre::Real(0);
	if (mapSize == 0 || worldSize == 0) {
		ERROR_CRASH("must be set non zero terrain mapSize and worldSize");
	}
	Ogre::TerrainGroup* terrain = OGRE_NEW Ogre::TerrainGroup(
		scnMgr, Ogre::Terrain::ALIGN_X_Z,
		mapSize, worldSize
	);
	
	terrain->setResourceGroup(
		xmlNode.attribute("resourceGroup").as_string()
	);
	
	MGE::Utils::XMLNode* xmlTagNode = xmlNode->first_node("origin");
	if(xmlTagNode) {
		LOG_INFO("Set terrain origin");
		terrain->setOrigin(MGE::XMLUtils::getValue<Ogre::Vector3>(xmlTagNode));
		LOG_SUBINFO_STREAM("terrain origin is: " << terrain->getOrigin() );
	} else {
		LOG_INFO("Not set terrain origin");
		terrain->setOrigin(Ogre::Vector3::ZERO);
	}
	
	// Process terrain pages (*)
	xmlTagNode = xmlNode->first_node("terrainPages");
	if(xmlTagNode) {
		MGE::Utils::XMLNode* xmlPageNode = xmlTagNode->first_node("terrainPage");
		while(xmlPageNode)
		{
			std::string name = xmlPageNode.attribute("name").as_string();
			int pageX = xmlPageNode.attribute("pageX").as_int(0);
			int pageY = xmlPageNode.attribute("pageY").as_int(0);
			
			if (Ogre::ResourceGroupManager::getSingleton().resourceExists(terrain->getResourceGroup(), name)) {
				terrain->defineTerrain(pageX, pageY, name);
				LOG_INFO("Add page: " + name);
			} else {
				LOG_WARNING("Skip (resource not exists) page: " + name);
			}
			
			xmlPageNode = xmlPageNode->next_sibling("terrainPage");
		}
	}
	
	LOG_INFO("LoadAllTerrains");
	terrain->loadAllTerrains(true);
	
	LOG_INFO("Free temporary resources");
	terrain->freeTemporaryResources();
	
	#ifdef USE_BULLET
	// Create terrain physics
	if ( xmlNode.attribute("hasPhysics").as_bool(false) ) {
		MGE::Physics::Physics::createTerrainPhysics(
			terrain,
			Ogre::Vector3(0,0,0)
		);
	}
	#endif
	
	// set pointer to created terrain in raycast system
	MGE::Physics::Physics::getPtr()->setTerrain(terrain);
	
	// write to log some info about terrain
	Ogre::Terrain* tmpT = terrain->getTerrain(0, 0);
	LOG_INFO( "terrain(0,0):"
		<< " position=" << tmpT->getPosition()
		<< " heightData=" << *(tmpT->getHeightData())
		<< " queryFlags=" << tmpT->getQueryFlags()
		<< " visibilityFlags=" << tmpT->getVisibilityFlags()
	);
#endif
	return nullptr;
}

#if defined(USE_BULLET) && defined(MGE_DEBUG_PHYSICS_DRAW)
void MGE::Physics::Physics::createDebugDraw(Ogre::SceneManager* scnMgr) {
	if(bulletWorld) {
		LOG_INFO("Creating Bullet Debug Draw");
		debugDraw = new BtOgre::DebugDrawer(scnMgr->getRootSceneNode(), bulletWorld, scnMgr->getName());
		debugDraw->setDebugMode(false);
		bulletWorld->setDebugDrawer(debugDraw);
	}
}

void MGE::Physics::Physics::setDebugMode(bool enabled) {
	debugDraw->setDebugMode(enabled);
	debugDraw->step();
}
#endif
