/*
Copyright (c) 2017-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2000-2014 Torus Knot Software Ltd

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

Based on:
	→ OGRE (MIT licensed)
*/

#include "rendering/utils/OgreHLMS.h"

#include "LogSystem.h"
#include "StringUtils.h"

#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>

#include <OgreArchiveManager.h>
#include <OgrePlatformInformation.h>
#include <OgreHlmsManager.h>
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <Hlms/Pbs/OgreHlmsPbs.h>

#include "OgreHlmsDiskCache.h"
#include "OgreGpuProgramManager.h"


void MGE::OgreHLMS::initHLMS(const std::string_view& hlmsRootPath) {
	LOG_INFO("Initialise Ogre HLMS from: " << hlmsRootPath);
	
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
	const Ogre::String archiveType("FileSystem");
	Ogre::String mainFolderPath;
	Ogre::StringVector libraryFoldersPaths;
	Ogre::StringVector::const_iterator libraryFolderPathIt;
	Ogre::StringVector::const_iterator libraryFolderPathEn;
	
	Ogre::HlmsUnlit* hlmsUnlit = 0;
	Ogre::HlmsPbs* hlmsPbs = 0;
	
	// Create & Register HlmsUnlit
	LOG_INFO("For Unlit use:");
	{
		// Get the path to all the subdirectories used by HlmsUnlit
		Ogre::HlmsUnlit::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
		LOG_INFO("  - " << hlmsRootPath + mainFolderPath);
		Ogre::Archive* archiveUnlit = archiveManager.load( hlmsRootPath + mainFolderPath, archiveType, true );
		
		// Get the library archive(s)
		Ogre::ArchiveVec archiveUnlitLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while( libraryFolderPathIt != libraryFolderPathEn )
		{
			Ogre::Archive* archiveLibrary = archiveManager.load( hlmsRootPath +* libraryFolderPathIt, archiveType, true );
			LOG_INFO("  - " << hlmsRootPath +* libraryFolderPathIt);
			archiveUnlitLibraryFolders.push_back( archiveLibrary );
			++libraryFolderPathIt;
		}
		
		// Create and register the unlit Hlms
		hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &archiveUnlitLibraryFolders );
		#ifdef MGE_DEBUG
		hlmsUnlit->setDebugOutputPath(true, true, MGE_DEBUG_HLMS_PATH);
		#else
		hlmsUnlit->setDebugOutputPath(false, false);
		#endif
		Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );
	}
	
	// Create & Register HlmsPbs
	LOG_INFO("For Pbs use:");
	{
		Ogre::HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
		LOG_INFO("  - " << hlmsRootPath + mainFolderPath);
		Ogre::Archive* archivePbs = archiveManager.load( hlmsRootPath + mainFolderPath, archiveType, true );
		
		Ogre::ArchiveVec archivePbsLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while( libraryFolderPathIt != libraryFolderPathEn )
		{
			Ogre::Archive* archiveLibrary = archiveManager.load( hlmsRootPath +* libraryFolderPathIt, archiveType, true );
			LOG_INFO("  - " << hlmsRootPath +* libraryFolderPathIt);
			archivePbsLibraryFolders.push_back( archiveLibrary );
			++libraryFolderPathIt;
		}
		
		hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &archivePbsLibraryFolders );
		#ifdef MGE_DEBUG
		hlmsPbs->setDebugOutputPath(true, true, MGE_DEBUG_HLMS_PATH);
		#else
		hlmsPbs->setDebugOutputPath(false, false);
		#endif
		Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
	}
	
	// fixes for 3D11 ...
	Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
	if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" ) {
		//Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
		//and below to avoid saturating AMD's discard limit (8MB) or
		//saturate the PCIE bus in some low end machines.
		bool supportsNoOverwriteOnTextureBuffers;
		renderSystem->getCustomAttribute( "MapNoOverwriteOnDynamicBufferSRV", &supportsNoOverwriteOnTextureBuffers );
		
		if( !supportsNoOverwriteOnTextureBuffers ) {
			hlmsPbs->setTextureBufferDefaultSize( 512 * 1024 );
			hlmsUnlit->setTextureBufferDefaultSize( 512 * 1024 );
		}
	}
	
}

void MGE::OgreHLMS::loadHLMSCache() {
	LOG_INFO("Load Microcode and HLMS Cache");
	
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
	Ogre::Archive* cacheDirArch = archiveManager.load( "cache", "FileSystem", true );
	
	Ogre::String filename("microcodeCodeCache.cache");
	if( cacheDirArch->exists( filename ) ) {
		Ogre::DataStreamPtr shaderCacheFile = cacheDirArch->open( filename );
		Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache( shaderCacheFile );
	}
	
	Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
	Ogre::HlmsDiskCache diskCache( hlmsManager );
	
	#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
	const size_t numThreads = std::max<size_t>( 1u, Ogre::PlatformInformation::getNumLogicalCores() );
	#endif
	
	for( size_t i=Ogre::HLMS_LOW_LEVEL + 1u; i<Ogre::HLMS_MAX; ++i ) {
		Ogre::Hlms* hlms = hlmsManager->getHlms( static_cast<Ogre::HlmsTypes>( i ) );
		if( hlms ) {
			filename = "hlmsDiskCache" + Ogre::StringConverter::toString( i ) + ".bin";
			try {
				if( cacheDirArch->exists( filename ) ) {
					Ogre::DataStreamPtr diskCacheFile = cacheDirArch->open( filename );
					diskCache.loadFrom( diskCacheFile );
					diskCache.applyTo(
						hlms
						#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
						, numThreads
						#endif
					);
				}
			} catch( Ogre::Exception& ) {
				LOG_WARNING(
					"Error loading cache from " + filename + "."
					"If you have issues, try deleting the file and restarting the app"
				);
			}
		}
	}
	archiveManager.unload(cacheDirArch);
	
}

void MGE::OgreHLMS::saveHLMSCache() {
	LOG_INFO("Save Microcode and HLMS Cache");
	
	std::filesystem::create_directory("cache");
	
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
	Ogre::Archive* cacheDirArch = archiveManager.load( "cache", "FileSystem", false );
	
 	Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
	Ogre::HlmsDiskCache diskCache( hlmsManager );
	
	for( size_t i=Ogre::HLMS_LOW_LEVEL + 1u; i<Ogre::HLMS_MAX; ++i ) {
		Ogre::Hlms* hlms = hlmsManager->getHlms( static_cast<Ogre::HlmsTypes>( i ) );
		if( hlms ) {
			diskCache.copyFrom( hlms );
			Ogre::DataStreamPtr diskCacheFile = cacheDirArch->create( "hlmsDiskCache" + Ogre::StringConverter::toString( i ) + ".bin" );
			diskCache.saveTo( diskCacheFile );
		}
	}
	if( Ogre::GpuProgramManager::getSingleton().isCacheDirty() ) {
		Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache(
			cacheDirArch->create( "microcodeCodeCache.cache" )
		);
	}
	archiveManager.unload(cacheDirArch);
	
}
