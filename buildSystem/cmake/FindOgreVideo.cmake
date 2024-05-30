include(FindPkgMacros)

set(PKG_NAME OGREVIDEO)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path( ${PKG_NAME}_INCLUDE_DIR
		NAMES
			OgreVideoManager.h
		PATHS
			${OGRE_INCLUDE_DIR}/Plugins/Theora
			${OGRE_INCLUDE_DIR}/Plugins/TheoraVideo
			${OGRE_INCLUDE_DIR}/Plugins/Video
		PATH_SUFFIXES
			ogre-video
	)
	find_library( ${PKG_NAME}_LIBRARY
		NAMES
			Plugin_TheoraVideoSystem
			Plugin_TheoraVideoSystem.so
			TheoraVideoSystem
			libTheoraVideoSystem
		PATH_SUFFIXES
			OGRE
	)
endif ()

find_library(${PKG_NAME}_THEORAPLAYER_LIBRARY   NAMES theoraplayer libtheoraplayer PATH_SUFFIXES OGRE)
findpkg_addlib(${PKG_NAME} THEORAPLAYER)

findpkg_finish(${PKG_NAME})
