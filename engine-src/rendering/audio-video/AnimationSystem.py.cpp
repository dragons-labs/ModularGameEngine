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

#include "ScriptsInterface.h"

#include "rendering/audio-video/AnimationSystem.h"

#include "data/property/pybind11_ogre_swig_cast.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE { namespace ScriptsInterface {
	bool setAnimation(const Ogre::SceneNode* node, const std::string& name, MGE::AnimationSystem::Operation mode, float initTime, float endTime, float speedFactor, bool loop, bool save) {
		return MGE::AnimationSystem::getPtr()->setAnimation(node, name, mode, initTime, endTime, speedFactor, loop, save);
	}
} }

MGE_SCRIPT_API_FOR_MODULE(AnimationSystem) {
	py::enum_<MGE::AnimationSystem::Operation>(m, "SetAnimationMode", DOC(MGE, AnimationSystem, Operation))
		.value("ADD",        MGE::AnimationSystem::ADD)
		.value("SET_POSE",   MGE::AnimationSystem::SET_POSE)
		.value("REMOVE",     MGE::AnimationSystem::REMOVE)
		.value("REPLACE",    MGE::AnimationSystem::REPLACE)
		.value("REMOVE_ALL", MGE::AnimationSystem::REMOVE_ALL)
		//.export_values();
	;
	
	m.def("setAnimation", &setAnimation, DOC(MGE, AnimationSystem, setAnimation));
	
	m.def("getAnimationTime", &MGE::AnimationSystem::getAnimationTime, DOC(MGE, AnimationSystem, getAnimationTime));
	
	m.def("createParticle", &MGE::AnimationSystem::createParticle, DOC(MGE, AnimationSystem, createParticle));
}
#endif
