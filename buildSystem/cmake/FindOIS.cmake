include(FindPkgMacros)

set(PKG_NAME OIS)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path( ${PKG_NAME}_INCLUDE_DIR
		NAMES
			OIS.h
		PATH_SUFFIXES
			OIS
			ois
	)
	find_library( ${PKG_NAME}_LIBRARY
		NAMES
			OIS
			libOIS
	)
endif ()

findpkg_finish(${PKG_NAME})
