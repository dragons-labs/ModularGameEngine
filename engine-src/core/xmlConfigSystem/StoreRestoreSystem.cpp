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

#include "StoreRestoreSystem.h"

#include "LogSystem.h"
#include "StringUtils.h"

#include <pugixml.hpp>

const std::string_view MGE::SaveableToXMLInterface::getXMLTagName() const {
	return MGE::EMPTY_STRING_VIEW;
}

void MGE::StoreRestoreSystem::storeToXML(pugi::xml_node& xmlNode, bool onlyRef) {
	for (auto&& [tagName, listener] : saveListeners.listeners) {
		auto xmlStoreNode = xmlNode.append_child( listener->getXMLTagName().data() ); // string_view returned by getXMLTagName() is NULL-end
		listener->storeToXML( xmlStoreNode, onlyRef );
	}
}

void MGE::StoreRestoreSystem::unload() {
	unloadListeners.callAll( &MGE::UnloadableInterface::unload );
}

void MGE::StoreRestoreSystem::restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) {
	for (const auto& xmlSubNode : xmlNode) {
		std::string_view xmlSubNodeName = xmlSubNode.name();
		
		LOG_VERBOSE("RestoreFromXML", "parse tag: " << xmlSubNodeName);
		
		auto range = MGE::Range(restoreListeners.listeners, xmlSubNodeName);
		for (auto&& [tagName, listener] : range) {
			(listener->restoreFromXML)(xmlSubNode, context);
		}
		if (range.begin() == restoreListeners.listeners.end()) {
			LOG_ERROR("RestoreFromXML", "ignoring unregistered tag: " << xmlSubNodeName);
		}
	}
}


bool MGE::StoreRestoreSystem::addSaveListener(MGE::SaveableToXMLInterface* obj, int saveKey) {
	return saveListeners.addListener(obj, saveKey) && restoreListeners.addListener(obj, obj->getXMLTagName());
}

void MGE::StoreRestoreSystem::remSaveListener(MGE::SaveableToXMLInterface* obj) {
	saveListeners.remListener(obj);
	restoreListeners.remListener(obj);
}

bool MGE::StoreRestoreSystem::addUnloadListener(MGE::UnloadableInterface* obj, int unloadKey) {
	return unloadListeners.addListener(obj, unloadKey);
}

void MGE::StoreRestoreSystem::remUnloadListener(MGE::UnloadableInterface* obj) {
	unloadListeners.remListener(obj);
}
