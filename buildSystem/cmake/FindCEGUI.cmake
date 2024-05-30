include(FindPkgMacros)
set(PKG_NAME CEGUI)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path( ${PKG_NAME}_INCLUDE_DIR
		NAMES
			CEGUI/CEGUI.h
		PATH_SUFFIXES
			cegui-1
			cegui-9999
			cegui-0
	)
	find_library( ${PKG_NAME}_LIBRARY
		NAMES
		libCEGUIBase
		libCEGUIBase-1
		libCEGUIBase-9999
		libCEGUIBase-0
		CEGUIBase 
		CEGUIBase-1
		CEGUIBase-9999
		CEGUIBase-0
	)
endif()

find_library( ${PKG_NAME}_OGRE_LIBRARY
	NAMES
		libCEGUIOgreRenderer
		libCEGUIOgreRenderer-1
		libCEGUIOgreRenderer-9999
		libCEGUIOgreRenderer-0
		CEGUIOgreRenderer
		CEGUIOgreRenderer-1
		CEGUIOgreRenderer-9999
		CEGUIOgreRenderer-0
	)
findpkg_addlib(${PKG_NAME} OGRE)

findpkg_finish(${PKG_NAME})
