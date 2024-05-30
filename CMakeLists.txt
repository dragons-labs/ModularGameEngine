cmake_minimum_required (VERSION 3.18)
cmake_policy(SET CMP0009 NEW)
cmake_policy(SET CMP0054 NEW)

# set path to additional cmake stuff
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/buildSystem/cmake")

# start project (we use only one project in main dir so CMAKE_SOURCE_DIR==PROJECT_SOURCE_DIR and CMAKE_BINARY_DIR==PROJECT_BINARY_DIR)
project("MGE" CXX)

# C++ standard is global, required settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++20 instead of -std=gnu++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Project ${CMAKE_PROJECT_NAME} source='${PROJECT_SOURCE_DIR}', binary='${PROJECT_BINARY_DIR}', make_module='${CMAKE_MODULE_PATH}'")


#
# generate BuildInfo
#

add_library(BuildInfo STATIC EXCLUDE_FROM_ALL "${PROJECT_BINARY_DIR}/buildInfo.cpp")
add_custom_target (BuildInfo.cpp
	COMMENT "Generating buildInfo.cpp"
	COMMAND ${PROJECT_SOURCE_DIR}/buildSystem/scripts/buildInfo.sh "${PROJECT_SOURCE_DIR}" > "${PROJECT_BINARY_DIR}/buildInfo.cpp"
	BYPRODUCTS "${PROJECT_BINARY_DIR}/buildInfo.cpp"
	VERBATIM
)
add_custom_command(TARGET BuildInfo POST_BUILD
	COMMAND touch -d "1970-01-01 00:00:00" "${PROJECT_BINARY_DIR}/libBuildInfo.a"
	VERBATIM
)
add_dependencies(BuildInfo BuildInfo.cpp)


#
# setting sources paths
#

