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

#include "MainLoopListener.h"
#include "ModuleBase.h"

#include "data/structs/BaseComponent.h"

namespace MGE { struct BaseActor; }
namespace MGE { struct FlammableObject; }

namespace MGE {

/// @addtogroup Game
/// @{
/// @file

/**
 * @brief class for processing fires
 */
struct FireSubSystem :
	public MGE::Module,
	public MGE::MainLoopListener,
	public MGE::Unloadable,
	public MGE::Singleton<FireSubSystem>
{
	/// @copydoc MGE::MainLoopListener::update
	bool update(float gameTimeStep, float realTimeStep) override;
	
	/// list of burning object
	std::set<MGE::FlammableObject*>    objectsOnFire;
	
	/// @copydoc MGE::SaveableToXML::unload
	virtual bool unload() override;
	
	FireSubSystem();
};

/**
 * @brief class for flammable object
 */
struct FlammableObject :
	public MGE::BaseComponent
{
	/// when true object is flammable (can be on fire)
	bool  isFlammable;
	/// flash point temperature
	float flashPoint;
	/// maximum temperature of fire
	float fireTemperature;
	/// explosion initialization temperature
	float explosionPoint;
	
	/// when true object is on fire
	bool  isOnFire;
	/// current fuel level
	float fuelLevel;
	/// current temperature
	float temperature;
	/// time to explosion
	float timeToExplosion;
	/// cooling efficiency factor
	float coolingEfficiency;
	
	/// update fire status
	void process(float gameTimeStep);
	
	/**
	 * @brief set fire state of object
	 * 
	 * @param state - fire state to set, possible values:
	 *        @li -1 = burned (no fuel)
	 *        @li  0 = not on fire, but possible ready to start fire (normal flammable of object)
	 *        @li  1 = init fire
	 *        @li  2 = on (full) fire
	 */
	void setFire(int state);
	
	/// @copydoc MGE::BaseObject::storeToXML
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false) const override;
	
	/// @copydoc MGE::BaseComponent::restoreFromXML
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode) override;
	
	/// numeric ID of primary type implemented by this MGE::BaseComponent derived class
	/// (aka numeric ID of this MGE::BaseComponent derived class, must be unique)
	inline static const int classID = 10;
	
	/// @copydoc MGE::BaseComponent::provideTypeID
	virtual bool provideTypeID(int id) const override {
		return id == classID;
	}
	
	/// @copydoc MGE::BaseComponent::getClassID
	virtual int getClassID() const override {
		return classID;
	}
	
	/// static function for register in MGE::ComponentFactory
	static MGE::BaseComponent* create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID);
	
protected:
	/// constructor
	FlammableObject(MGE::NamedObject* parent);
	
	/// destructor
	virtual ~FlammableObject();
	
private:
	/// pointer to "parent" actor
	MGE::BaseActor* owner;
	
	inline void setOnFire() {
		isOnFire = true;
		MGE::FireSubSystem::getPtr()->objectsOnFire.insert(this);
	}
	
	inline void unsetOnFire() {
		isOnFire = false;
		MGE::FireSubSystem::getPtr()->objectsOnFire.erase(this);
	}
	template <typename Archive> inline void store_restore(Archive& xmlArch);
};

/// @}

}
