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
#include "BaseClasses.h"
#include "StringTypedefs.h"

#include <pybind11/embed.h>

#include <map>
#include <list>
#include <functional>

#ifdef NO_DEFAULT_GIL_LOCK
	/// macro to acquire GIL (when using NO_DEFAULT_GIL_LOCK)
	#define MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL pybind11::gil_scoped_acquire acquire;
#else
	/// macro to acquire GIL (when using NO_DEFAULT_GIL_LOCK)
	#define MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
#endif

namespace MGE {

/// @addtogroup ScriptsSystem
/// @{
/// @file

/**
 * @brief Python Scripting System for call python objects from engine c++ code and for provide Python API to call engine c++ function from python code.
 * 
 * @note
 *    Require existing default (obtain by LOG macro) logger of @ref MGE::Log type.
 * 
 * @remark
 *    Singleton, due to Python interpreter.
 */
class ScriptsSystem : public MGE::Singleton<ScriptsSystem> {
public:
	/**
	 * @brief Constructor - initialize python interpreter and script system.
	 *
	 */
	ScriptsSystem();
	
	/**
	 * @brief Destructor - finalize python interpreter.
	 *
	 * @attention
	 *       Read warning about recreate Python interpreter in [pybind documentation](https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#interpreter-lifetime)
	 *       before use ScriptsSystem destructor + constructor for recreate interpreter.
	 */
	~ScriptsSystem();
	
	/**
	 * @brief Load and execute python code from file.
	 * 
	 * @param path        Path to script file.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 * 
	 * @return pybind11 object with results, null on error
	 * 
	 * @note While use NO_DEFAULT_GIL_LOCK mode this function can be called with or without GIL acquire,
	 *       but returned value must be retrieved and next must processed <b>and destroyed</b> with GIL acquire.
	 *       See @ref runFileWithCast and @ref runFileWithVoid as NO_DEFAULT_GIL_LOCK safe alternatives.
	 */
	pybind11::object runFile(
		null_end_string path,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	);
	
	/**
	 * @brief Load and execute python code from file.
	 * 
	 * Call @ref runFile and cast return value to @a RES (type of @a def).
	 * 
	 * @param path        Path to script file.
	 * @param def         Value to return if error occurs while executing string.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 * 
	 * @return Execution result casted to @a RES type, @a def on error.
	 */
	template <typename RES> inline RES runFileWithCast(
		null_end_string path,
		RES def,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			return ( runFile(path, mode, locals, globals) ).template cast<RES>();
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			return def;
		}
	}
	
