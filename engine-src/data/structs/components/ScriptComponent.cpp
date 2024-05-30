/*
Copyright (c) 2018-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "data/structs/components/ScriptComponent.h"

#include "data/structs/BaseActor.h"
#include "data/structs/factories/ComponentFactory.h"
#include "Engine.h"

MGE::ScriptComponent::~ScriptComponent() {
	MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
	delete pythonObject;
}

MGE::BaseComponent* MGE::ScriptComponent::create(MGE::NamedObject* parent, const pugi::xml_node& config, std::set<int>* typeIDs, int createdForID) {
	if (!config)
		throw std::logic_error("Can't create ScriptComponent without XML config");
	
	typeIDs->insert(createdForID);
	return new MGE::ScriptComponent(parent, config, createdForID);
}

void MGE::ScriptComponent::setup(int typeID, const std::string& className) {
	MGE::ComponentFactory::getPtr()->registerComponent(
		typeID, className, MGE::ScriptComponent::create
	);
}

// no auto registration via MGE_ACTOR_COMPONENT_CREATOR / MGE_REGISTER_ACTOR_COMPONENT because this is "component template"
// creating real component need provide typeID and className arguments to MGE::ScriptComponent::setup


bool MGE::ScriptComponent::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const {
	if ( !pythonObject )
		// we don't need write createPythonObject attribute due to using needInit
		return false;
	
	std::string toStore = MGE::ScriptsSystem::getPtr()->runObjectWithCast<std::string>(
		(MGE::ComponentFactory::getPtr()->getName(scriptClassID) + ".store").c_str(),
		MGE::EMPTY_STRING,
		*pythonObject
	);
	xmlNode.text().set(toStore.data(), toStore.length());
	return true;
}

/**
@page XMLSyntax_ActorComponent

@subsection ActorComponent_ScriptComponent ScriptComponent

The syntax depends on value of @a scriptClassID attribute.

When creating new ScriptComponent (@ref pythonObject was not created), will be called constructor of Python class indicated by @a scriptClassID value.
Pointer to owner BaseActor and text representation of @c \<Component\> XML node will be passed to it.

When @ref pythonObject was created (call restore on existed ScriptComponent), will be called restore method on it.
Text representation of @c \<Component\> XML node will be passed to it.

Optional attribute @c createPythonObject can be set to false to avoid creating (and use) python object.
This can be useful on class derived from ScriptComponent.
*/

MGE::ScriptComponent::ScriptComponent(MGE::NamedObject* parent, const pugi::xml_node& xmlNode, int createdForID) :
	scriptClassID( createdForID )
{
	std::string className( MGE::ComponentFactory::getPtr()->getName(scriptClassID) );
	
	LOG_INFO("create ScriptComponent for " + className);
	
	if ( xmlNode.attribute("createPythonObject").as_bool(true) ) {
		MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
		pythonObject = new pybind11::object(
			MGE::ScriptsSystem::getPtr()->runObjectThrow(
				className.c_str(),
				pybind11::cast(static_cast<MGE::BaseActor*>(parent), pybind11::return_value_policy::reference),
				MGE::XMLUtils::nodeAsString(xmlNode)
			)
		);
	} else {
		pythonObject = nullptr;
	}
}

bool MGE::ScriptComponent::restoreFromXML(
	const pugi::xml_node& xmlNode, MGE::NamedObject* parent, Ogre::SceneNode* sceneNode
) {
	if ( pythonObject ) {
		std::string className( MGE::ComponentFactory::getPtr()->getName(scriptClassID) );
		
		LOG_INFO("restore ScriptComponent for " + className);
		
		MGE::ScriptsSystem::getPtr()->runObjectWithVoid(
			(className + ".restore").c_str(), *pythonObject,
			MGE::XMLUtils::nodeAsString(xmlNode)
		);
	}
	return true;
}

void MGE::ScriptComponent::init(MGE::NamedObject* parent) {
	if ( pythonObject ) {
		std::string className( MGE::ComponentFactory::getPtr()->getName(scriptClassID) );
		
		LOG_INFO("restore ScriptComponent for " + className);
		
		MGE::ScriptsSystem::getPtr()->runObjectWithVoid(
			(className + ".init").c_str(), pythonObject
		);
	}
}
