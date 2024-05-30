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

#include "force_inline.h"
#include "pragma.h"
#include "StringUtils.h"
#include "ListenerSet.h"
#include "BaseClasses.h"

#include <set>
#include <pugixml.hpp>

namespace MGE { struct LoadingContext; struct Module; }

namespace MGE {

/// @addtogroup XMLConfigSystem
/// @{
/// @file

/**
 * @brief Parse XML config file. Provide access to main config settings and module create interface.
 * 
 * @remark
 *    (Trivial) Singleton, for support auto registration in @ref configParserListeners.
 */
class ConfigParser : public MGE::TrivialSingleton<ConfigParser> {
public:
	/**
	 * @brief Read and parse main config file.
	 * 
	 * @param path          Path to config file.
	 * @param rootNodeName  Name of root XML node in config file.
	 */
	void initMainConfig(null_end_string path, MGE::null_end_string rootNodeName);
	
	/**
	 * @brief Access to main config file.
	 * 
	 * @param nodeName  Name of XML node to get from main config.
	 * 
	 * @return Direct child (with name indicated by @a nodeName) of root XML node from main config.
	 */
	pugi::xml_node getMainConfig(null_end_string nodeName);
	
	
	/**
	 * @brief Struct used to describe loaded (via config) engine module. Used in LoadedModulesSet.
	 */
	struct LoadedModuleInfo {
		/// name of module (name of xml tag used to create module by @ref createAndConfigureModules)
		std::string id;
		/// pointer to module
		MGE::Module* ptr;
		/// numeric runlevel at which module was created (pass thru via @ref createAndConfigureModules)
		int runlevel;
	};
	
	/**
	 * @brief Type for loaded (created) modules colection.
	 */
	typedef std::multiset<LoadedModuleInfo, std::less<>> LoadedModulesSet;
	
	/**
	 * @brief Parse XML config to create and configure engine modules.
	 *        For each child call function registered in @ref configParserListeners for the child node name.
	 * 
	 * @param[out] createdModules  Set of created engine module to add modules created by this call of createAndConfigureModules.
	 * @param[in]  xmlNode         XML node with (as child nodes) list of modules to load.
	 * @param[in]  context         Structure with info about restoring/loading context.
	 *                             Pass thru to called function (@ref SceneConfigParseFunction) as context argument.
	 * @param[in]  runlevel        Runlevel used for add load info to @a createdModules.
	 *                             Pass thru to created @ref LoadedModuleInfo entry in @a createdModules.
	 */
	void createAndConfigureModules(LoadedModulesSet& createdModules, const pugi::xml_node& xmlNode, const MGE::LoadingContext* context = nullptr, int runlevel = 0);
	
	/**
	 * @brief Create engine module(s) based on module name nad XML configuration node.
	 * 
	 * @param[out] createdModules  Set of created engine module to add modules created by this call of createAndConfigureModules.
	 * @param[in]  xmlNodeName     Name of module to load (typically the same as xmlNode.name()).
	 * @param[in]  xmlNode         XML node with configuration for modules to load.
	 * @param[in]  context         Structure with info about restoring/loading context.
	 *                             Pass thru to called function (@ref SceneConfigParseFunction) as context argument.
	 * @param[in]  runlevel        Runlevel used for add load info to @a createdModules.
	 *                             Pass thru to created @ref LoadedModuleInfo entry in @a createdModules.
	 */
	int createAndConfigureModules(LoadedModulesSet& createdModules, const std::string_view& xmlNodeName, const pugi::xml_node& xmlNode, const MGE::LoadingContext* context = nullptr, int runlevel = 0);
	
	/**
	 * @brief Type of static function for register in @ref configParserListeners.
	 * 
	 * @param xmlNode      XML node object, that will be parsed to load state of this object.
	 * @param context      Structure with info about restoring/loading context. In general case can be NULL.
	 * 
	 * @return Pointer to created module, NULL when module was not created due to error or function only do some thinks and do not create module.
	 */
	typedef MGE::Module* (*SceneConfigParseFunction)(const pugi::xml_node& xmlNode, const MGE::LoadingContext* context);
	
	/**
	 * @brief Add (register) listener in @ref configParserListeners.
	 * 
	 */
	bool addConfigParseListener(const std::string_view& tagName, SceneConfigParseFunction callbackFunction);
	
	/**
	 * @brief Remove (unregister) listener from @ref configParserListeners.
	 * 
	 * @remark Typically modules used static registration via @ref MGE_REGISTER_MODULE / @ref MGE_REGISTER_MODULE_FULL macro.
	 *         So they don't use unregistration, but this can be useful for sub-module level configuration nodes.
	 */
	void remConfigParseListener(SceneConfigParseFunction callbackFunction);
	
	/**
	 * @brief Print to log all registered XML tag names.
	 */
	void listListeners();
	
protected:
	/**
	 * @brief Listener set for static function used for processing XML nodes with module configuration.
	 *        These functions will be call with corresponding XML node (key == tag name) while processing config with @ref createAndConfigureModules.
	 */
	MGE::FunctionListenerSet<SceneConfigParseFunction, std::string_view, std::string>   configParserListeners;
	
	/**
	 * @brief XML document with main config.
	 */
	pugi::xml_document mainConfig;
	
