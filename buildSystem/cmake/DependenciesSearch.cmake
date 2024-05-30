# Copyright (c) 2013-2022 Robert Ryszard Paciorek <rrp@opcode.eu.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

include(FindPkgConfig)

#
# _LIBRARIES and _INCLUDEDIR standardization
# set _FOUND variable
#
macro(check PREFIX HEARER_ONLY)
	if (${PREFIX}_LINK_LIBRARIES)
		set(${PREFIX}_LIBRARIES ${${PREFIX}_LINK_LIBRARIES})
	endif ()
	if (${PREFIX}_INCLUDEDIR AND NOT ${PREFIX}_INCLUDE_DIRS)
		set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDEDIR})
	endif ()
	
	if (${PREFIX}_INCLUDE_DIRS AND (${PREFIX}_LIBRARIES OR ${HEARER_ONLY}))
		set(${PREFIX}_FOUND TRUE)
	else()
		set(${PREFIX}_FOUND FALSE)
	endif ()
endmacro(check)

#
# check _LIBRARIES and _INCLUDEDIR (vs REQUIRED_INC REQUIRED_LIB)
# update LIBRARIES_INCLUDE_DIRS and LIBRARIES_BINARY_FILES lists
# set _FOUND variable
#
macro(check_and_add PREFIX REQUIRED_INC REQUIRED_LIB HEARER_ONLY)
	if ((REQUIRED_INC AND NOT ${PREFIX}_INCLUDE_DIRS) OR (REQUIRED_LIB AND NOT ${PREFIX}_LIBRARIES))
		message(FATAL_ERROR
			"\n"
			"Required library ${PREFIX} not found! "
			"Install the library (including dev packages) and try again."
			"\n"
			"If the library is already installed, set the missing variables"
			"(${PREFIX}_INCLUDE_DIR and ${PREFIX}_LIBRARY) manually in cmake"
			"(via 'CMakeCache.txt' file, `cmake -D command .` or `ccmake .` command)."
			"\n"
		)
	endif ()
	
	if (${PREFIX}_INCLUDE_DIRS AND (${PREFIX}_LIBRARIES OR ${HEARER_ONLY}))
		list(REMOVE_DUPLICATES ${PREFIX}_INCLUDE_DIRS)
		list(REMOVE_DUPLICATES ${PREFIX}_LIBRARIES)
		message(STATUS "[Searching for ${PREFIX}] found (include_dir = ${${PREFIX}_INCLUDE_DIRS}, lib = ${${PREFIX}_LIBRARIES})" )
		set(${PREFIX}_FOUND TRUE)
		
		message(STATUS "[Searching for ${PREFIX}] update libraries and include directories lists" )
		if (${PREFIX}_INCLUDE_DIRS)
			set(${PREFIX}_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS} CACHE STRING "include dirs (will be use as \"system\" include) for ${PREFIX}" FORCE)
			list(APPEND LIBRARIES_INCLUDE_DIRS ${${PREFIX}_INCLUDE_DIRS})
		endif ()
		if (${PREFIX}_LIBRARIES AND NOT ${HEARER_ONLY})
			set(${PREFIX}_LIBRARIES ${${PREFIX}_LIBRARIES} CACHE STRING "libraries (full path to file or -l option args) for ${PREFIX}" FORCE)
			list(APPEND LIBRARIES_BINARY_FILES ${${PREFIX}_LIBRARIES})
		endif ()
	endif ()
endmacro(check_and_add)

#
# do search and call check_and_add
#
macro(find_and_add PREFIX LIBNAME REQUIRED_INC REQUIRED_LIB_RAW HEARER_ONLY VERSION)
	# set REQUIRED_LIB respecting HEARER_ONLY
	if (${HEARER_ONLY})
		set(REQUIRED_LIB FALSE)
	else ()
		set(REQUIRED_LIB ${REQUIRED_LIB_RAW})
	endif ()
	
	# start search, set _FOUND to false => always (re)check
	message(STATUS "[Searching for ${PREFIX}] libname=${LIBNAME}, required headers=${REQUIRED_INC}, required lib=${REQUIRED_LIB}, minimum version = ${VERSION}, heareronly=${HEARER_ONLY}) ...")
	set(${PREFIX}_FOUND FALSE)
	
	# try use Find${LIBNAME}.cmake for find_package module search
	if (NOT ${${PREFIX}_FOUND})
		message(STATUS "[Searching for ${PREFIX}] use find_package by module ...")
		if (${VERSION})
			find_package(${LIBNAME} ${VERSION} QUIET MODULE)
		else ()
			find_package(${LIBNAME} QUIET MODULE)
		endif ()
		check(${PREFIX} ${HEARER_ONLY})
	endif ()
	
	# try use pkg
	if (NOT ${${PREFIX}_FOUND})
		message(STATUS "[Searching for ${PREFIX}] use pkg ...")
		if (${VERSION})
			pkg_check_modules(${PREFIX} ${LIBNAME}>=${VERSION} QUIET)
		else ()
			pkg_check_modules(${PREFIX} ${LIBNAME} QUIET)
		endif ()
		check(${PREFIX} ${HEARER_ONLY})
	endif ()
	
	# if still none try use config mode of find_package
	if (NOT ${${PREFIX}_FOUND})
		message(STATUS "[Searching for ${PREFIX}] use find_package by config ...")
		if (${VERSION})
			find_package(${LIBNAME} ${VERSION} QUIET CONFIG)
		else ()
			find_package(${LIBNAME} QUIET CONFIG)
		endif ()
		check(${PREFIX} ${HEARER_ONLY})
	endif ()
	
	check_and_add(${PREFIX} ${REQUIRED_INC} ${REQUIRED_LIB} ${HEARER_ONLY})
