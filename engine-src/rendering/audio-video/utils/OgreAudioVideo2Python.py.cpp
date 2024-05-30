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

#include "config.h"

#ifdef USE_OGGSOUND
#include <OgreOggISound.h>
#endif

#ifdef USE_OGGVIDEO
#include <TheoraVideoClip.h>
#endif

#include "LogSystem.h"
#include "ScriptsInterface.h"

#ifndef __DOCUMENTATION_GENERATOR__
MGE_SCRIPT_API_FOR_MODULE(OgreAudioVideo) {
	#ifdef USE_OGGSOUND
	py::class_<OgreOggSound::OgreOggISound, std::unique_ptr<OgreOggSound::OgreOggISound, py::nodelete>>(
		m, "OgreOggISound", "Equivalent of OgreOggSound::OgreOggISound for scripts"
	)
		.def("play",       &OgreOggSound::OgreOggISound::play,
			"start / play sound"
		)
		.def("pause",      &OgreOggSound::OgreOggISound::pause,
			"pause sound"
		)
		.def("stop",       &OgreOggSound::OgreOggISound::stop,
			"stop sound"
		)
		.def("loop",       &OgreOggSound::OgreOggISound::loop,
			"set looping status"
		)
		.def("startFade",  &OgreOggSound::OgreOggISound::startFade,
			"Starts a fade in/out of the sound volume"
		)
		
		.def("isPlaying",  &OgreOggSound::OgreOggISound::isPlaying,
			"return true when sound is playing"
		)
		.def("isPaused",   &OgreOggSound::OgreOggISound::isPaused,
			"return true when sound is paused"
		)
		.def("isStopped",  &OgreOggSound::OgreOggISound::isStopped,
			"return true when sound is stopped"
		)
		.def("isFading",   &OgreOggSound::OgreOggISound::isFading,
			"return true when sound is fading"
		)
		
		.def("setPlayPosition", &OgreOggSound::OgreOggISound::setPlayPosition,
			"Sets the position of the playback cursor in seconds"
		)
		.def("getPlayPosition", &OgreOggSound::OgreOggISound::getPlayPosition,
			"Gets the position of the playback cursor in seconds "
		)
	;
	#endif
	#ifdef USE_OGGVIDEO
	py::class_<TheoraVideoClip, std::unique_ptr<TheoraVideoClip, py::nodelete>>(
		m, "TheoraVideoClip", "Equivalent of TheoraVideoClip for scripts"
	)
		.def("play",       &TheoraVideoClip::play,
			"start / play videoclip"
		)
		.def("pause",      &TheoraVideoClip::pause,
			"pause videoclip"
		)
		.def("stop",       &TheoraVideoClip::stop,
			"stop videoclip"
		)
		.def("restart",    &TheoraVideoClip::restart,
			"restart videoclip"
		)
		.def("isDone",     &TheoraVideoClip::isDone,
			"return true when videoclip is done"
		)
		.def("isPaused",   &TheoraVideoClip::isPaused,
			"return true when videoclip is paused"
		)
		
		.def("setAutoRestart",  &TheoraVideoClip::setAutoRestart,
			"set auto-restart for videoclip"
		)
		.def("seek",            &TheoraVideoClip::seek,
			"seek videoclip"
		)
		.def("getTimePosition", &TheoraVideoClip::getTimePosition,
			"get current time position in videoclip"
		)
		.def("getDuration",     &TheoraVideoClip::getDuration,
			"get duration of videoclip"
		)
		
		.def("setPlaybackSpeed", &TheoraVideoClip::setPlaybackSpeed,
			"set playback speed"
		)
		.def("getPlaybackSpeed", &TheoraVideoClip::getPlaybackSpeed,
			"get playback speed"
		)
	;
	#endif
}
#endif
