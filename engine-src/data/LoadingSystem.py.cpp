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

#include "data/LoadingSystem.h"

#include "ScriptsInterface.h"
#include "data/property/pybind11_ogre_swig_cast.py.h"

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	void loadDotSceneFile(
		MGE::LoadingSystem& ls,
		const std::string& filePath,
		Ogre::SceneNode* parent = nullptr
	) {
		ls.loadDotSceneFile(filePath, nullptr, parent);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(LoadingSystem) {
	py::enum_<MGE::LoadingSystem::SceneLoadStates>(
		m, "SceneLoadStates", DOC(MGE, LoadingSystem, SceneLoadStates)
	)
		.value("NO_SCENE",    MGE::LoadingSystem::NO_SCENE)
		.value("IN_PROGRESS", MGE::LoadingSystem::IN_PROGRESS)
		.value("GAME",        MGE::LoadingSystem::GAME)
		.value("EDITOR",      MGE::LoadingSystem::EDITOR)
		//.export_values()
	;
	
	py::class_<MGE::LoadingSystem, std::unique_ptr<MGE::LoadingSystem, py::nodelete>>(
		m, "LoadingSystem", DOC(MGE, LoadingSystem)
	)
		.def("loadMapConfig", &MGE::LoadingSystem::loadMapConfig,
			DOC(MGE, LoadingSystem, loadMapConfig)
		)
		.def("loadSave", &MGE::LoadingSystem::loadSave,
			DOC(MGE, LoadingSystem, loadSave)
		)
		.def("loadEditor", &MGE::LoadingSystem::loadEditor,
			DOC(MGE, LoadingSystem, loadEditor)
		)
		.def("loadDotSceneFile", &loadDotSceneFile,
			DOC(MGE, LoadingSystem, loadDotSceneFile),
			py::arg("filePath"), py::arg("parent")
		)
		.def("loadDotSceneXML", &MGE::LoadingSystem::loadDotSceneXML,
			DOC(MGE, LoadingSystem, loadDotSceneXML),
			py::arg("xmlStr"), py::arg("context") = py::none(), py::arg("parent") = py::none()
		)
		.def("writeSave", &MGE::LoadingSystem::writeSave,
			DOC(MGE, LoadingSystem, writeSave)
		)
		.def("writeScene", &MGE::LoadingSystem::writeScene,
			DOC(MGE, LoadingSystem, writeScene)
		)
		.def("clearScene", &MGE::LoadingSystem::clearScene,
			DOC(MGE, LoadingSystem, clearScene)
		)
		.def("getGameSceneManager", &MGE::LoadingSystem::getGameSceneManager,
			py::return_value_policy::reference,
			DOC(MGE, LoadingSystem, getGameSceneManager)
		)
		.def("getSaveName", &MGE::LoadingSystem::getSaveName,
			DOC(MGE, LoadingSystem, getSaveName)
		)
		.def("getLoadingFilePath", &MGE::LoadingSystem::getLoadingFilePath,
			py::return_value_policy::reference,
			DOC(MGE, LoadingSystem, getLoadingFilePath)
		)
		.def("getSceneLoadState", &MGE::LoadingSystem::getSceneLoadState,
			DOC(MGE, LoadingSystem, getSceneLoadState)
		)
		.def_static("get", &MGE::LoadingSystem::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("LoadingSystem"))
	;
}
#endif