	/**
	 * @brief Load and execute python code from file.
	 * 
	 * Call @ref runFile and ignore return value.
	 * 
	 * @param path        Path to script file.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 */
	inline void runFileWithVoid(
		null_end_string path,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			runFile(path, mode, locals, globals);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
		}
	}
	
	/**
	 * @brief Run python code from string.
	 * 
	 * @param code        Python code to run.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 * 
	 * @return pybind11 object with results, null on error
	 * 
	 * @note While use NO_DEFAULT_GIL_LOCK mode this function can be called with or without GIL acquire,
	 *       but returned value must be retrieved and next must processed <b>and destroyed</b> with GIL acquire.
	 *       See @ref runStringWithCast and @ref runStringWithVoid as NO_DEFAULT_GIL_LOCK safe alternatives.
	 */
	pybind11::object runString(
		null_end_string code,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	);
	
	/**
	 * @brief Run python code from string.
	 * 
	 * Call @ref runString and cast return value to @a RES (type of @a def).
	 * 
	 * @param code        Python code to run.
	 * @param def         Value to return if error occurs while executing string.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 * 
	 * @return Execution result casted to @a RES type, @a def on error.
	 */
	template <typename RES> inline RES runStringWithCast(
		null_end_string code,
		RES def,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			return ( runString(code, mode, locals, globals) ).template cast<RES>();
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			return def;
		}
	}
	
	/**
	 * @brief Run python code from string.
	 * 
	 * Call @ref runString and ignore return value.
	 * 
	 * @param code        Python code to run.
	 * @param mode        Set to:
	 *                      - ``Py_file_input``   for normal execution,
	 *                      - ``Py_single_input`` for interactive interpreter execution (print unassigned values, ...),
	 *                      - ``Py_eval_input``   for eval() function mode execution.
	 * @param locals      'locals' python dict.
	 * @param globals     'globals' python dict.
	 */
	inline void runStringWithVoid(
		null_end_string code,
		int mode          = Py_file_input,
		PyObject* locals  = pythonGlobals,
		PyObject* globals = pythonGlobals
	)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			runString(code, mode, locals, globals);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
		}
	}
	
	/**
	 * @brief Run python code from string in separate thread.
	 * 
	 * @param code        Python code to run.
	 */
	void runStringInThread(const std::string& code);
	
	/**
	 * @brief load scripts from directory or single file
	 * 
	 * @param[in] path        path to script(s) - single .py file or scripts directory
	 * 
	 * function recursive load all .py file in @a path directory (if @a path is directory)
	 * or simple load @a path file (if @a path is .py file)
	 */
	void loadScriptsFromFilesystem(const std::string& path);
	
	/**
	 * @brief Return python object based on name.
	 * 
	 * @param[in] name  Object name to get.
	 * 
	 * @attention
	 *       This function evaluate name as python code. Use with caution!
	 *       For python function in global namespace, you can use simple: \code getGlobalsDict()["functionName"] \endcode
	 *       For functions in modules: \code getGlobalsDict()["moduleName"].attr("functionName") \endcode
	 * 
	 * @note While use NO_DEFAULT_GIL_LOCK mode this function can be called with or without GIL acquire,
	 *       but returned value must be retrieved and next must processed <b>and destroyed</b> with GIL acquire.
	 */
	FORCE_INLINE pybind11::object getObject(null_end_string name) {
		return runString(name, Py_eval_input);
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Return null object on error.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 * 
	 * @return* pybind11* object with results, null on error.
	 * 
	 * @note While use NO_DEFAULT_GIL_LOCK mode this function can be called with or without GIL acquire,
	 *       but returned value must be retrieved and next must processed <b>and destroyed</b> with GIL acquire.
	 *       See @ref runObjectWithCast and @ref runObjectWithVoid as NO_DEFAULT_GIL_LOCK safe alternatives.
	 */
	template <typename... ARG> inline pybind11::object runObject(null_end_string name, ARG... args)  {
		try {
			return getObject(name)(args...);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			return pybind11::reinterpret_steal<pybind11::object>(nullptr);
		}
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Do result cast and return default value on error.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] def   Default value to return on python execution error.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 * 
	 * @return Execution result casted to @a RES type, @a def on error.
	 */
	template <typename RES, typename... ARG> inline RES runObjectWithCast(null_end_string name, RES def, ARG... args)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			return ( getObject(name)(args...) ).template cast<RES>();
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			return def;
		}
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Do not return result.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 */
	template <typename... ARG> inline void runObjectWithVoid(null_end_string name, ARG... args)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			getObject(name)(args...);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
		}
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Throw exception on error.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 * 
	 * @return* pybind11* object with results, throw exception on error.
	 * 
	 * @note While use NO_DEFAULT_GIL_LOCK mode this function can be called with or without GIL acquire,
	 *       but returned value must be retrieved and next must processed <b>and destroyed</b> with GIL acquire.
	 *       See @ref runObjectWithCastThrow and @ref runObjectWithVoidThrow as NO_DEFAULT_GIL_LOCK safe alternatives.
	 */
	template <typename... ARG> inline pybind11::object runObjectThrow(null_end_string name, ARG... args)  {
		try {
			return getObject(name)(args...);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			throw e;
		}
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Do result cast. Throw exception on error.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 * 
	 * @return Execution result casted to @a RES type, @a def on error.
	 */
	template <typename RES, typename... ARG> inline RES runObjectWithCastThrow(null_end_string name, ARG... args)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			return ( getObject(name)(args...) ).template cast<RES>();
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			throw e;
		}
	}
	
	/**
	 * @brief Run python callable object (e.g. function) based on (scoped) name.
	 *        Do not return result. Throw exception on error.
	 * 
	 * @param[in] name  Scoped name of python object to run.
	 * @param[in] args  Unspecified number of arguments for python callable object.
	 */
	template <typename... ARG> inline void runObjectWithVoidThrow(null_end_string name, ARG... args)  {
		try {
			MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
			getObject(name)(args...);
		} catch(pybind11::error_already_set& e) {
			onError(e.what());
			throw e;
		}
	}
	
	/**
	 * @brief Return python globals directory.
	 *
	 * @note While use NO_DEFAULT_GIL_LOCK mode all operation on returned object must be doing with GIL acquire.
	 */
	FORCE_INLINE PyObject* getGlobals() const {
		return pythonGlobals;
	}
	
	/**
	 * @brief Return python globals directory as pybind11::dict.
	 *
	 * @note While use NO_DEFAULT_GIL_LOCK mode all operation on returned object must be doing with GIL acquire.
	 */
	FORCE_INLINE pybind11::dict getGlobalsDict() const {
		return pybind11::handle(pythonGlobals).cast<pybind11::dict>();
	}
	
	/**
	 * @brief Type of function to recive script output (stdout and errors message).
	 * 
	 * This is one argument function or class member function. Argument is a pointer to message.
	 * This pointer can be deleted after this function return (do not store this pointer).
	 */
	typedef std::function<void(const std::string& text)> ScriptOutputListener;
	
	/**
	 * @brief Class to use as python stdout object for write output to log.
	 *
	 */
	struct ScriptOutputLogger {
		/// constructor
		ScriptOutputLogger();
		
		/// stdout writter (callable from python)
		void write(const std::string& txt, const std::string& listener_id = "");
		
		/// when current thread or default listener (empty string) is not NULL
		/// then capture output (errors messages too) as text and call listener
		std::map<std::string, ScriptOutputListener> listeners;
	};
	
	/**
	 * @brief Set script output (stdout and errors message) listener.
	 * 
	 * @param id_str    listener id_string – thread name to capture output, empty string for global (all threads) listener.
	 * @param listener  static function or std::bind to receive python output or nullptr to remove listener.
	 *
	 * \par Example
		\code{.cpp}
			void f1(const std::string& txt);
			void f2(const std::string& txt, void* myObj);
			struct c1 {
				void f3(const std::string& txt);
			}
			
			// [...]
			
			scriptSys->setScriptOutputListener(f1);
			scriptSys->setScriptOutputListener(std::bind(f2, std::placeholders::_1, this));
			scriptSys->setScriptOutputListener(std::bind(&c1::f3, this, std::placeholders::_1));
		\endcode
	 */
	void setScriptOutputListener(const std::string id_str, const ScriptOutputListener& listener);
	
protected:
	/// Python globals dictionary.
	static PyObject* pythonGlobals;
	
	/// Used on NO_DEFAULT_GIL_LOCK mode to hold global GIL release
	/// (need because GIL is hold by default after call initialize_interpreter)
	pybind11::gil_scoped_release *gil_release;
	
	/// ScriptOutputLogger used to catch python stdout
	ScriptOutputLogger* pythonStdout;
	
	/// Helper function to catch error messages.
	void onError(std::string errorMessage = MGE::EMPTY_STRING);
};
/// @}
}
