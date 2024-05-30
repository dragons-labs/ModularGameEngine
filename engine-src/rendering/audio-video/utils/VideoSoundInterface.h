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

#pragma   once
#include "config.h"

#if defined(USE_OGGVIDEO) && defined(USE_OGGSOUND)

#include "LogSystem.h"

#include <OgreOggSoundManager.h>
#include <OgreOggStreamBufferSound.h>
#include <TheoraAudioInterface.h>
#include <TheoraVideoClip.h>

namespace MGE {

/// @addtogroup AudioVideo
/// @{
/// @file

/**
 * @brief Interface between "Ogre Theora Video Plugin" and "OgreOggSound Plugin" – per video interface object.
 */
struct VideoSoundInterface : public TheoraAudioInterface {
	OgreOggSound::OgreOggStreamBufferSound* ogreOggSoundObj;
	int numOfOutputChannels;
	
	short* dataBuf;
	int    dataBufSize;
	int    dataBufPos;
	
	int    inserCounter;
	
	VideoSoundInterface(TheoraVideoClip* owner, int nChannels, int freq);
	
	virtual ~VideoSoundInterface();
	
	void insertData(float **data, int nSamples) override final;
	
	void destroy() override final;
};

/**
 * @brief Factory for interface between "Ogre Theora Video Plugin" and "OgreOggSound Plugin"
 */
struct VideoSoundInterfaceFactory : TheoraAudioInterfaceFactory {
	TheoraAudioInterface* createInstance(TheoraVideoClip* owner, int nChannels, int freq) override final {
		return new VideoSoundInterface(owner, nChannels, freq);
	}
	
	static VideoSoundInterfaceFactory* getSingletonPtr() {
		static VideoSoundInterfaceFactory singleton;
		return& singleton;
	}
protected:
	VideoSoundInterfaceFactory() {}
	
	virtual ~VideoSoundInterfaceFactory() {}
};

/// @}

}

#endif // defined(USE_OGGVIDEO) && defined(USE_OGGSOUND)
