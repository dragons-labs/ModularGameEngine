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
#include "BaseClasses.h"
#include "ListenerSet.h"
#include "LogSystem.h"

#include <pybind11/embed.h>

//
// support script api initFunctions registration
//

namespace MGE { 

/// @addtogroup ScriptsSystem
/// @{
/// @file

/**
 * @brief  Namespace for script (Python) API related stuff - wrappers, submodule init functions, ...
 */
namespace ScriptsInterface {
	namespace py = pybind11;
	
	/**
	 * @brief Support auto registration multiple C++ modules in single Python module script interface.
	 *        See @ref MGE_REGISTER_SCRIPT_API_INITIALIZER macro.
	 * 
	 * @remark
	 *    (Trivial) Singleton, for support auto registration in @ref initFunctions.
	 */
	struct Init : MGE::TrivialSingleton<Init> {
		/// type of init function
		typedef bool (*ModuleInitFunction)(pybind11::module_&);
		
		/// call all registered module init functions
		inline void initAll(py::module_& m) {
			initFunctions.callAll(m);
		}
		
		/// register module init function
		/// use @ref MGE_REGISTER_SCRIPT_API_INITIALIZER macro instead of direct call this function.
		inline bool registerModule(ModuleInitFunction function, int key) {
			return initFunctions.addListener(function, key);
		}
		
	protected:
		/// set of function to call on module import ... each of them create part of module API with pybind11
		FunctionListenerSet<ModuleInitFunction> initFunctions;
		
		friend class TrivialSingleton<Init>;
		Init() = default;
		~Init() = default;
	};
}
/// @}
}


////////////////////////////   Preprocessor Macros   ////////////////////////////

/// @addtogroup ScriptsSystem
/// @{

/**
 * @brief Register engine module @a MODULE to init Script API by call @c initAPI__<i>MODULE</i> function.
 *        For auto registration using static bool variable @c isRegistred__<i>MODULE</i>.
 *        Function @c initAPI__<i>MODULE</i> must be defined and macro must be call inside @ref MGE::ScriptsInterface namespace.
 * 
 * @param MODULE   Name of module to register (@c initAPI__<i>MODULE</i> function will be registered).
 * @param KEY      (optional, default 10) Numeric value to determined order of registered function call. 
 *                 This is need to enforce registration base class before derived classes.
 * 
 * \par Example
	\code{.cpp}
		namespace MGE { namespace ScriptsInterface {
			bool initAPI__ABC(py::module_& m);
			MGE_REGISTER_SCRIPT_API_INITIALIZER(ABC);
		} }
	\endcode
 */
#ifdef __DOCUMENTATION_GENERATOR__
#define MGE_REGISTER_SCRIPT_API_INITIALIZER(MODULE, KEY)
#else
#define MGE_REGISTER_SCRIPT_API_INITIALIZER(...)            BOOST_PP_OVERLOAD(MGE_REGISTER_SCRIPT_API_INITIALIZER_, __VA_ARGS__)(__VA_ARGS__)
#define MGE_REGISTER_SCRIPT_API_INITIALIZER_2(MODULE, KEY) \
	MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors") \
	bool isRegistred__ ## MODULE = Init::getPtr()->registerModule(initAPI__ ## MODULE, KEY); \
	MGE_CLANG_WARNING_POP
#define MGE_REGISTER_SCRIPT_API_INITIALIZER_1(MODULE)       MGE_REGISTER_SCRIPT_API_INITIALIZER_2(MODULE, 10)
#endif

/**
 * @brief Create function @c initAPIx__<i>MODULE</i> and @c initAPI__<i>MODULE</i> (in namespace @ref MGE::ScriptsInterface)
 *        and register it (@c initAPIx__ via @c initAPI__ proxy) to init Python script API for module @a MODULE.
 * 
 *        For registration is calling macro @ref MGE_REGISTER_SCRIPT_API_INITIALIZER.
 * 
 * @param MODULE   Name of module to register (@c initAPI__<i>MODULE</i> function will be registered).
 * @param KEY      (optional, defaul 10) Numeric value to determined order of registered function call. 
 *                 This is need to enforce registration base class before derived classes.
 * 
 * \par Example
	\code{.cpp}
		MGE_SCRIPT_API_FOR_MODULE(MyModuleName, 13) {
			// create and return SomeModule class object
			// SomeModule should derived from MGE::Module;
			
			// this block of code is function body call with arguments:
			//  * py::module_& m
			
			// it will be call (in order determined by KEY, in this example 13)
			// on import MGE python module
		}
	\endcode
 */
#ifdef __DOCUMENTATION_GENERATOR__
#define MGE_SCRIPT_API_FOR_MODULE(MODULE, KEY)
#else
#define MGE_SCRIPT_API_FOR_MODULE(...)            BOOST_PP_OVERLOAD(MGE_SCRIPT_API_FOR_MODULE_, __VA_ARGS__)(__VA_ARGS__)
#define MGE_SCRIPT_API_FOR_MODULE_2(MODULE, KEY) \
	namespace MGE { namespace ScriptsInterface { \
		void initAPIx__ ## MODULE (py::module_& m); \
		bool initAPI__ ## MODULE (py::module_& m) { \
			LOG_INFO("ScriptsInterface", "init " #MODULE); \
			initAPIx__ ## MODULE(m); \
			return true; \
		} \
		MGE_REGISTER_SCRIPT_API_INITIALIZER_2(MODULE, KEY) \
	} } \
	void MGE::ScriptsInterface::initAPIx__ ## MODULE (py::module_& m)
#define MGE_SCRIPT_API_FOR_MODULE_1(MODULE)   MGE_SCRIPT_API_FOR_MODULE_2(MODULE, 10)
#endif


////////////////////////////   include pybind11_mkdoc generated docstring file   ////////////////////////////

#ifndef __DOCUMENTATION_GENERATOR__
	// try use docstring file from PYTHON_DOCSTRINGS_FILE macro, unset on fail
	#if defined(PYTHON_DOCSTRINGS_FILE)
		#if ! __has_include(PYTHON_DOCSTRINGS_FILE)
			#undef PYTHON_DOCSTRINGS_FILE
		#endif
	#endif
	
	// try use default docstring file when PYTHON_DOCSTRINGS_FILE macro is not set
	#if !defined(PYTHON_DOCSTRINGS_FILE)
		#define PYTHON_DOCSTRINGS_FILE  "docstrings.h"
	#endif
	
	// use docstring file from PYTHON_DOCSTRINGS_FILE when available otherwise print warning
	#if __has_include(PYTHON_DOCSTRINGS_FILE)
		MGE_GNUC_WARNING_IGNORED("-Wreserved-macro-identifier")
		#include PYTHON_DOCSTRINGS_FILE
		MGE_GNUC_WARNING_POP
	#else
		#pragma message "Build without pybind11_mkdoc generated file => no docstring in python API"
		#define DOC(...) ""
	#endif
#else
	/// Return pybind11_mkdoc generated docstring.
	#define DOC(...) ""
#endif

/// Put docstring for singleton getPtr() method for @a a class.
#define DOC_SINGLETON_GET(a) "return " a " singleton object"

/// @}
