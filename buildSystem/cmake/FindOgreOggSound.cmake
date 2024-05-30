include(FindPkgMacros)

set(PKG_NAME OGREOGGSOUND)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path( ${PKG_NAME}_INCLUDE_DIR
		NAMES
			OgreOggSound.h
		PATHS
			${OGRE_INCLUDE_DIR}/Plugins/OggSound
			${OGRE_INCLUDE_DIR}/OgreOggSound
			${OGRE_INCLUDE_DIR}/OggSound
		PATH_SUFFIXES
			OgreOggSound
			OGRE/OgreOggSound
			OGRE/OggSound
	)
	find_library( ${PKG_NAME}_LIBRARY
		NAMES
			Plugin_OggSound
			Plugin_OggSound.so
			OgreOggSound
		PATH_SUFFIXES
			OGRE
	)
endif ()

find_path( ${PKG_NAME}_OpenAL_INCLUDE_DIR
	NAMES
		al.h
	PATH_SUFFIXES
		AL
)
findpkg_addincdir(${PKG_NAME} OpenAL)

findpkg_finish(${PKG_NAME})
