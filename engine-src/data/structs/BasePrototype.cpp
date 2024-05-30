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

#include "data/structs/BasePrototype.h"

#include "Engine.h"
#include "data/structs/factories/ComponentFactory.h"
#include "data/utils/OgreResources.h"
#include "data/property/G11n.h"

#include "LogSystem.h"
#include "XmlUtils.h"

const std::string& MGE::BasePrototypeImpl::getType() const {
	return MGE::BasePrototype::TypeName();
}

const std::string& MGE::BasePrototypeImpl::getName() const {
	return config.name;
}

pugi::xml_node MGE::BasePrototype::getPrototypeXML(
	const MGE::ResourceLocationInfo* config,
	pugi::xml_document& xmlDoc
) {
	// get config xml file paths
	std::list<std::string> pathList;
	MGE::OgreResources::getResourcePaths(config->fileName, config->fileGroup, &pathList, false, "Prototypes");
	
	// for all matching paths:
	for (auto& path : pathList) {
		// search for ActorPrototype node with correct name
		auto xmlNode = MGE::XMLUtils::openXMLFile(xmlDoc, path.c_str(), "Prototypes");
		if (xmlNode) {
			for (auto xmlSubNode : xmlNode.children("ActorPrototype")) {
				if (config->name == xmlSubNode.attribute("name").as_string()) {
					LOG_VERBOSE("found <ActorPrototype> with name=" + config->name + " in file: " + path);
					return xmlSubNode;
				}
			}
		} else {
			LOG_ERROR("can't find <Prototypes> in file: " + path);
		}
	}
	LOG_ERROR("can't find <ActorPrototype> with name=" + config->name + " in file: " + config->fileName + " in group: " + config->fileGroup);
	return MGE::EMPTY_XML_NODE;
}

MGE::BasePrototypeImpl::BasePrototypeImpl(
	const std::string& _name, const std::string& _fileName, const std::string& _fileGroup
) :
	config(_name, _fileName, _fileGroup)
{
	LOG_INFO("Creating prototype " << config.name << " from file: " << config.fileName << " in group: " << config.fileGroup);
	pugi::xml_document xmlDoc;
	pugi::xml_node     xmlNode = getPrototypeXML(&config, xmlDoc);
	
	// restore from XML config
	if (xmlNode) {
		restoreFromXML(xmlNode, nullptr);
	} else {
		throw std::logic_error("can't find config for: name=" + config.name + " in file: " + config.fileName + " in group: " + config.fileGroup);
	}
	
	LOG_INFO("Prototype " << config.name << " created");
}

MGE::BasePrototypeImpl::~BasePrototypeImpl() { }


const MGE::Any& MGE::BasePrototypeImpl::getProperty(const std::string_view& key) const {
	return properties.getProperty(key);
}

[[ noreturn ]] size_t MGE::BasePrototypeImpl::remProperty(const std::string_view& key) {
	throw std::logic_error("can't modify property on GameObjectPrototype");
}

[[ noreturn ]] bool MGE::BasePrototypeImpl::addProperty(const std::string_view& key, const MGE::Any& val, bool replace) {
	throw std::logic_error("can't modify property on GameObjectPrototype");
}

[[ noreturn ]] bool MGE::BasePrototypeImpl::setProperty(const std::string_view& key, const MGE::Any& val) {
	throw std::logic_error("can't modify property on GameObjectPrototype");
}

const MGE::ResourceLocationInfo* MGE::BasePrototypeImpl::getLocationInfo() const {
	return& config;
}

bool MGE::BasePrototypeImpl::storeToXML(pugi::xml_node& xmlNode, bool /*onlyRef*/) const {
	config.storeToXML(xmlNode);
	return true;
}

bool MGE::BasePrototypeImpl::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* /*context*/) {
	LOG_INFO("restore/load GameObjectPrototype: " + config.name + " from: " + config.fileName + " in: " + config.fileGroup);
	
	properties.clearAll();
	properties.restoreFromXML(xmlNode, MGE::G11n::getLang(), true);
	
	MGE::ComponentFactory::getPtr()->restoreComponents(xmlNode, &components, this, NULL);
	
	LOG_INFO("GameObjectPrototype: " << config.name << " successful loaded from: " << config.fileName << " in: " << config.fileGroup);
	return true;
}

const MGE::BaseComponent* MGE::BasePrototypeImpl::getComponent(int typeID) const {
	auto iter = components.find(typeID);
	if (iter != components.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

MGE::BaseComponent* MGE::BasePrototypeImpl::getComponent(int typeID, int classID) {
	auto iter = components.find(typeID);
	if (iter != components.end()) {
		return iter->second;
	} else if (classID != 0) {
		return MGE::ComponentFactory::getPtr()->createComponent(classID, &components, this);
	} else {
		return NULL;
	}
}
