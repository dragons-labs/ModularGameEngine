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

#include "pragma.h"
#include "ListenerSet.h"
#include "BaseClasses.h"

#include <string>
#include <string_view>

namespace pugi { class xml_node; }

namespace MGE { struct LoadingContext; struct SceneObjectInfo; }

namespace MGE {

/// @addtogroup XMLConfigSystem
/// @{
/// @file

/**
 * @brief System for parsing XML scene configuration.
 * 
 * Support static (@ref MGE_REGISTER_SCENE_ELEMENT macro) and manual (call @ref addSceneNodesCreateListener member function) registration for XML tag parser function.
 * 
 * @remark
 *    (Trivial) Singleton, for support auto registration in @ref sceneNodesCreateListeners.
 */
class SceneLoader : public MGE::TrivialSingleton<SceneLoader> {
public:
	/**
	 * @brief Type of static function for register in @ref sceneNodesCreateListeners, for description of args see @ref parseSceneXMLNode.
	 * 
	 * @remark  We use const reference as @a parent, so function can modify information about parent.
	 *          But MGE::SceneObjectInfo is set non-const pointers, so function can modify parent object using those pointers.
	 */
	typedef void* (*SceneNodesCreateFunction)(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent);
	
	/**
	 * @brief Add (register) listener in @ref sceneNodesCreateListeners.
	 */
	bool addSceneNodesCreateListener(const std::string_view& tagName, SceneNodesCreateFunction callbackFunction);
	
	/**
	 * @brief Remove (unregister) listener from @ref sceneNodesCreateListeners.
	 */
	void remSceneNodesCreateListener(SceneNodesCreateFunction callbackFunction);
	
	/**
	 * @brief Parse all registered in @ref sceneNodesCreateListeners sub tags of @a xmlNode.
	 *
	 * @param xmlNode      XML node to parse.
	 * @param context      Structure with info about restoring/loading context.
	 * @param parent       Structure with info about parent.
	 * 
	 * @remark  This function can be call recursive - by calling from function registred in sceneNodesCreateListeners.
	 */
	void parseSceneXMLNode(
		const pugi::xml_node&         xmlNode,
		const MGE::LoadingContext*    context,
		const MGE::SceneObjectInfo&   parent
	);
	
	/**
	 * @brief Print to log all registered XML tag names.
	 */
	void listListeners();
	
protected:
	/**
	 * @brief Listener set for static function used for processing XML nodes with scene elements.
	 *        These functions will be call with corresponding XML node (key == tag name) while processing
	 *        sub-nodes of \<nodes\> in .scene file and its childs.
	 * 
	 * @note  In case when @a SceneNodesCreateFunction is used for creating new C++ objects,
	 *        these objects <b>must</b> derive from @ref MGE::Unloadable or @ref MGE::SaveableToXML to use store/restore system for unloading.
	 */
	MGE::FunctionListenerSet<SceneNodesCreateFunction, std::string_view, std::string> sceneNodesCreateListeners;
};
/// @}
}


////////////////////////////   Preprocessor Macros   ////////////////////////////

/// @addtogroup XMLConfigSystem
/// @{
/// @file

/**
 * @brief Register @a FUNCTION function in @ref MGE::SceneLoader::sceneNodesCreateListeners to be call for process XML tag @a TAGNAME in scene config.
 * 
 * @param TAGNAME   Name of XML node in configuration files to call @FUNCTION.
 * @param FUNCTION  Function (@ref MGE::SceneLoader::SceneNodesCreateFunction) to register.
 *                  Optional argument – when not provide using @c create__<i>TAGNAME</i>.
 * @param VARIABLE  Variable created and used for static auto registration.
 *                  Will be created in @c MGE::SceneLoaderListenerRegistration namespace.
 *                  Optional argument – when not provide using @c isRegistred__<i>TAGNAME</i>.
 */
#ifdef __DOCUMENTATION_GENERATOR__
#define MGE_REGISTER_SCENE_ELEMENT(TAGNAME, FUNCTION, VARIABLE)
#else
#define MGE_REGISTER_SCENE_ELEMENT(...) \
	BOOST_PP_OVERLOAD(MGE_REGISTER_SCENE_ELEMENT_, __VA_ARGS__)(__VA_ARGS__)
#define MGE_REGISTER_SCENE_ELEMENT_3(TAGNAME, FUNCTION, VARIABLE) \
	MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors") \
	namespace MGE::SceneLoaderListenerRegistration { bool VARIABLE = MGE::SceneLoader::getPtr()->addSceneNodesCreateListener(#TAGNAME, FUNCTION); } \
	MGE_CLANG_WARNING_POP
#define MGE_REGISTER_SCENE_ELEMENT_2(TAGNAME, FUNCTION) \
	MGE_REGISTER_SCENE_ELEMENT_3(TAGNAME, FUNCTION, isRegistred__ ## TAGNAME)
#define MGE_REGISTER_SCENE_ELEMENT_1(TAGNAME) \
	MGE_REGISTER_SCENE_ELEMENT_2(TAGNAME, create__ ## TAGNAME)
#endif

/**
 * @brief Like @ref MGE_REGISTER_SCENE_ELEMENT, but cast FUNCTION to @ref MGE::SceneLoader::SceneNodesCreateFunction.
 */
#define MGE_REGISTER_SCENE_ELEMENT_CAST(TAGNAME, FUNCTION) \
	MGE_REGISTER_SCENE_ELEMENT_2(TAGNAME, reinterpret_cast<MGE::SceneLoader::SceneNodesCreateFunction>(FUNCTION))


/**
 * @brief Create (@ref MGE::SceneLoader::SceneNodesCreateFunction) function @c create__<i>TAGNAME</i> (in namespace MGE::SceneLoaderListenerRegistration)
 *        and register it to be call by @ref MGE::SceneLoader::parseSceneXMLNode on XML tag @a TAGNAME in scene config.
 * 
 *        For registration is calling macro @ref MGE_REGISTER_SCENE_ELEMENT.
 * 
 * @param TAGNAME   Name (NOT string literal) of XML node in configuration files to call defined code block (function).
 * 
 * \par Example
	\code{.cpp}
		MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(SomeModuleTag) {
			// Create and return SomeModule class object
			// SomeModule should derived from MGE::Module.
			
			// This block of code is function body called with arguments:
			//  * const pugi::xml_node& xmlNode
			//  * const MGE::LoadingContext* context
			//  * const MGE::SceneObjectInfo& parent
			// see MGE::SceneLoader::SceneNodesCreateFunction for details.
			
			// It will be call with <SomeModuleTag> xml tag as xmlNode argument
			// while parsing configuration files.
		}
	\endcode
 */
#define MGE_SCENE_ELEMENT_FOR_XMLTAG(TAGNAME) \
	namespace MGE { namespace SceneLoaderListenerRegistration { \
		MGE::Module* create__ ## TAGNAME (const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent); \
	} } \
	MGE_REGISTER_SCENE_ELEMENT_2(TAGNAME, MGE::SceneLoaderListenerRegistration::create__## TAGNAME) \
	MGE::Module* MGE::SceneLoaderListenerRegistration::create__ ## TAGNAME (const pugi::xml_node& xmlNode, const MGE::LoadingContext* context, const MGE::SceneObjectInfo& parent)

/// @}
