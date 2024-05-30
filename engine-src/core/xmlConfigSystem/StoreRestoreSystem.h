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

#include "force_inline.h"
#include "ListenerSet.h"
#include "BaseClasses.h"

#include <string_view>

namespace pugi { class xml_node; }

namespace MGE { struct LoadingContext; }

namespace MGE {

/// @addtogroup XMLConfigSystem
/// @{
/// @file

/**
 * @brief Base (interface) class for XML storage objects.
 */
struct SaveableToXMLInterface {
	/**
	 * @brief Store object state to XML.
	 * 
	 * @param xmlNode      XML node to store this object state.
	 * @param onlyRef      If true and supported by storing object, then store only reference to object (name, config source, etc). 
	 * 
	 * @note
	 *        Function can add attributes, text content or (multiple/single) child nodes to @a xmlNode.
	 *        So @a xmlNode should be used only for this object (should be dedicated child).
	 * 
	 * @note
	 *        Restoring (@ref restoreFromXML) can support more syntax variant than storing (@ref storeToXML),
	 *        but must support current storing syntax.
	 */
	virtual bool storeToXML(pugi::xml_node& xmlNode, bool onlyRef) const = 0;
	
	/**
	 * @brief Store object state to XML – rvalue reference version of @ref storeToXML.
	 */
	FORCE_INLINE bool storeToXML(pugi::xml_node&& xmlNode, bool onlyRef) const {
		return storeToXML(xmlNode, onlyRef);
	};
	
	/**
	 * @brief Write to XML operator - simply call @ref storeToXML.
	 */
	friend FORCE_INLINE pugi::xml_node& operator<<(pugi::xml_node& xmlNode, const SaveableToXMLInterface& val) {
		val.storeToXML(xmlNode, false);
		return xmlNode;
	}
	
	/**
	 * @brief Load / restore object state from XML.
	 * 
	 * @param xmlNode    XML node with data to restore this object state.
	 * @param context    Structure with info about restoring/loading context.
	 * 
	 * @note
	 *        @a xmlNode is root node for this object.
	 *        So this function can restore values from it atributes, text content or (multiple/single) child nodes.
	 *        @a xmlNode passed to this function is on the same level of XML DOM as @a xmlNode of @ref storeToXML.
	 * 
	 * @note
	 *        Restoring (@ref restoreFromXML) can support more syntax variant than storing (@ref storeToXML),
	 *        but must support current storing syntax.
	 */
	virtual bool restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context) = 0;
	
	/**
	 * @brief Return "external" XML node name for store/restore operation via @ref MGE::StoreRestoreSystem listeners.
	 * 
	 * @attention
	 *          Returned value must be string_view to NULL end buffer!
	 * 
	 * @remark  This is necessarily, because @ref storeToXML receive "dedicated child" for storing single object.
	 *          So this child must be created externally (in case using @ref MGE::StoreRestoreSystem listeners by this system),
	 *          but for create it we need name (and the same name must be used as key in restore listener).
	 */
	virtual const std::string_view getXMLTagName() const;
	
	/**
	 * @brief Virtual destructor - do nothing.
	 */
	virtual ~SaveableToXMLInterface() = default;
};

/**
 * @brief Base (interface) class for removable / unloadable objects.
 */
struct UnloadableInterface {
	/**
	 * @brief Unload (delete) object or reset state of (permanently existing) object to default.
	 */
	virtual bool unload() = 0;
	
	/// virtual destructor - do nothing
	virtual ~UnloadableInterface() = default;
};

/**
 * @brief XML based store and restore system.
 * 
 * @remark  Typically listeners are auto registration by creating instances of @ref SaveableToXML @ref Unloadable derived classes (in constructors).
 *          @ref SaveableToXMLInterface and @ref UnloadableInterface clases can be used for manual registered listeners in @ref MGE::StoreRestoreSystem.
 *          Can be used also for some objects stored / restored by its parent systems,
 *          but for some objects serviced by parent systems will be needed diferent set of arguments for storeToXML / restoreFromXML function.
 *          Despite this, for API consistency, is recommended use storeToXML / restoreFromXML names for those function and define operator << calling storeToXML.
 */
class StoreRestoreSystem {
public:
	/**
	 * @brief Call storeToXML on all registered @ref saveListeners.
	 * 
	 * @param xmlNode      XML node to store state of all registered objects.
	 * @param onlyRef      If true and supported by storing object, then store only reference to object (name, config source, etc).
	 *                     Pass thru to listener function.
	 */
	void storeToXML(pugi::xml_node& xmlNode, bool onlyRef = false);
	
	/**
	 * @brief Call unload on all registered @ref unloadListeners.
	 */
	void unload();
	
	/**
	 * @brief Process XML save node @a xmlNode by calling corresponding (tag name == key) function from @ref restoreListeners on it sub-nodes.
	 * 
	 * @param xmlNode    XML node with save data to restore.
	 * @param context    Structure with info about restoring/loading context.
	 */
	void restoreFromXML(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context = nullptr);
	
	/**
	 * @brief Add (register) listener in @ref saveListeners and @ref restoreListeners.
	 * 
	 * @param obj        Listener class object.
	 * @param saveKey    Key for save listener (determinate order of save operations).
	 */
	bool addSaveListener(MGE::SaveableToXMLInterface* obj, int saveKey);
	
	/**
	 * @brief Remove (unregister) listener from @ref saveListeners and @ref restoreListeners.
	 * 
	 * @param obj        Listener class object.
	 */
	void remSaveListener(MGE::SaveableToXMLInterface* obj);
	
	/**
	 * @brief Add (register) listener in @ref unloadListeners.
	 * 
	 * @param obj        Listener class object.
	 * @param unloadKey  Key for unload listener (determinate order of unload operations).
	 */
	bool addUnloadListener(MGE::UnloadableInterface* obj, int unloadKey);
	
	/**
	 * @brief Remove (unregister) listener from @ref unloadListeners.
	 * 
	 * @param obj        Listener class object.
	 */
	void remUnloadListener(MGE::UnloadableInterface* obj);
	
	
protected:
	template <typename CLASS> friend struct SaveableToXML;
	friend struct Unloadable;
	
	/// set for write save listeners
	MGE::ClassPtrListenerSet<MGE::SaveableToXMLInterface, int>                             saveListeners;
	/// set for load save listeners
	MGE::ClassPtrListenerSet<MGE::SaveableToXMLInterface, std::string_view, std::string>   restoreListeners;
	/// set for unload listeners
	MGE::ClassPtrListenerSet<MGE::UnloadableInterface, int>                                unloadListeners;
};

/// @}
}
