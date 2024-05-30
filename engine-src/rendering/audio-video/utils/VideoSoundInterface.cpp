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

#include "rendering/audio-video/utils/VideoSoundInterface.h"

#if defined(USE_OGGVIDEO) && defined(USE_OGGSOUND)

MGE::VideoSoundInterface::VideoSoundInterface(TheoraVideoClip* owner, int nChannels, int freq) :
	TheoraAudioInterface(owner, nChannels, freq),
	numOfOutputChannels( (nChannels == 1) ? 1 : 2 ),
	dataBufSize(numOfOutputChannels * freq / 20),
	dataBufPos(0),
	inserCounter(0)
{
	Ogre::String idName = Ogre::StringConverter::toString(reinterpret_cast<size_t>(owner), 0, ' ', std::ios_base::hex);
	
	LOG_DEBUG("VideoSoundInterface for " << owner->getName() << " / " << idName <<
						" inputChannels=" << mNumChannels << " outputChannels=" << numOfOutputChannels <<
						" freq=" << mFreq << " dataBufSize=" << dataBufSize <<
						" soundMgr=" << OgreOggSound::OgreOggSoundManager::getSingletonPtr()
	);
	
	ogreOggSoundObj = static_cast<OgreOggSound::OgreOggStreamBufferSound*>(
		OgreOggSound::OgreOggSoundManager::getSingletonPtr()->createSound( idName, "BUFFER" )
	);
	ogreOggSoundObj->setFormat(
		(nChannels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		mFreq/nChannels  /// @todo TEST: why mFreq/nChannels instead of mFreq ???
	);
	
	dataBuf = new short[dataBufSize + 32];
}

MGE::VideoSoundInterface::~VideoSoundInterface() {
	LOG_DEBUG("VideoSoundInterface DESTRUCTOR " << OgreOggSound::OgreOggSoundManager::getSingletonPtr());
	OgreOggSound::OgreOggSoundManager::getSingletonPtr()->destroySound(ogreOggSoundObj);
	// ogreOggSoundObj->_getManager()->destroyMovableObject(ogreOggSoundObj);
}

void MGE::VideoSoundInterface::insertData(float **data, int nSamples) {
	int nSamplePacks = nSamples / mNumChannels;
	for (int i=0, inIdx=0; i<nSamplePacks; ++i) {
		for (int j=0; j<numOfOutputChannels; ++j)
			dataBuf[dataBufPos++]   = static_cast<short>(Ogre::Math::Clamp((*data)[inIdx+j], -1.0f, 1.0f) * 32767);
		inIdx  += mNumChannels;
		
		if (dataBufPos >= dataBufSize) {
			ogreOggSoundObj->insertData(reinterpret_cast<char*>(dataBuf), dataBufPos * 2, (++inserCounter) > 2);
			dataBufPos = 0;
		}
	}
}

void MGE::VideoSoundInterface::destroy() {
	delete this;
}

#endif // defined(USE_OGGVIDEO) && defined(USE_OGGSOUND)
