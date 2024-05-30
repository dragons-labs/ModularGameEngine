include(FindPkgMacros)

set(PKG_NAME BTOGRE)
findpkg_begin(${PKG_NAME})

if (NOT ${PKG_NAME}_FOUND)
	find_path( ${PKG_NAME}_INCLUDE_DIR
		NAMES
			BtOgreExtras.h
		PATH_SUFFIXES
			btogre
			BtOgre21
	)
	find_library( ${PKG_NAME}_LIBRARY
		NAMES
			BtOgre
			libBtOgre
			libBtOgre21
			BtOgre21
		PATH_SUFFIXES
			btogre
			BtOgre21
		)
endif ()

findpkg_finish(${PKG_NAME})
