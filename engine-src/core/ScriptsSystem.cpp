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

#include "ScriptsSystem.h"
#include "LogSystem.h"
#include "pragma.h"

#include <filesystem>

/* ***********     running python code     ********** */

pybind11::object MGE::ScriptsSystem::runString(
	null_end_string code,
	int mode,
	PyObject* locals,
	PyObject* globals
) {
	LOG_DEBUG("Execute Python string: " << code);
	
	PyObject* result = PyRun_String( code, mode, globals, locals );
	
	if (!result)
		onError();
	
	return pybind11::reinterpret_steal<pybind11::object>(result);
}

pybind11::object MGE::ScriptsSystem::runFile(
	null_end_string path,
	int mode,
	PyObject* locals,
	PyObject* globals
){
	LOG_INFO("Execute Python file: " << path);
	
	pybind11::str path_pystr( path );
	
	FILE* file = _Py_fopen_obj( path_pystr.ptr(), "r" );
	if (!file) {
		PyErr_Clear();
		LOG_ERROR("Can't open python script file: " << path);
		return pybind11::reinterpret_steal<pybind11::object>(nullptr);
	}
	
	getGlobalsDict()["__file__"] = std::move(path_pystr);
	
	PyObject* result = PyRun_FileEx(file, path, mode, globals, locals, 1);
	
	if (!result)
		onError();
	
	getGlobalsDict()["__file__"] = pybind11::none();
	
	return pybind11::reinterpret_steal<pybind11::object>(result);
}

void MGE::ScriptsSystem::loadScriptsFromFilesystem(const std::string& path) {
	LOG_INFO("Load python scripts from file or directory: " << path);
	
	std::filesystem::path script_path (path);
	int count = 0;
	
	// when single .py file load this file
	if (std::filesystem::is_regular_file(script_path) && script_path.extension() == ".py") {
		runFile( script_path.generic_string().c_str() );
		count = 1;
	// when directory load all .py file in this directory
	} else if (std::filesystem::is_directory(script_path)) {
		for (auto const& iter : std::filesystem::recursive_directory_iterator{script_path}) {
			if (std::filesystem::is_regular_file(iter.path()) && iter.path().extension() == ".py") {
				runFile( iter.path().generic_string().c_str() );
				++count;
			}
		}
	}
	
	if (count < 1) {
		LOG_WARNING("Can't find script file(s)");
	}
}

void MGE::ScriptsSystem::runStringInThread(const std::string& code) {
	MGE_SCRIPTS_SYSTEM_GET_SCOPED_GIL
	LOG_DEBUG("AA");
	MGE::ScriptsSystem::getPtr()->getGlobalsDict()["__MGE_ScriptsSystem__Thread__Command__"] = code;
	LOG_DEBUG("BB");
	MGE::ScriptsSystem::getPtr()->runString( "threading.Thread(target=exec, args=[__MGE_ScriptsSystem__Thread__Command__]).start()" );
	LOG_DEBUG("CC");
	MGE::ScriptsSystem::getPtr()->getGlobalsDict()["__MGE_ScriptsSystem__Thread__Command__"] = "";
}


/* ***********     init and deinit python interpreter     ********** */

PyObject* MGE::ScriptsSystem::pythonGlobals = nullptr;

MGE::ScriptsSystem::ScriptsSystem() {
	LOG_INFO("Initialize python interpreter and script system");
	
	// due to Python interpreter construction (and use pybind11::initialize_interpreter() here)
	// we can't create multiple instances of ScriptsSystem
	if (pythonGlobals)
		throw std::logic_error("pythonGlobals not NULL on call ScriptsSystem constructor");
	
	// init python interpreter and pybind11 internals
	pybind11::initialize_interpreter();
	pythonGlobals = pybind11::module::import("__main__").attr("__dict__").ptr();
	
	LOG_INFO("ScriptsSystem", "Python interpreter initialized");
	
	// set redirect python stdout to log
	pythonStdout = new MGE::ScriptsSystem::ScriptOutputLogger();
	pybind11::module::import("__MGE_ScriptsSystem__");
	getGlobalsDict()["sys"] = pybind11::module::import("sys");
	// getGlobalsDict()["sys"].attr("stdout") = pybind11::cast(pythonStdout);
	getGlobalsDict()["sys"].attr("MGEStdOut") = pybind11::cast(pythonStdout);
	runStringWithVoid(R"(
import threading
class MGEStdOut:
  def write(self, text):
    sys.MGEStdOut.write(text, threading.current_thread().name)
sys.stdout = MGEStdOut()
sys.stderr = MGEStdOut()
)");
	
	#ifdef NO_DEFAULT_GIL_LOCK
	gil_release = new pybind11::gil_scoped_release();
	#else
	gil_release = nullptr;
	#endif
	
	LOG_INFO("ScriptsSystem", "ScriptOutputLogger created and connected");
}

MGE::ScriptsSystem::~ScriptsSystem() {
	delete gil_release;
	runString("sys.stdout = sys.__stdout__");
	delete pythonStdout;
	pybind11::finalize_interpreter();
	pythonGlobals = nullptr;
}

void MGE::ScriptsSystem::setScriptOutputListener(const std::string id_str, const ScriptOutputListener& listener) {
	if (listener) {
		pythonStdout->listeners[id_str] = listener;
	} else {
		pythonStdout->listeners.erase(id_str);
	}
}

/* ***********     onError() and ScriptOutputLogger     ********** */

void MGE::ScriptsSystem::onError(std::string errorMessage) {
	if (errorMessage.size() > 0)
		errorMessage += "\n";
	else
		errorMessage = pybind11::detail::error_fetch_and_normalize("MGE::ScriptsSystem::onError").error_string();
	
	MGE_LOG.logMultiLine(errorMessage, MGE::Log::Error, "python");
	
	if (auto listener = pythonStdout->listeners.find(""); listener != pythonStdout->listeners.end())
		listener->second(errorMessage);
}

MGE::ScriptsSystem::ScriptOutputLogger::ScriptOutputLogger() {}

void MGE::ScriptsSystem::ScriptOutputLogger::write(const std::string& txt, const std::string& listener_id) {
	MGE_LOG.logLevel(MGE::Log::Info, "python") << txt;
	
	if (listener_id != "")
		if (auto listener = listeners.find(listener_id); listener != listeners.end())
			listener->second(txt);
	
	if (auto listener = listeners.find(""); listener != listeners.end())
		listener->second(txt);

}

MGE_CLANG_WARNING_IGNORED("-Wglobal-constructors")

PYBIND11_EMBEDDED_MODULE(__MGE_ScriptsSystem__, m) {
	LOG_INFO("ScriptsSystem", "init \"__MGE_ScriptsSystem__\" Python module");
	pybind11::class_<MGE::ScriptsSystem::ScriptOutputLogger>(m, "ScriptOutputLogger")
		.def("write", &MGE::ScriptsSystem::ScriptOutputLogger::write, "stdout writter (callable from python)",
			 pybind11::arg("txt"), pybind11::arg("listener_id") = ""
		)
	;
}

MGE_CLANG_WARNING_POP
