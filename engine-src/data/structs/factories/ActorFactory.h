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

#include "data/structs/BaseActor.h"

#include <OgreVector3.h>
#include <OgreNameGenerator.h>
#include <unordered_map>

namespace MGE { struct LoadingContext; struct SceneObjectInfo; }

namespace MGE {

/// @addtogroup WorldStruct
/// @{
/// @file

/**
 * @brief Factory for game objects (actors).
 * 
 * It's three way to create actor:
 *   - by reading actor config from .scene xml file (see @ref MGE::ActorFactory::processActorXMLNode)
 *   - by calling @ref MGE::ActorFactory::createActor from code with correct actor prototype and unique name
 *   - by restoring it from xml save file (see @ref MGE::ActorFactory::restore)
 * 
 * \par Restoring from save file
 * On save restore we create new actor only when: in current scene do not exist actor with the same name,
 * or existing actor use other prototype (then recreate it by calling @ref MGE::ActorFactory::reCreateActor).
 * 
 * \par Actor prototypes
 * Typically, actors are based on prototypes, but you can create actor without prototype (with NULL prototype).
 * 
 * \par 
 * Actor prototype is MGE::BasePrototype based object identified by @ref MGE::ResourceLocationInfo
 * (prototype name + file name + file group name). Prototype can have own (read-only) properties, etc, which actor can refer to.
 * Prototype config node (used to identified protype) is parsing every time you create an actor (with not NULL prototype)
 * and is directly used to creating actor (typically for creating 3d objects and components set).
 * 
 * \par
 * Local config of actor (from .scene or save file) is proccessed @b after proccessed actor prototype config node,
 * so can supplement / override some of prototype setting (but not all -- can't destroy object created by prototype config).
 * 
 * \par Use MGE::BaseActor::restore() function
 * Save restoring call @ref MGE::BaseActor::restore() with xml actor node from save on every actor
 * (previously existing, recreated and created) @b after completed restoring @ref MGE::ActorFactory::allActors
 * (when all actors are created with correct prototype and actors taht do not exist in save file are destroyed).
 * 
 * \par 
 * Loading from .scene xml file call @ref MGE::BaseActor::restore() with xml actor node from .scene file
 * immediately after creating actor.
 * Therefore, in .scene file xml actors node you can't use syntax elements that refernce to other actors),
 * if you need this you must put it in .state (fake save) file loading after .scene file
 * (see @ref MGE::LoadingSystem and @ref XMLSyntax_MapConfig for more detail).
 */
struct ActorFactory :
	public MGE::SaveableToXML<ActorFactory>,
	public MGE::Singleton<ActorFactory>
{
public:
	/**
	 * @brief return Actor identifying by @a name (if not exist return NULL)
	 * 
	 * @param name name of object to get
	 */
	inline MGE::BaseActor* getActor(
		const std::string_view& name
	) {
		auto iter = allActors.find( name );
		if (iter != allActors.end()) {
			return iter->second;
		} else {
			return NULL;
		}
	}
	
	/**
	 * @brief find actors in @a range from @a point
	 *
	 * @param[in]  point   center of search circle
	 * @param[in]  range   radius of search circle
	 * @param[out] results map (radius square → actor) with found actors
	 */
	void findActors(const Ogre::Vector3& point, float range, std::multimap<float, MGE::BaseActor*>* results);
	
	/**
	 * @brief find actors in @a range from @a point
	 *
	 * @param[in]  point   center of search circle
	 * @param[in]  range   radius of search circle
	 * 
	 * @return map (radius square → actor) with found actors
	 */
	inline std::multimap<float, MGE::BaseActor*> findActors(const Ogre::Vector3& point, float range) {
		std::multimap<float, MGE::BaseActor*> ret;
		findActors(point, range, &ret);
		return ret;
	}
	
	/**
	 * @brief create Actor identifying by @a name based on @a prototype
	 * 
	 * @param prototype  pointer to prototype using to create object
	 * @param name       name of object to create
	 * @param position   position of new object
	 * @param rotation   orientation of new object
	 * @param findFreePositionOnGround  when true find free position on ground near @a position and puit object there
	 * @param scnMgr     pointer to scene menager used to create object (when NULL use default)
	 */
	MGE::BaseActor* createActor(
		const MGE::BasePrototype* prototype,
		Ogre::String name,
		Ogre::Vector3 position = Ogre::Vector3::ZERO,
		const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
		bool findFreePositionOnGround = true,
		Ogre::SceneManager* scnMgr = NULL
	);
	
	/**
	 * @brief delete Actor @a obj
	 *
	 * @param obj             pointer to Actor object to delete
	 * @param deleteSceneNode when true recursive delete SceneNode
	 */
	void destroyActor(MGE::BaseActor* obj, bool deleteSceneNode = true);
	
	/**
	 * @brief re-create Actor identifying by @a name based on @a prototype
	 * 
	 * @param actor       actor to replace
	 * @param prototype   prototype for new actor
	 */
	MGE::BaseActor* reCreateActor(
		MGE::BaseActor*           actor,
		const MGE::BasePrototype* prototype
	);
	
	/**
	 * @brief parse Actor xml node
	 * 
	 * Implementation of @ref MGE::SceneLoader::SceneNodesCreateFunction.
	 */
	static MGE::BaseActor* processActorXMLNode(
		const pugi::xml_node&       xmlNode,
		const MGE::LoadingContext*  context,
		const MGE::SceneObjectInfo& parent
	);
	
	/// list of all scene objects (as map name -> object pointer)
	std::unordered_map<std::string, MGE::BaseActor*, MGE::string_hash, std::equal_to<>>   allActors;
	
	/// Name of XML tag for @ref MGE::SaveableToXML::getXMLTagName.
	inline static const char* xmlStoreRestoreTagName = "Actors";
	
	/// @copydoc MGE::SaveableToXMLInterface::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) override;
	
	/// @copydoc MGE::SaveableToXMLInterface::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const override;
	
	/// @copydoc MGE::UnloadableInterface::unload
	virtual bool unload() override;
	
	/// constructor ... register listeners
	ActorFactory();
	
protected:
	/// createObject Actor based on @a prototype and @a node
	MGE::BaseActor* _createActor(
		const MGE::BasePrototype* prototype,
		Ogre::SceneNode* node
	);
	
protected:
	/// destructor ... unregister listeners
	~ActorFactory();
};


/// @}

}
