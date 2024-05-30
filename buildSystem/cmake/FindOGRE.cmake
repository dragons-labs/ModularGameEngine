include(FindPkgMacros)
include(PreprocessorUtils)

set(PKG_NAME OGRE)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path(${PKG_NAME}_INCLUDE_DIR NAMES Ogre.h PATH_SUFFIXES OGRE)
	find_library(${PKG_NAME}_LIBRARY  NAMES OgreMain libOgreMain)
endif()

if (OGRE_INCLUDE_DIR)
	file(READ "${OGRE_INCLUDE_DIR}/OgrePrerequisites.h" OGRE_TEMP_VERSION_CONTENT)
	has_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_NEXT_VERSION OGRE_NEXT_VERSION)
	get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MAJOR OGRE_VERSION_MAJOR)
	get_preprocessor_entry(OGRE_TEMP_VERSION_CONTENT OGRE_VERSION_MINOR OGRE_VERSION_MINOR)
	if (NOT ${OGRE_NEXT_VERSION} AND NOT (${OGRE_VERSION_MAJOR} EQUAL 2 OR ${OGRE_VERSION_MINOR} GREATER 1))
		message(FATAL_ERROR "build with Ogre < 2.1 is not supported")
		find_library(${PKG_NAME}_Terrain_LIBRARY  NAMES OgreTerrain libOgreTerrain)
		findpkg_addlib(${PKG_NAME} Terrain)
	else()
		find_library(${PKG_NAME}_HLMS_PBS_LIBRARY    NAMES OgreHlmsPbs libOgreHlmsPbs)
		find_library(${PKG_NAME}_HLMS_UNLIT_LIBRARY  NAMES OgreHlmsUnlit libOgreHlmsUnlit)
		findpkg_addlib(${PKG_NAME} HLMS_PBS)
		findpkg_addlib(${PKG_NAME} HLMS_UNLIT)
		find_path( ${PKG_NAME}_HLMS_INCLUDE_DIR
			NAMES
				OgreHlmsBufferManager.h
			PATHS
				${OGRE_INCLUDE_DIR}/Hlms/Common
			PATH_SUFFIXES
				OGRE/Hlms/Common
		)
		findpkg_addincdir(${PKG_NAME} HLMS)
		find_path( ${PKG_NAME}_HLMS_DIR
			NAMES
				Unlit/GLSL/PixelShader_ps.glsl
			NO_DEFAULT_PATH
			HINTS
				/usr/local/share/OGRE/Media/Hlms
				/usr/share/OGRE/Media/Hlms
				/usr/share/Ogre/Media/Hlms
		)
	endif()
	find_library(${PKG_NAME}_RENDER_SYSTEM  NAMES RenderSystem_GL3Plus.so PATH_SUFFIXES OGRE)
	find_library(${PKG_NAME}_PLUGIN_PARTICLEFX  NAMES Plugin_ParticleFX.so ParticleFX.so PATH_SUFFIXES OGRE)
endif()
findpkg_finish(${PKG_NAME})