# get paths to all source files
file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/engine-src/*.cpp")
file(GLOB_RECURSE Headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/engine-src/*.h")
file(GLOB_RECURSE PySources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/engine-src/*.py.cpp")
file(GLOB_RECURSE PyHeaders CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/engine-src/*.py.h")
file(GLOB TestsSources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/engine-tests/*.cpp")

if(TARGET_SYSTEM_IS_WINDOWS)
	list(REMOVE_ITEM Sources "${PROJECT_SOURCE_DIR}/engine-src/utils/misc/asioSyn.cpp")
	list(REMOVE_ITEM Sources "${PROJECT_SOURCE_DIR}/engine-src/modules/vncClient.cpp")
endif()

# add project include dirs
list(APPEND ProjectIncludeDirs 
	"${PROJECT_SOURCE_DIR}/engine-src" 
	"${PROJECT_SOURCE_DIR}/engine-src/core" 
	"${PROJECT_SOURCE_DIR}/engine-src/core/utils" 
	"${PROJECT_SOURCE_DIR}/engine-src/core/xmlConfigSystem" 
	"${PROJECT_BINARY_DIR}"
)


#
# setting project options
#

option(BUILD_TESTS                "Build unit tests with \"Boost unit test framework\"" ON)
set(MGE_COMPILE_OPTIONS           "-DMGE_DEBUG_LEVEL=2;-DMGE_DEBUG_SINGLETON=1" CACHE STRING "Compile option to set (mostly debug options - see config.h.ii for details, use `;` as separator)")

option(USE_BULLET                 "Use Bullet (if possible - final results is in 'config.h')" ON)
option(USE_BULLET_POSIX_THREADS   "Use Bullet POSIX multitthreading (if possible - final results is in 'config.h')" OFF)
set(BULLET_MAX_DISPATCHER_THREADS 0 CACHE STRING "Max number of Bullet dispatcher threads (0 == disable multitthreading)")
set(BULLET_MAX_SOLVER_THREADS     0 CACHE STRING "Max number of Bullet solver threads (0 == disable multitthreading)")

option(USE_OGGSOUND               "Use OgreOggSound (if possible - final results is in 'config.h')" ON)
option(USE_OGGVIDEO               "Use Ogre Theora Video Plugin (if possible - final results is in 'config.h')" ON)
option(USE_CEF                    "Use Chromium Embedded Framework (if possible - final results is in 'config.h')" ON)

#
# compiler related settings
#

include(CompilersSettings)


#
# dependencies checking - set LIBRARIES_INCLUDE_DIRS and LIBRARIES_BINARY_FILES list
#

include(PreprocessorUtils)
include(DependenciesSearch)

find_and_add_library(pugixml REQUIRED)

#find_and_add_library(PythonLibs MIN_VER=3.6 PREFIX=PYTHON REQUIRED)
find_and_add_library(python3-embed MIN_VER=3.6 PREFIX=PYTHON REQUIRED)
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})
find_and_add_library(pybind11 HEARER_ONLY PREFIX REQUIRED)

find_and_add_library(OGRE REQUIRED)
add_compile_options("-DOGRE_DEBUG_MODE=0") # WARNING: MUST be set to the same value as used to build Ogre !!!

find_and_add_library(OgreOggSound IF=USE_OGGSOUND)
add_compile_options("-DHAVE_EFX=0") # WARNING: MUST be set to the same value as used to build OgreOggSound !!!

find_and_add_library(OgreVideo IF=USE_OGGVIDEO)

find_and_add_library(OIS REQUIRED)

find_and_add_library(CEGUI REQUIRED)

if (${PKG_NAME}_INCLUDE_DIR)
	file(READ "${${PKG_NAME}_INCLUDE_DIR}/CEGUI/Version.h" CEGUI_TEMP_VERSION_CONTENT)
	get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_MAJOR  CEGUI_VERSION_MAJOR)
	get_preprocessor_entry(CEGUI_TEMP_VERSION_CONTENT CEGUI_VERSION_MINOR  CEGUI_VERSION_MINOR)
	if (${CEGUI_VERSION_MAJOR} LESS 1 AND ${CEGUI_VERSION_MINOR} LESS 9)
		message(FATAL_ERROR "Build with CEGUI 0.8.x or older is not supported.")
	endif()
	
	file(READ "${${PKG_NAME}_INCLUDE_DIR}/CEGUI/Config.h" CEGUI_TEMP_CONFIG_CONTENT)
	get_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_STRING_CLASS       CEGUI_STRING_CLASS)
	get_preprocessor_entry(CEGUI_TEMP_CONFIG_CONTENT CEGUI_STRING_CLASS_UTF_8 CEGUI_STRING_CLASS_UTF_8)
	if (NOT ${CEGUI_STRING_CLASS} EQUAL ${CEGUI_STRING_CLASS_UTF_8})
		message(WARNING "Using CEGUI with non UTF-8 String class is NOT tested.")
	endif()
endif()

if (UNIX)
	find_package(X11 REQUIRED)
	list(APPEND LIBRARIES_BINARY_FILES ${X11_X11_LIB})
endif ()

find_and_add_library(freetype2 REQUIRED)

find_and_add_library(PNG REQUIRED)

find_and_add_library(CEF IF=USE_CEF)
if (${USE_CEF})
	add_executable(
		cef_sub_process
		"${PROJECT_SOURCE_DIR}/resources-src/Programs/cef_sub_process.cpp"
	)
	target_link_libraries(cef_sub_process ${CEF_LIBRARIES})
	target_include_directories(cef_sub_process PUBLIC ${ProjectIncludeDirs})
	target_include_directories(cef_sub_process SYSTEM PUBLIC ${CEF_INCLUDE_DIR})
	# add_custom_command(TARGET cef_sub_process POST_BUILD
	# 	COMMAND cp -r ${CEF_PAKS_DIR}/*  ${PROJECT_BINARY_DIR}
	# 	COMMAND cp -r ${CEF_BLOBS_DIR}/* ${PROJECT_BINARY_DIR}
	# )
endif ()

find_and_add_library(BtOgre IF=USE_BULLET)
find_and_add_library(Bullet IF=USE_BULLET)
if (${USE_BULLET})
	find_library(BULLETIMPORTER_LIBRARY  NAMES BulletWorldImporter libBulletWorldImporter)
	if (NOT BULLETIMPORTER_LIBRARY)
		message(WARNING "Found Bullet but not BulletWorldImporter ...")
		set(USE_BULLET_IMPORTER OFF)
	else()
		set(USE_BULLET_IMPORTER ON)
		list(APPEND LIBRARIES_BINARY_FILES ${BULLETIMPORTER_LIBRARY})
	endif ()
	
	if (${USE_BULLET_POSIX_THREADS} AND UNIX)
		set(BULLET_USE_POSIX_THREADS TRUE)
	endif ()
endif ()


message(STATUS "Looking for Boost ...")
find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options)
check_and_add(Boost FALSE FALSE FALSE)

find_package(Boost 1.55.0 COMPONENTS stacktrace_backtrace)
if (Boost_STACKTRACE_BACKTRACE_LIBRARY)
	list(APPEND LIBRARIES_BINARY_FILES "${Boost_STACKTRACE_BACKTRACE_LIBRARY}" "libdl.a" "libbacktrace.a") # *.a need by stacktrace_backtrace
	set(BOOST_STACKTRACE_LINK TRUE)
endif()


# add some platform standard libraries ...
if(UNIX)
	list(APPEND LIBRARIES_BINARY_FILES m)
elseif(TARGET_SYSTEM_IS_WINDOWS)
	list(APPEND LIBRARIES_BINARY_FILES wsock32 ws2_32)
endif()

# add poco for support glob on Windows
if(TARGET_SYSTEM_IS_WINDOWS)
	find_library(POCO_LIBRARIES NAMES PocoFoundation)
	find_path(POCO_INCLUDE_DIRS NAMES Poco/Glob.h)
	check_and_add(POCO FALSE FALSE FALSE)
endif()


print_library_info()



#
# generate build-time configs, run-time configs and other run-time resources
#

# write build-time config files
configure_file( "${PROJECT_SOURCE_DIR}/config.h.ii" "${PROJECT_BINARY_DIR}/config.h" )
configure_file( "${PROJECT_SOURCE_DIR}/config-cef.h.ii" "${PROJECT_BINARY_DIR}/config-cef.h" )

# prepare run enviroment
execute_process(COMMAND ${PROJECT_SOURCE_DIR}/buildSystem/scripts/prepareRunDir.sh "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file( "${PROJECT_SOURCE_DIR}/resources-src/ConfigFiles/plugins.cfg.in"   "${PROJECT_BINARY_DIR}/conf/plugins.cfg" @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/resources-src/ConfigFiles/MGEConfig.xml.in" "${PROJECT_BINARY_DIR}/conf/MGEConfig.xml" @ONLY )


#
# generate MGE_Lib
#

add_library(MGE_Lib SHARED EXCLUDE_FROM_ALL ${Sources})
	# do not use static, because on static some `pybind11_init_impl_XXXX` functions (results of `PYBIND11_EMBEDDED_MODULE`) from .a can be missing in executable ELF file
set_target_compile_options(MGE_Lib)
target_include_directories(MGE_Lib PUBLIC ${ProjectIncludeDirs})
target_include_directories(MGE_Lib SYSTEM PUBLIC ${LIBRARIES_INCLUDE_DIRS})
target_compile_options(MGE_Lib PRIVATE -g ${MGE_COMPILE_OPTIONS})
# TODO use `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPYTHON_DOCSTRINGS_FILE='\$(notdir \$<)__docstrings.h'")` to support per `.py.cpp` file docstrings header
#      (via pybind11_mkdoc XXX.py.cpp -> XXX.py.cpp__docstrings.h included only headers used by XXX.py.cpp and its dependencies
target_link_options(MGE_Lib PRIVATE -g -rdynamic)
target_link_libraries(MGE_Lib ${LIBRARIES_BINARY_FILES})

#
# generate DocStringsForPythonAPI
#

find_program(PYBIND11_MKDOC pybind11-mkdoc PATHS ~/.local/bin/)
if(NOT PYBIND11_MKDOC)
    message(FATAL_ERROR "Could not find PYBIND11_MKDOC using the following names: pybind11-mkdoc.
    Try use `git clone https://github.com/pybind/pybind11_mkdoc ~/pybind11_mkdoc &&  pip3 install ~/pybind11_mkdoc/` to install pybind11_mkdoc.")
endif()


add_custom_target(DocStringsForPythonAPI
	COMMENT "Generating docstring for python api with pybind11_mkdoc"
	DEPENDS "${PROJECT_BINARY_DIR}/docstrings.h"
)

set(IncludeDirs_AsString "")
foreach(dir IN LISTS ProjectIncludeDirs LIBRARIES_INCLUDE_DIRS)
	set(IncludeDirs_AsString ${IncludeDirs_AsString} -I ${dir})
endforeach()

set(PyDocsHeaders "") # TODO (temporary?) hack – generate docs only from headers matching to .py.cpp or .py.h files
foreach(file IN LISTS PySources)
	string(REPLACE ".py.cpp" ".h" docFile ${file})
	if(EXISTS ${docFile})
		set(PyDocsHeaders ${PyDocsHeaders} ${docFile})
	endif()
endforeach()
foreach(file IN LISTS PyHeaders)
	string(REPLACE ".py.h" ".h" docFile ${file})
	if(EXISTS ${docFile})
		set(PyDocsHeaders ${PyDocsHeaders} ${docFile})
	endif()
endforeach()

add_custom_command(
	OUTPUT "${PROJECT_BINARY_DIR}/docstrings.h"
	# if docstrings.h.tmp is missing or outdated rebuild it
	COMMAND 
		[ "${PROJECT_BINARY_DIR}/docstrings.h.tmp" -nt "${PROJECT_BINARY_DIR}/docstrings.stamp" ] ||
		${PYBIND11_MKDOC} -o "${PROJECT_BINARY_DIR}/docstrings.h.tmp" -std=c++20 ${IncludeDirs_AsString} -D __PYTHON_DOCUMENTATION_GENERATOR__ -D MGE_DEBUG_LEVEL=0 -w999 ${PyDocsHeaders}
	# only when docstrings.h and docstrings.h.tmp are different replace/create docstrings.h as a copy of docstrings.h.tmp
	COMMAND
		diff "${PROJECT_BINARY_DIR}/docstrings.h.tmp" "${PROJECT_BINARY_DIR}/docstrings.h" >/dev/null 2>&1 ||
		cp -a "${PROJECT_BINARY_DIR}/docstrings.h.tmp" "${PROJECT_BINARY_DIR}/docstrings.h"
	DEPENDS "${PROJECT_BINARY_DIR}/docstrings.stamp"
)

add_custom_command(
	OUTPUT "${PROJECT_BINARY_DIR}/docstrings.stamp"
	COMMAND touch "${PROJECT_BINARY_DIR}/docstrings.stamp"
	DEPENDS ${PySources} # In fact DEPENDS ${Headers}, but rebuild this (and all *.py.cpp) due to change only in header file (without change python api) is senseless ...
)
add_dependencies(MGE_Lib DocStringsForPythonAPI)


#
# generate swigpyrun.h
#

find_program(SWIG swig REQUIRED)

add_custom_target(Binding_for_SWIG_API
	DEPENDS "${PROJECT_BINARY_DIR}/swigpyrun.h"
)
add_custom_command (
	OUTPUT "${PROJECT_BINARY_DIR}/swigpyrun.h"
	COMMAND ${SWIG} -python -external-runtime
)
add_dependencies(MGE_Lib Binding_for_SWIG_API)

#
# main executables with runtime dependencies
#

add_executable(Game "${PROJECT_SOURCE_DIR}/main.cpp")
target_compile_options(Game PRIVATE -g -DMGE_DEBUG_LEVEL=0)
target_link_libraries(Game MGE_Lib BuildInfo ${LIBRARIES_BINARY_FILES})

# preprocess compositor scripts as dependency of main executable
add_custom_command(
	OUTPUT  ${PROJECT_BINARY_DIR}/OgreCompositor/.compositor.timestamp
	COMMENT "Preprocessing compositor scripts"
	COMMAND ${PROJECT_SOURCE_DIR}/buildSystem/scripts/preprocessCompositorScripts.sh "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/OgreCompositor"
	DEPENDS ${PROJECT_SOURCE_DIR}/resources-src/OGRE_compositor/*.compositor ${PROJECT_SOURCE_DIR}/buildSystem/scripts/preprocessCompositorScripts.sh
)
add_custom_target(PreprocessCompositorScripts
	DEPENDS ${PROJECT_BINARY_DIR}/OgreCompositor/.compositor.timestamp
)
add_dependencies(Game PreprocessCompositorScripts)


#
# documentation
#

add_custom_target(docs ALL)

# add pyDoc target with generating writePyDoc executables
if(NOT ${CMAKE_CROSSCOMPILING})
	add_custom_target(writePyDoc.py
		COMMAND ${PROJECT_SOURCE_DIR}/buildSystem/scripts/preparePyDoc.sh "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
	)
	
	add_custom_target(pyDoc
		COMMAND mkdir -p ${PROJECT_BINARY_DIR}/Documentation/python
		COMMAND rm ${PROJECT_BINARY_DIR}/Documentation/python/*.* 2> /dev/null || true
		COMMAND cd ${PROJECT_BINARY_DIR} && ./Game --exec-script writePyDoc.py || true
		DEPENDS writePyDoc.py Game
	)
	
	add_dependencies(docs pyDoc)
endif ()

# add doxygen target
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${PROJECT_SOURCE_DIR}/buildSystem/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doxygen
		COMMENT "Generating API documentation with Doxygen"
		COMMAND ${PROJECT_SOURCE_DIR}/buildSystem/scripts/doxygen.sh "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" "${DOXYGEN_EXECUTABLE}"
	)
	add_dependencies(docs doxygen)
endif()


#
# unit tests
#

if (${BUILD_TESTS})
	find_package(Boost 1.55.0 COMPONENTS unit_test_framework)
	if (Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
		enable_testing()
		
		message(STATUS "Add unit tests:")
		foreach(test_src IN LISTS TestsSources)
			get_filename_component(test_exec ${test_src} NAME_WE)
			set(test_exec "test_${test_exec}")
			message(STATUS "* ${test_exec}")
			
			add_executable(${test_exec} "${test_src}")
			target_include_directories(${test_exec} PRIVATE ${Boost_INCLUDE_DIRS})
			target_link_libraries(${test_exec} MGE_Lib BuildInfo ${LIBRARIES_BINARY_FILES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
			target_compile_options(${test_exec} PRIVATE -g -DMGE_DEBUG_LEVEL=0)
			add_test(NAME ${test_exec} COMMAND ${test_exec} --log_level=test_suite)
		endforeach()
		
		add_library(swig_test SHARED EXCLUDE_FROM_ALL "${PROJECT_SOURCE_DIR}/engine-tests/python/python_swig.py.cpp")
		target_include_directories(swig_test PRIVATE ${ProjectIncludeDirs} ${LIBRARIES_INCLUDE_DIRS})
		target_link_libraries(swig_test ${OGRE_LIBRARY})
		
		add_custom_command(TARGET swig_test POST_BUILD
			COMMAND ln -sf "${PROJECT_BINARY_DIR}/libswig_test.so" "${PROJECT_BINARY_DIR}/swig_test.so"
		)
		add_dependencies(swig_test Binding_for_SWIG_API)
		add_dependencies(test_scriptsSystem swig_test)
		add_custom_command(TARGET test_scriptsSystem POST_BUILD
			COMMAND ln -sf "${PROJECT_SOURCE_DIR}/engine-tests/python/*.py" "${PROJECT_BINARY_DIR}/"
		)
	else()
		set(BUILD_TESTS OFF)
	endif()
endif()


#
# full target
#

add_custom_target(full)
add_dependencies(full Game)
add_dependencies(full docs)
if (${USE_CEF})
	add_dependencies(full cef_sub_process)
endif()