endmacro(find_and_add)


#
# do search and add results to LIBRARIES_INCLUDE_DIRS and LIBRARIES_BINARY_FILES lists
# 
# when call with:
#  * REQUIRED and not found generate fatal error
#    (can be call with REQUIRED=includes or REQUIRED=libs for search only include files or libs)
#  * HEARER_ONLY don't serach libs (only headers)
#  * IF=var serch only if var is true and set var to false when not found
#  * PREFIX don't use LIBNAME as prefix without uppercase it
#    (when call with arg as PREFIX=name use name as prefix (without uppercase it))
#  * MIN_VER=x set minimum version to x
#
macro(find_and_add_library LIBNAME)
	# parse optional args
	set(REQUIRED_INC FALSE)
	set(REQUIRED_LIB FALSE)
	set(HEARER_ONLY  FALSE)
	set(IFMODE FALSE)
	set(IFVAR "")
	set(PREFIX "")
	set(VERSION FALSE)
	foreach (loop_var ${ARGN})
		string(REGEX MATCH "^.*=" ArgName "${loop_var}")
		if ("${ArgName}" STREQUAL "")
			set(ArgName "${loop_var}")
			set(ArgVal  "")
		else()
			string(REPLACE "=" "" ArgName "${ArgName}")
			string(REPLACE "${ArgName}=" "" ArgVal "${loop_var}")
		endif()
		
		if ("${ArgName}" STREQUAL "REQUIRED")
			if ("${ArgVal}" STREQUAL "includes")
				set(REQUIRED_INC TRUE)
			elseif ("${ArgVal}" STREQUAL "libs")
				set(REQUIRED_LIB TRUE)
			else ()
				set(REQUIRED_INC TRUE)
				set(REQUIRED_LIB TRUE)
			endif ()
		endif ()
		if ("${ArgName}" STREQUAL "HEARER_ONLY")
			set(HEARER_ONLY  TRUE)
		endif ()
		if ("${ArgName}" STREQUAL "PREFIX")
			if ("${ArgVal}" STREQUAL "")
				set(PREFIX "${LIBNAME}")
			else ()
				set(PREFIX "${ArgVal}")
			endif ()
		endif ()
		if ("${ArgName}" STREQUAL "IF")
			set(IFMODE TRUE)
			set(IFVAR ${ArgVal})
		endif ()
		if ("${ArgName}" STREQUAL "MIN_VER")
			set(VERSION ${ArgVal})
		endif ()
	endforeach ()
	
	if ("${PREFIX}" STREQUAL "")
		# by default use uppercase LIBNAME for variables and raw LIBNAME for find_package/pkg_check_modules
		string(TOUPPER ${LIBNAME} PREFIX)
	endif ()
	
	if (NOT ${IFMODE})
		find_and_add(${PREFIX} ${LIBNAME} ${REQUIRED_INC} ${REQUIRED_LIB} ${HEARER_ONLY} ${VERSION})
	else ()
		if (${IFVAR})
			find_and_add(${PREFIX} ${LIBNAME} FALSE FALSE ${HEARER_ONLY} ${VERSION})
			if (NOT ${PREFIX}_FOUND)
				set(${IFVAR} OFF)
			endif ()
		else ()
			message(STATUS "[Searching for ${LIBNAME}] disabled by ${IFVAR}=${${IFVAR}}")
		endif ()
	endif ()
endmacro(find_and_add_library LIBNAME)

#
# print LIBRARIES_INCLUDE_DIRS and LIBRARIES_BINARY_FILES lists
# 
macro(print_library_info)
	list(REMOVE_DUPLICATES LIBRARIES_INCLUDE_DIRS)
	message(STATUS "LIBRARIES_INCLUDE_DIRS:")
	foreach(dir ${LIBRARIES_INCLUDE_DIRS})
		message(STATUS "  ${dir}")
	endforeach()

	if(TARGET_SYSTEM_IS_UNIX)
		list(REMOVE_DUPLICATES LIBRARIES_BINARY_FILES)
	endif()
	message(STATUS "LIBRARIES_BINARY_FILES:")
	foreach(lib ${LIBRARIES_BINARY_FILES})
		message(STATUS "  ${lib}")
	endforeach()
endmacro(print_library_info)
