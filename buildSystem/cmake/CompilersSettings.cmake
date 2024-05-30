# get compiler name
get_filename_component(CXX_COMPILER_NAME "${CMAKE_CXX_COMPILER}" NAME)
message(STATUS "The C++ compiler: ${CXX_COMPILER_NAME}")

# set macro for setting C++ flags and other compiler related stuff for target

if ("${CXX_COMPILER_NAME}" MATCHES "^clang[+][+].*")
	macro(set_target_compile_options TARGET)
		target_compile_options(${TARGET} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
		target_compile_options(${TARGET} PRIVATE "-Wshadow") # warning about local variable shadow class variables
		target_compile_options(${TARGET} PRIVATE "-Wshadow-field-in-constructor") # warning about constructor parameters shadow class variables
		target_compile_options(${TARGET} PRIVATE "-Woverloaded-virtual") # warning about hides overloaded virtual function
		target_compile_options(${TARGET} PRIVATE "-Weffc++" "-Wnon-virtual-dtor") # warning about non-virtual destructor when class have virtual functions
		target_compile_options(${TARGET} PRIVATE "-Wmissing-noreturn") # warning about noreturn function without noreturn attribute
		target_compile_options(${TARGET} PRIVATE "-Wimplicit-fallthrough") # warning about unannotated fall-through between switch labels (to annotated use [[clang::fallthrough]];)
		target_compile_options(${TARGET} PRIVATE "-Wold-style-cast") # warning about old-style cast (C-style cast)
		target_compile_options(${TARGET} PRIVATE "-Wglobal-constructors") # warning about global constructor / destructor (static memebers in classes with non trival constructors)
		target_compile_options(${TARGET} PRIVATE "-Wreserved-id-macro") # warn about macros names start with _
		
		target_compile_options(${TARGET} PRIVATE "-Wno-unused-parameter") # no warning about unused parameter in functions (enabled by -Wextra)
		target_compile_options(${TARGET} PRIVATE "-Wno-vla" "-Wno-vla-extension") # no warning about C99 variable-length automatic arrays (enabled by -Wpedantic)
		
		# target_compile_options(${TARGET} PRIVATE "-Weverything")
		# target_compile_options(${TARGET} PRIVATE "-Wno-undefined-var-template") # no warning about unused parameter in templates
		# target_compile_options(${TARGET} PRIVATE "-Wno-unused-macros") # no warning about unused macro
		# target_compile_options(${TARGET} PRIVATE "-Wno-padded") # no warning about struct size padding
		# target_compile_options(${TARGET} PRIVATE "-Wno-covered-switch-default") # no warning about unnecessary default in switch
		# target_compile_options(${TARGET} PRIVATE "-Wno-switch-enum") # no warning about switch do not handled all enum values
		# target_compile_options(${TARGET} PRIVATE "-Wno-float-equal") # no warning about == or != floating point comparation
		# target_compile_options(${TARGET} PRIVATE "-Wno-exit-time-destructors") # no warning about exit time destructor (we use static variable in functions)
		# target_compile_options(${TARGET} PRIVATE "-Wno-double-promotion") # no warning about doble to float conversion
		# target_compile_options(${TARGET} PRIVATE "-Wno-shift-sign-overflow") # no warning about overflow after shift operation
		# target_compile_options(${TARGET} PRIVATE "-Wno-c++98-compat-pedantic" "-Wno-c++98-compat" "-Wno-c99-compat") # no warning about incompatibility with old standards ... we use C++11
		# target_compile_options(${TARGET} PRIVATE "-Wno-disabled-macro-expansion") # disabled because occurs on BOOST_PYTHON_FUNCTION_OVERLOADS macro ...
		# target_compile_options(${TARGET} PRIVATE "-Wno-undefined-func-template") # no warning about used undefined template function (this warning occurs when use undefined template function in other template)
		# target_compile_options(${TARGET} PRIVATE "-Wno-missing-prototypes") # no warning about "no previous prototype for function"
		# target_compile_options(${TARGET} PRIVATE "-Wno-deprecated") # no warning about deprecated features (eg. implicit copy constructor ... because it has a user-declared destructor)
		# target_compile_options(${TARGET} PRIVATE "-Wno-weak-vtables") # no warning on classes with virtual method and no translation unit
		# target_compile_options(${TARGET} PRIVATE "-Wno-conversion") # no warning about conversion (doble->float, unsigned<->signed, ...)
		# target_compile_options(${TARGET} PRIVATE "-Wno-documentation" "-Wno-documentation-unknown-command")  # disabled because wrong parse doxygen syntax (multiple error on correct syntax)
		
		target_compile_options(${TARGET} PRIVATE "-ftemplate-backtrace-limit=0")
		target_compile_options(${TARGET} PRIVATE "-fcolor-diagnostics")
	endmacro()
	set(TARGET_SYSTEM_IS_UNIX ON)
elseif ("${CXX_COMPILER_NAME}" STREQUAL "g++")
	macro(set_target_compile_options TARGET)
		target_compile_options(${TARGET} PRIVATE "-Wall")
		target_compile_options(${TARGET} PRIVATE "-Winline")
		target_compile_options(${TARGET} PRIVATE "-Wsuggest-override") # warning about suggest override
		target_compile_options(${TARGET} PRIVATE "-Wstrict-overflow=5")
		target_compile_options(${TARGET} PRIVATE "-Wsuggest-attribute=noreturn")
		target_compile_options(${TARGET} PRIVATE "-Wsuggest-attribute=const")
		target_compile_options(${TARGET} PRIVATE "-Wsuggest-attribute=pure")
	endmacro()
	set(TARGET_SYSTEM_IS_UNIX ON)
elseif ("${CXX_COMPILER_NAME}" STREQUAL "x86_64-w64-mingw32-g++")
	macro(set_target_compile_options TARGET)
		set(CMAKE_SYSTEM_NAME Windows)
		set(CMAKE_LINKER "x86_64-w64-mingw32-ld")
		set(CMAKE_FIND_ROOT_PATH "/04-Game-Dependencies-4-Windows/;/usr/x86_64-w64-mingw32/")
		set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
		set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
		set(CMAKE_INCLUDE_PATH "/04-Game-Dependencies-4-Windows/mingw64/include/;/usr/x86_64-w64-mingw32/include/")
		set(CMAKE_LIBRARY_PATH "/04-Game-Dependencies-4-Windows/mingw64/lib;/usr/x86_64-w64-mingw32/lib/")
		message(STATUS "Cross-compiling with " ${CXX_COMPILER_NAME} " for " ${CMAKE_SYSTEM_NAME})
		message(STATUS "  LD=" ${CMAKE_LINKER})
		message(STATUS "  CMAKE_INCLUDE_PATH=" ${CMAKE_INCLUDE_PATH})
		message(STATUS "  CMAKE_LIBRARY_PATH=" ${CMAKE_LIBRARY_PATH})
	endmacro()
	set(TARGET_SYSTEM_IS_WINDOWS ON)
else()
	message(FATAL_ERROR "Unsupported C++ compiler - can't set flags")
endif()
