include(FindPkgMacros)

set(PKG_NAME CEF)
findpkg_begin(${PKG_NAME})

find_path( ${PKG_NAME}_RESOURCES_DIR_PATH
	NAMES
		chrome_100_percent.pak
	PATHS
		/usr/local/lib
		/usr/lib
		/usr/local/share
		/usr/share
	PATH_SUFFIXES
		cef
		cef_binary
		CEF
		CEF_BINARY
	NO_DEFAULT_PATH
)

message(STATUS "${PKG_NAME} resources directory path is: ${${PKG_NAME}_RESOURCES_DIR_PATH}")

if (NOT ${PKG_NAME}_FOUND)
	find_path(${PKG_NAME}_INCLUDE_DIR NAMES cef_client.h PATH_SUFFIXES cef)
	find_library(${PKG_NAME}_LIBRARY  PATHS ${${PKG_NAME}_RESOURCES_DIR_PATH} NAMES cef libcef)
endif ()

message(STATUS "${PKG_NAME} library path is: ${${PKG_NAME}_LIBRARY}")

find_library(${PKG_NAME}_dllWrapper_LIBRARY  NAMES cef_dll_wrapper libcef_dll_wrapper)
findpkg_addlib(${PKG_NAME} dllWrapper)

findpkg_finish(${PKG_NAME})
