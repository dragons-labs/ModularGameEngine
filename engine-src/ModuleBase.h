/*
Copyright (c) 2022-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

#include "Engine.h"
#include "EngineModule.h"
#include "StoreRestoreSystem.h"

namespace MGE {

/// @addtogroup EngineMain
/// @{
/// @file

/**
 * @brief Base class for removable / unloadable objects, with auto registration and unregistration in @ref MGE::StoreRestoreSystem.
 */
struct Unloadable : public MGE::UnloadableInterface {
	/// @copydoc MGE::UnloadableInterface::unload
	///
	/// @remark This (Unloadable class) implementation simply call destructor on object.
	virtual bool unload() override {
		delete this;
		return true;
	}
	
	/**
	 * @brief Constructor - register from @ref MGE::StoreRestoreSystem::unloadListeners.
	 * 
	 * @param unloadKey  Key for unload listener (determinate order of unload operations).
	 */
	Unloadable(int unloadKey) {
		MGE::Engine::getPtr()->getStoreRestoreSystem()->unloadListeners.addListener(this, unloadKey);
	}
	
	/**
	 * @brief Destructor - unregister from @ref MGE::StoreRestoreSystem::unloadListeners.
	 */
	virtual ~Unloadable() {
		MGE::Engine::getPtr()->getStoreRestoreSystem()->unloadListeners.remListener(this);
	}
};

/**
 * @brief Base (interface) class for XML storage objects, with auto registration and unregistration in @ref MGE::StoreRestoreSystem.
 * 
 * @note  Derived class must provide @a xmlStoreRestoreTagName static string (e.g. as `inline static const char* xmlStoreRestoreTagName = "MyTag";`)
 *        with XML tag name for use as return value in @ref getXMLTagName.
 *        If not, should derived by `SaveableToXML<void>` and override @ref getXMLTagName virtual member function.
 */
template <typename CLASS> struct SaveableToXML : public SaveableToXMLInterface, public Unloadable {
	/// @copydoc MGE::SaveableToXMLInterface::getXMLTagName
	virtual const std::string_view getXMLTagName() const override {
		if constexpr (std::is_same_v<CLASS, void>) {
			return "INVALID TAG NAME";
		} else {
			return CLASS::xmlStoreRestoreTagName;
		}
	}
	
	/**
	 * @brief Constructor - register from @ref MGE::StoreRestoreSystem::saveListeners.
	 * 
	 * @param saveKey    Key for save listener (determinate order of save operations).
	 * @param unloadKey  Key for unload listener (determinate order of unload operations).
	 */
	SaveableToXML(int saveKey, int unloadKey) :
		Unloadable(unloadKey)
	{
		MGE::Engine::getPtr()->getStoreRestoreSystem()->saveListeners.addListener(this, saveKey);
		MGE::Engine::getPtr()->getStoreRestoreSystem()->restoreListeners.addListener(this, getXMLTagName());
	}
	
	/**
	 * @brief Destructor - unregister from @ref MGE::StoreRestoreSystem::restoreListeners.
	 */
	virtual ~SaveableToXML() {
		MGE::Engine::getPtr()->getStoreRestoreSystem()->saveListeners.remListener(this);
		MGE::Engine::getPtr()->getStoreRestoreSystem()->restoreListeners.remListener(this);
	}
};

/// @}

}