	/**
	 * @brief XML root node with main config.
	 */
	pugi::xml_node mainConfigRootNode;
};

/// %Compare operations. Needed to use in std::multiset. @{
FORCE_INLINE bool operator< (const ConfigParser::LoadedModuleInfo& a, const ConfigParser::LoadedModuleInfo& b) { return a.id < b.id; }

FORCE_INLINE bool operator< (const ConfigParser::LoadedModuleInfo& a, const std::string& s) { return a.id < s; }
FORCE_INLINE bool operator< (const std::string& s, const ConfigParser::LoadedModuleInfo& b) { return s < b.id; }

FORCE_INLINE bool operator< (const ConfigParser::LoadedModuleInfo& a, const std::string_view& s) { return a.id < s; }
FORCE_INLINE bool operator< (const std::string_view& s, const ConfigParser::LoadedModuleInfo& b) { return s < b.id; }
/// @}

/// @}
}


////////////////////////////   Preprocessor Macros   ////////////////////////////

/// @addtogroup XMLConfigSystem
/// @{
/// @file

/**
 * @brief Register @a FUNCTION function in @ref MGE::ConfigParser::configParserListeners to be call (by @ref MGE::ConfigParser::createAndConfigureModules)
 *        for process XML tag @a TAGNAME to create and configure engine module.
 * 
 * @param TAGNAME   Name of XML node in configuration files to call @FUNCTION.
 * @param FUNCTION  Function (@ref MGE::ConfigParser::SceneConfigParseFunction) to register.
 *                  Optional argument – when not provide using @c create__<i>TAGNAME</i>.
 * @param VARIABLE  Variable created and used for static auto registration.
 *                  Will be created in @c MGE::ConfigParseListenerRegistration namespace.
 *                  Optional argument – when not provide using @c isRegistred__<i>TAGNAME</i>.
 * 
 * \par Example
	\code{.cpp}
		MGE::Module* create__SomeModule(const pugi::xml_node& xml, const MGE::LoadingContext* context) {
			// create and return SomeModule class object
			// SomeModule should derived from MGE::Module;
		}
		MGE_REGISTER_MODULE(SomeModule); // SomeModule__load() will be call with <SomeModule> xml tag as argument.
	\endcode
 */
#ifdef __DOCUMENTATION_GENERATOR__
#define MGE_REGISTER_MODULE(TAGNAME, FUNCTION, VARIABLE)
#else
#define MGE_REGISTER_MODULE(...) \
	BOOST_PP_OVERLOAD(MGE_REGISTER_MODULE_, __VA_ARGS__)(__VA_ARGS__)
#define MGE_REGISTER_MODULE_3(TAGNAME, FUNCTION, VARIABLE) \
	MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors") \
	namespace MGE::ConfigParseListenerRegistration { bool VARIABLE = MGE::ConfigParser::getPtr()->addConfigParseListener(#TAGNAME, FUNCTION); } \
	MGE_CLANG_WARNING_POP
#define MGE_REGISTER_MODULE_2(TAGNAME, FUNCTION) \
	MGE_REGISTER_MODULE_3(TAGNAME, FUNCTION, isRegistred__ ## TAGNAME)
#define MGE_REGISTER_MODULE_1(TAGNAME) \
	MGE_REGISTER_MODULE_2(TAGNAME, create__ ## TAGNAME)
#endif

/**
 * @brief Like @ref MGE_REGISTER_MODULE, but cast FUNCTION to @ref MGE::ConfigParser::SceneConfigParseFunction.
 */
#define MGE_REGISTER_MODULE_CAST(TAGNAME, FUNCTION) \
	MGE_REGISTER_MODULE_2(TAGNAME, reinterpret_cast<MGE::ConfigParser::SceneConfigParseFunction>(FUNCTION))

/**
 * @brief Create (@ref MGE::ConfigParser::SceneConfigParseFunction) function @c create__<i>TAGNAME</i> (in @c namespace MGE::ConfigParseListenerRegistration)
 *        and register it to be call by @ref MGE::ConfigParser::createAndConfigureModules on XML tag @a TAGNAME for create and configure engine module.
 * 
 *        For registration is calling macro @ref MGE_REGISTER_MODULE.
 * 
 * @param TAGNAME   Name of XML node in configuration files to call defined code block (function).
 * 
 * \par Example
	\code{.cpp}
		MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(SomeModuleTag) {
			// Create and return SomeModule class object
			// SomeModule should derived from MGE::Module.
			
			// This block of code is function body called with arguments:
			//  * const pugi::xml_node& xmlNode
			//  * const MGE::LoadingContext* context
			// see MGE::ConfigParser::SceneConfigParseFunction for details.
			
			// It will be call with <SomeModuleTag> xml tag as xmlNode argument
			// while parsing configuration files.
		}
	\endcode
 */
#define MGE_CONFIG_PARSER_MODULE_FOR_XMLTAG(TAGNAME) \
	namespace MGE { namespace ConfigParseListenerRegistration { \
		MGE::Module* create__ ## TAGNAME (const pugi::xml_node& xmlNode, const MGE::LoadingContext* context); \
	} } \
	MGE_REGISTER_MODULE_2(TAGNAME, MGE::ConfigParseListenerRegistration::create__ ## TAGNAME) \
	MGE::Module* MGE::ConfigParseListenerRegistration::create__ ## TAGNAME (const pugi::xml_node& xmlNode, const MGE::LoadingContext* context)

/// @}
