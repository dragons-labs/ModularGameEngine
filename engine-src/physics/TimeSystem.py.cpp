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

#include "physics/TimeSystem.h"

#include "ScriptsInterface.h"

// no Ogre in this API #include "data/property/pybind11_ogre_swig_cast.py.h"
// no STL containers in this API #include "data/property/pybind11_stl.py.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(TimeSystem) {
	py::class_<MGE::TimerSet, std::unique_ptr<MGE::TimerSet, py::nodelete>>(
		m,
		"TimerSet",
		"equivalent of MGE::TimerSet for scripts\n\
		\n\
		All engine exported TimerSet instances are available as module-level variables in MGE module - see DATA section in help(MGE)\n\
		\n\
		Typically, there are two module-level (and global accessible too) instances of this class:\n\
			* gameTimer (don't work on active pause)\n\
			* realtimeTimer (work on active pause, don't chane speed when change game speed) \
		"
	)
		.def("addTimer",      &MGE::TimerSet::addTimer,
			DOC(MGE, TimerSet, addTimer)
		)
		.def("stopTimer",    &MGE::TimerSet::stopTimer,
			DOC(MGE, TimerSet, stopTimer)
		)
		.def("getCounter",    &MGE::TimerSet::getCounter,
			DOC(MGE, TimerSet, getCounter)
		)
		.def("getCounterStr", &MGE::TimerSet::getCounterStr,
			DOC(MGE, TimerSet, getCounterStr)
		)
	;

	py::class_<MGE::TimeSystem, std::unique_ptr<MGE::TimeSystem, py::nodelete>>(
		m, "TimeSystem", "equivalent of MGE::TimeSystem for scripts"
	)
		.def("setSpeed",     &MGE::TimeSystem::setSpeed,
			DOC(MGE, TimeSystem, setSpeed)
		)
		.def("getSpeed",     &MGE::TimeSystem::getSpeed,
			DOC(MGE, TimeSystem, getSpeed)
		)
		.def("pause",        &MGE::TimeSystem::pause,
			DOC(MGE, TimeSystem, pause)
		)
		.def("unpause",      &MGE::TimeSystem::unpause,
			DOC(MGE, TimeSystem, unpause)
		)
		.def("switchPause",  py::overload_cast<>( &MGE::TimeSystem::switchPause ),
			 DOC(MGE, TimeSystem, switchPause, 2)
		)
		.def("switchPause",  py::overload_cast<bool>( &MGE::TimeSystem::switchPause ),
			 DOC(MGE, TimeSystem, switchPause)
		)
		.def("gameIsPaused", &MGE::TimeSystem::gameIsPaused,
			DOC(MGE, TimeSystem, gameIsPaused)
		)
		.def("getMilliseconds",        &MGE::TimeSystem::getMilliseconds,
			DOC(MGE, TimeSystem, getMilliseconds)
		)
		.def_readonly("gameTimer",     &MGE::TimeSystem::gameTimer,
			DOC(MGE, TimeSystem, gameTimer)
		)
		.def_readonly("realtimeTimer", &MGE::TimeSystem::realtimeTimer,
			DOC(MGE, TimeSystem, realtimeTimer)
		)
		.def_static("get", &MGE::TimeSystem::getPtr, py::return_value_policy::reference, DOC_SINGLETON_GET("TimeSystem"))
	;
}
#endif
