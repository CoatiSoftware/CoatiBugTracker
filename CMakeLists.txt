cmake_minimum_required(VERSION 3.8)

if (WIN32 AND (CMAKE_GENERATOR STREQUAL "Ninja"))
	set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
endif()
set(BUILD_CXX_LANGUAGE_PACKAGE OFF CACHE BOOL "Add C and C++ support to the Sourcetrail indexer.")
set(BUILD_JAVA_LANGUAGE_PACKAGE OFF CACHE BOOL "Add Java support to the Sourcetrail indexer.")
set(BUILD_PYTHON_LANGUAGE_PACKAGE OFF CACHE BOOL "Add Python support to the Sourcetrail indexer.")
set(DOCKER_BUILD OFF CACHE BOOL "Build runs in Docker")
set(TREAT_WARNINGS_AS_ERRORS ON CACHE BOOL "Treat compiler warnings as errors")

#set (CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/add_files.cmake)
include(cmake/create_source_groups.cmake)
include(cmake/version.cmake)
include(cmake/version_setup.cmake)
include(cmake/licenses.cmake)

# prohibit in-source-builds
if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(STATUS "In-source-builds are not allowed")
	message(STATUS "Clean your source directory (e.g. delete the CMakeCache.txt file)")
	message(FATAL_ERROR "Please create a separate build directory and call CMake again")
endif()

# speed up recompiling on unix with ccache
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
	message(STATUS "ccache found")

	# Support Unix Makefiles and Ninja
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


# Variables --------------------------------------------------------------------

set(PROJECT_NAME Sourcetrail)

set(APP_PROJECT_NAME "${PROJECT_NAME}")
set(APP_INDEXER_NAME "${PROJECT_NAME}_indexer")
set(LIB_UTILITY_PROJECT_NAME "${PROJECT_NAME}_lib_utility")
set(LIB_GUI_PROJECT_NAME "${PROJECT_NAME}_lib_gui")
set(LIB_CXX_PROJECT_NAME "${PROJECT_NAME}_lib_cxx")
set(LIB_JAVA_PROJECT_NAME "${PROJECT_NAME}_lib_java")
set(LIB_PYTHON_PROJECT_NAME "${PROJECT_NAME}_lib_python")
set(LIB_PROJECT_NAME "${PROJECT_NAME}_lib")
set(TEST_PROJECT_NAME "${PROJECT_NAME}_test")

if (WIN32)
	set(PLATFORM_INCLUDE "includesWindows.h")
elseif (APPLE)
	set(PLATFORM_INCLUDE "includesMac.h")
else ()
	set(PLATFORM_INCLUDE "includesLinux.h")
endif ()


# Project ----------------------------------------------------------------------

project(${PROJECT_NAME})

# set Standard build type to Release
set(CMAKE_BUILD_TYPE_INIT "Release")

#RPATH
if(UNIX AND NOT APPLE)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/lib/:$$ORIGIN/lib/")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)


# Settings ---------------------------------------------------------------------

if(WIN32)
	add_compile_definitions(NOMINMAX) # Prevent Windows.h from redefining min/max functions.
	add_compile_definitions(BOOST_USE_WINDOWS_H)
	# If clang-cl.exe ...
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
		add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # Clang-cl doesn't disable the warnings as boost expects.
	endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC") # Clang-cl.exe
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fms-compatibility-version=1926 -Xclang -fcxx-exceptions -Xclang -Wno-unknown-warning-option -Xclang -fcolor-diagnostics -Xclang -fvisibility-inlines-hidden -Xclang -fexceptions -Xclang -fcxx-exceptions")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option -fcolor-diagnostics -fvisibility-inlines-hidden")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	ADD_COMPILE_OPTIONS(/MP)
endif()

# FIXME: necessary to fix build in Docker
if (DOCKER_BUILD)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()

if (TREAT_WARNINGS_AS_ERRORS)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		# Visual Studio 2017 version 15.9 <= Version <= Visual Studio 2019 Version 16.4
		if ((MSVC_VERSION GREATER_EQUAL 1916) AND (MSVC_VERSION LESS_EQUAL 1924))
			# Warning 4003: not enough actual parameters for macro 'identifier'
			# Warning 4250: 'class1' inherits 'class2::member' via dominance
			set(WARNINGS_LIST "/wd4003 /wd4250")
			set(CMAKE_CXX_WARNINGS_FLAGS "/experimental:external /external:anglebrackets /external:W0 /WX")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_WARNINGS_FLAGS} ${WARNINGS_LIST}")

			# Treat linker warnings as errors
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
			set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /WX")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")

			message(STATUS "'Treat warnings as errors' policy is enabled")
		else()
			message(STATUS "'Treat warnings as errors' policy is disabled")
		endif()
	else()
		message(STATUS "'Treat warnings as errors' policy is disabled")
	endif()
endif()

# For debugging the release build on linux
#if (UNIX AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	#add_definitions(-fno-omit-frame-pointer)
#endif ()


# Clang ------------------------------------------------------------------------

if (BUILD_CXX_LANGUAGE_PACKAGE)

	find_package(LLVM REQUIRED)
	find_package(Clang REQUIRED)

	if (LLVM_FOUND)
		message(STATUS "Found LLVM ${LLVM_VERSION}")
	endif()

endif()


# Boost ------------------------------------------------------------------------

set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Set to ON to force the use of the static libraries.")
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.67 COMPONENTS system program_options filesystem date_time REQUIRED)
if(DEFINED BOOST_LIB_TOOLSET)
	add_compile_definitions(BOOST_LIB_TOOLSET="${BOOST_LIB_TOOLSET}")
endif()

# Qt ---------------------------------------------------------------------------

find_package(Qt5 COMPONENTS Widgets PrintSupport Network Svg REQUIRED)

if (WIN32)
	find_package(Qt5 COMPONENTS WinExtras REQUIRED)
endif()

if(Qt5Widgets_FOUND)
	message(STATUS "Found Qt ${Qt5Widgets_VERSION_STRING}")

	# FIX: Qt was built with -reduce-relocations
	if (Qt5_POSITION_INDEPENDENT_CODE)
		SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
	endif()
endif()


# Setup ---------------------------------------------------------------------------

if (WIN32)
	get_filename_component(QT_BINARY_DIR "${QT_MOC_EXECUTABLE}" PATH)
	set(QT_PLUGINS_DIR "${QT_BINARY_DIR}/../plugins")

	function(COPY_QT_BINARIES IS_DEBUG IS_APP)
		set(SUFFIX "")
		if (IS_DEBUG)
			set(SUFFIX "d")
		endif()

		set(CONFIGURATION "Release")
		if (IS_DEBUG)
			set(CONFIGURATION "Debug")
		endif()

		if(CMAKE_CL_64)
			set(BITS "64")
		else()
			set(BITS "32")
		endif()

		set(TARGET "test")
		if (IS_APP)
			set(TARGET "app")

			file(GLOB MY_PUBLIC_HEADERS
				"${CMAKE_SOURCE_DIR}/setup/dynamic_libraries/win${BITS}/app/${CONFIGURATION}/*"
			)
			file(COPY ${MY_PUBLIC_HEADERS} DESTINATION "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/")

		endif()

		configure_file("${QT_BINARY_DIR}/Qt5Core${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5Core${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt5Gui${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5Gui${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt5Network${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5Network${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt5Svg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5Svg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt5Widgets${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5Widgets${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt5WinExtras${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt5WinExtras${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/platforms/qwindows${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/platforms/qwindows${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/styles/qwindowsvistastyle${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/styles/qwindowsvistastyle${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/imageformats/qgif${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qgif${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qicns${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qicns${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qico${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qico${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qjpeg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qjpeg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qsvg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qsvg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qtga${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qtga${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qtiff${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qtiff${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qwbmp${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qwbmp${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qwebp${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qwebp${SUFFIX}.dll" COPYONLY)
	endfunction(COPY_QT_BINARIES)

	COPY_QT_BINARIES(True True)
	COPY_QT_BINARIES(True False)
	COPY_QT_BINARIES(False True)
	COPY_QT_BINARIES(False False)

	configure_file("${CMAKE_SOURCE_DIR}/setup/icon/windows/sourcetrail.ico" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/sourcetrail.ico" COPYONLY)

endif()


add_subdirectory(src/app)
add_subdirectory(src/external)
add_subdirectory(src/indexer)
add_subdirectory(src/lib)
add_subdirectory(src/lib_cxx)
add_subdirectory(src/lib_gui)
add_subdirectory(src/lib_java)
add_subdirectory(src/lib_python)
add_subdirectory(src/lib_utility)
add_subdirectory(src/test)


# Lib Utility ------------------------------------------------------------------

add_library(${LIB_UTILITY_PROJECT_NAME} ${LIB_UTILITY_FILES})

create_source_groups(${LIB_UTILITY_FILES})

set_property(
	TARGET ${LIB_UTILITY_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${LIB_UTILITY_INCLUDE_PATHS}"
)

target_include_directories(${LIB_UTILITY_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIR}
)

target_link_libraries(${LIB_UTILITY_PROJECT_NAME} ${Boost_LIBRARIES} Qt5::Widgets Qt5::Network)

if (UNIX AND NOT APPLE)
	find_package(Threads REQUIRED)
	target_link_libraries(${LIB_UTILITY_PROJECT_NAME} ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
endif()

if (WIN32)
	target_link_libraries(${LIB_UTILITY_PROJECT_NAME} Qt5::WinExtras)
endif()


# Lib --------------------------------------------------------------------------

set_source_files_properties(${EXTERNAL_FILES} PROPERTIES COMPILE_FLAGS "-w")
set_source_files_properties(${EXTERNAL_C_FILES} PROPERTIES COMPILE_FLAGS "-std=gnu89 -w")

add_library(${LIB_PROJECT_NAME} ${LIB_FILES} ${EXTERNAL_FILES} ${EXTERNAL_C_FILES})

create_source_groups(${LIB_FILES})
create_source_groups(${EXTERNAL_FILES})
create_source_groups(${EXTERNAL_C_FILES})

set_property(
	TARGET ${LIB_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${LIB_INCLUDE_PATHS}"
		"${LIB_GUI_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_CXX_INCLUDE_PATHS}"
		"${LIB_JAVA_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib"
)

target_include_directories(${LIB_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIR}
	"${EXTERNAL_INCLUDE_PATHS}"
	"${EXTERNAL_C_INCLUDE_PATHS}"
)

target_link_libraries(${LIB_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${LIB_GUI_PROJECT_NAME} ${Boost_LIBRARIES})

#configure language package defines
configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/language_packages.h.in"
	"${CMAKE_BINARY_DIR}/src/lib/language_packages.h"
)


# Lib Cxx ----------------------------------------------------------------------

if (BUILD_CXX_LANGUAGE_PACKAGE)

	add_library(${LIB_CXX_PROJECT_NAME} ${LIB_CXX_FILES})

	create_source_groups(${LIB_CXX_FILES})

	set_property(
		TARGET ${LIB_CXX_PROJECT_NAME}
		PROPERTY INCLUDE_DIRECTORIES
			"${LIB_CXX_INCLUDE_PATHS}"
			"${LIB_UTILITY_INCLUDE_PATHS}"
			"${LIB_INCLUDE_PATHS}"
			"${CMAKE_BINARY_DIR}/src/lib"
	)

	target_include_directories(${LIB_CXX_PROJECT_NAME} SYSTEM
		PUBLIC ${LLVM_INCLUDE_DIRS}
		${CLANG_INCLUDE_DIRS}
		${Boost_INCLUDE_DIRS}
		"${EXTERNAL_INCLUDE_PATHS}"
		"${EXTERNAL_C_INCLUDE_PATHS}"
	)

	link_directories(${LLVM_LIBRARY_DIRS} ${CLANG_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})

	llvm_map_components_to_libnames(REQ_LLVM_LIBS
		support core libdriver passes option
	)
	foreach(LLVM_TARGET ${LLVM_TARGETS_TO_BUILD})
		get_property(lib_deps GLOBAL PROPERTY "LLVMBUILD_LIB_DEPS_LLVM${LLVM_TARGET}CodeGen")
		list(APPEND REQ_LLVM_LIBS "LLVM${LLVM_TARGET}CodeGen")
		list(APPEND REQ_LLVM_LIBS "${lib_deps}")
		get_property(lib_deps GLOBAL PROPERTY "LLVMBUILD_LIB_DEPS_LLVM${LLVM_TARGET}AsmParser")
		if (NOT "${lib_deps}" STREQUAL "")
			list(APPEND REQ_LLVM_LIBS "LLVM${LLVM_TARGET}AsmParser")
			list(APPEND REQ_LLVM_LIBS "${lib_deps}")
		endif()
	endforeach()

	set(CLANG_LIBRARIES
		clangASTMatchers
		clangFrontend
		clangSerialization
		clangDriver
		clangTooling
		clangParse
		clangSema
		clangStaticAnalyzerFrontend
		clangStaticAnalyzerCheckers
		clangStaticAnalyzerCore
		clangAnalysis
		clangRewriteFrontend
		clangEdit
		clangAST
		clangLex
		clangBasic
	)

	target_link_libraries(${LIB_CXX_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${CLANG_LIBRARIES} ${REQ_LLVM_LIBS})

	if (WIN32)
		target_link_libraries(${LIB_CXX_PROJECT_NAME} version)
	endif()

else()

	message(STATUS "Building the Cxx indexer will be skipped. You can enable building this target by setting 'BUILD_CXX_LANGUAGE_PACKAGE' to 'ON'.")

endif()


# Lib Java ---------------------------------------------------------------------

if (BUILD_JAVA_LANGUAGE_PACKAGE)

	find_package(JNI)

	add_library(${LIB_JAVA_PROJECT_NAME} ${LIB_JAVA_FILES})

	create_source_groups(${LIB_JAVA_FILES})

	set_property(
		TARGET ${LIB_JAVA_PROJECT_NAME}
		PROPERTY INCLUDE_DIRECTORIES
			"${LIB_JAVA_INCLUDE_PATHS}"
			"${LIB_UTILITY_INCLUDE_PATHS}"
			"${LIB_INCLUDE_PATHS}"
			"${LIB_GUI_INCLUDE_PATHS}"
			"${CMAKE_BINARY_DIR}/src/lib"
	)

	target_include_directories(${LIB_JAVA_PROJECT_NAME} SYSTEM
		PUBLIC ${JNI_INCLUDE_DIRS}
		${Boost_INCLUDE_DIRS}
	)

	link_directories(${Boost_LIBRARY_DIRS})

	target_link_libraries(${LIB_JAVA_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${LIB_PROJECT_NAME})

	add_custom_command(
		TARGET ${LIB_JAVA_PROJECT_NAME}
		PRE_BUILD
		COMMAND ${PROJECT_SOURCE_DIR}/script/update_java_indexer.sh
		COMMENT "updating java indexer jars"
	)

else()

	message(STATUS "Building the Java indexer will be skipped. You can enable building this target by setting 'BUILD_JAVA_LANGUAGE_PACKAGE' to 'ON'.")

endif()


# Lib Python ---------------------------------------------------------------------

if (BUILD_PYTHON_LANGUAGE_PACKAGE)

	add_library(${LIB_PYTHON_PROJECT_NAME} ${LIB_PYTHON_FILES})

	create_source_groups(${LIB_PYTHON_FILES})

	set_property(
		TARGET ${LIB_PYTHON_PROJECT_NAME}
		PROPERTY INCLUDE_DIRECTORIES
			"${LIB_PYTHON_INCLUDE_PATHS}"
			"${LIB_UTILITY_INCLUDE_PATHS}"
			"${LIB_INCLUDE_PATHS}"
			"${CMAKE_BINARY_DIR}/src/lib"
	)

	#link_directories(${Boost_LIBRARY_DIRS})

	target_link_libraries(${LIB_PYTHON_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${LIB_PROJECT_NAME})

	add_custom_command(
		TARGET ${LIB_PYTHON_PROJECT_NAME}
		PRE_BUILD
		COMMAND ${PROJECT_SOURCE_DIR}/script/download_python_indexer.sh
		COMMENT "download python indexer"
	)

else()

	message(STATUS "Building the Python indexer will be skipped. You can enable building this target by setting 'BUILD_PYTHON_LANGUAGE_PACKAGE' to 'ON'.")

endif()


# Lib Gui ----------------------------------------------------------------------

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# target for running versionnumber script
# workaround for running customcommand (ninja dependency cycle)
add_custom_target(
	versionnumber ALL
)

add_library(${LIB_GUI_PROJECT_NAME} ${LIB_GUI_FILES} ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h)

target_link_libraries(${LIB_GUI_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${LIB_PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Svg)

if (WIN32)
	target_link_libraries(${LIB_GUI_PROJECT_NAME} Qt5::WinExtras)
endif()

# command for versioning script
add_custom_command(
	TARGET versionnumber
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -DBINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/version.cmake
	BYPRODUCTS ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h
	DEPENDS ${LIB_GUI_PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "check/update version number"
)
add_dependencies(${LIB_GUI_PROJECT_NAME} versionnumber)

create_source_groups(${LIB_GUI_FILES})

set_property(
	TARGET ${LIB_GUI_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${LIB_GUI_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib_gui"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)

# include external header without warnings
target_include_directories(${LIB_GUI_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	"${EXTERNAL_INCLUDE_PATHS}"
	"${EXTERNAL_C_INCLUDE_PATHS}"
)

# configure platform specific include file
configure_file(
	"${PROJECT_SOURCE_DIR}/src/lib_gui/platform_includes/includes.h.in"
	"${PROJECT_BINARY_DIR}/src/lib_gui/includes.h"
)

#configure the versioning file
configure_file(
	${CMAKE_SOURCE_DIR}/cmake/version.txt.in
	${CMAKE_BINARY_DIR}/version.txt
)

configure_file(
	${CMAKE_SOURCE_DIR}/cmake/productVersion.h.in
	${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h
)

set_property(SOURCE ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h PROPERTY SKIP_AUTOMOC ON)

set(CMAKE_AUTOMOC OFF)


# Indexer App ------------------------------------------------------------------

if (UNIX)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/app/")
else ()
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/app/")
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif ()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${APP_INDEXER_NAME} ${INDEXER_FILES})


set_target_properties(${APP_INDEXER_NAME} PROPERTIES OUTPUT_NAME sourcetrail_indexer)

if (WIN32)
	# hide the console when running a release build.
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /DEBUG:FASTLINK")
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES LINK_FLAGS_RELEASE "/ENTRY:\"mainCRTStartup\" /SUBSYSTEM:WINDOWS /DEBUG")
	set_target_properties(${APP_INDEXER_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

	# generate pdb for release build
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
endif ()

create_source_groups(${INDEXER_FILES})

target_link_libraries(
	${APP_INDEXER_NAME}
	${LIB_GUI_PROJECT_NAME}
	$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_PROJECT_NAME}>
	$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_PROJECT_NAME}>
	$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_PROJECT_NAME}>
	${LIB_PROJECT_NAME}
)

if (APPLE)
	find_library(CORE_FOUNDATION CoreFoundation)
	target_link_libraries(${APP_INDEXER_NAME} ${CORE_FOUNDATION})
endif ()

set_property(
	TARGET ${APP_INDEXER_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${APP_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_GUI_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib_gui"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)


# App --------------------------------------------------------------------------

if (UNIX)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/app/")
else ()
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/app/")
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif ()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
	file(WRITE ${CMAKE_BINARY_DIR}/Sourcetrail.rc
		"// Icon with lowest ID value placed first to ensure application icon\n"
		"// remains consistent on all systems.\n"
		"IDI_ICON1               ICON                    \"${CMAKE_BINARY_DIR}/Sourcetrail.ico\"\n"
	)

	add_executable(${APP_PROJECT_NAME} ${APP_FILES} ${CMAKE_BINARY_DIR}/Sourcetrail.rc)

	# also show the console when running a release build.
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /DEBUG:FASTLINK")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/ENTRY:\"mainCRTStartup\" /SUBSYSTEM:CONSOLE /DEBUG")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")

	# generate pdb for release build
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

	string(REGEX REPLACE "/" "\\\\" BACKSLASHED_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})
	string(REGEX REPLACE "/" "\\\\" BACKSLASHED_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})

	add_custom_command(
		TARGET ${APP_PROJECT_NAME}
		POST_BUILD
		COMMAND  rmdir "${BACKSLASHED_CMAKE_BINARY_DIR}\\Debug\\app\\data" & mklink /d /j "${BACKSLASHED_CMAKE_BINARY_DIR}\\Debug\\app\\data" "${BACKSLASHED_CMAKE_SOURCE_DIR}\\bin\\app\\data"
		COMMAND  rmdir "${BACKSLASHED_CMAKE_BINARY_DIR}\\Debug\\app\\user" & mklink /d /j "${BACKSLASHED_CMAKE_BINARY_DIR}\\Debug\\app\\user" "${BACKSLASHED_CMAKE_SOURCE_DIR}\\bin\\app\\user"
		COMMAND  rmdir "${BACKSLASHED_CMAKE_BINARY_DIR}\\Release\\app\\data" & mklink /d /j "${BACKSLASHED_CMAKE_BINARY_DIR}\\Release\\app\\data" "${BACKSLASHED_CMAKE_SOURCE_DIR}\\bin\\app\\data"
		COMMAND  rmdir "${BACKSLASHED_CMAKE_BINARY_DIR}\\Release\\app\\user" & mklink /d /j "${BACKSLASHED_CMAKE_BINARY_DIR}\\Release\\app\\user" "${BACKSLASHED_CMAKE_SOURCE_DIR}\\bin\\app\\user"
		COMMENT "create symbolic links for data"
	)
else ()
	add_executable(${APP_PROJECT_NAME} ${APP_FILES})
endif ()

create_source_groups(${APP_FILES})

target_link_libraries(
	${APP_PROJECT_NAME}
	${LIB_GUI_PROJECT_NAME}
	$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_PROJECT_NAME}>
	$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_PROJECT_NAME}>
	$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_PROJECT_NAME}>
	${LIB_PROJECT_NAME}
)

set_property(
	TARGET ${APP_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${APP_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_GUI_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib_gui"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)

target_include_directories(${APP_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	"${EXTERNAL_INCLUDE_PATHS}"
	"${EXTERNAL_C_INCLUDE_PATHS}"
)

# Use the Widgets module from Qt 5.
target_link_libraries(${APP_PROJECT_NAME} Qt5::Network Qt5::Widgets)

# add platform specific libraries
if (APPLE)
	find_library(CORE_FOUNDATION CoreFoundation)
	target_link_libraries(${APP_PROJECT_NAME} ${CORE_FOUNDATION})
endif ()

add_dependencies(${APP_PROJECT_NAME} ${APP_INDEXER_NAME})


# macOS Bundle ----------------------------------------------------------------

if (APPLE)

	set(MACOS_BUNDLE_NAME ${PROJECT_NAME})
	set(MACOS_BUNDLE_VERSION ${VERSION_STRING})
	set(MACOS_BINARY_NAME ${APP_PROJECT_NAME})
	set(MACOS_INDEXER_BINARY_NAME ${APP_INDEXER_NAME})

	get_property(QT_CORE_PATH TARGET ${Qt5Core_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_CORE_PATH ${QT_CORE_PATH} REALPATH)

	get_property(QT_GUI_PATH TARGET ${Qt5Gui_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_GUI_PATH ${QT_GUI_PATH} REALPATH)

	get_property(QT_WIDGETS_PATH TARGET ${Qt5Widgets_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_WIDGETS_PATH ${QT_WIDGETS_PATH} REALPATH)

	get_property(QT_PRINT_PATH TARGET ${Qt5PrintSupport_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_PRINT_PATH ${QT_PRINT_PATH} REALPATH)

	get_property(QT_NETWORK_PATH TARGET ${Qt5Network_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_NETWORK_PATH ${QT_NETWORK_PATH} REALPATH)

	get_property(QT_SVG_PATH TARGET ${Qt5Svg_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_SVG_PATH ${QT_SVG_PATH} REALPATH)

	list(APPEND MACOS_QT_FRAMEWORKS ${QT_CORE_PATH} ${QT_GUI_PATH} ${QT_WIDGETS_PATH} ${QT_PRINT_PATH} ${QT_NETWORK_PATH} ${QT_SVG_PATH})
	string(REPLACE ";" " " MACOS_QT_FRAMEWORKS "${MACOS_QT_FRAMEWORKS}")

	set(MACOS_BOOST_DIR "${Boost_INCLUDE_DIR}")
	STRING(REGEX REPLACE "/lib/cmake/clang" "" MACOS_CLANG_DIR "${Clang_DIR}")
	STRING(REGEX REPLACE "/lib/cmake/Qt5" "" MACOS_QT_DIR "${Qt5_DIR}")

	configure_file(
		${PROJECT_SOURCE_DIR}/setup/macOS/bundle_install.sh.in
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bundle_install.sh
		@ONLY
	)

	configure_file(
		${PROJECT_SOURCE_DIR}/setup/macOS/bundle_info.plist.in
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bundle_info.plist
		@ONLY
	)

endif ()


# Test ----------------------------------------------------------------------

if (UNIX)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/")
else ()
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/test/")
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif ()

add_executable (${TEST_PROJECT_NAME} ${TEST_FILES})

create_source_groups(${TEST_FILES})

target_link_libraries(
	${TEST_PROJECT_NAME}
	${LIB_GUI_PROJECT_NAME}
	$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_PROJECT_NAME}>
	$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_PROJECT_NAME}>
	$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_PROJECT_NAME}>
	${LIB_PROJECT_NAME}
	${LIB_GUI_PROJECT_NAME}
	$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_PROJECT_NAME}>
	$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_PROJECT_NAME}>
	$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_PROJECT_NAME}>
)

set_property(
	TARGET ${TEST_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${TEST_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_GUI_INCLUDE_PATHS}"
		"${EXTERNAL_INCLUDE_PATHS}"
		"${EXTERNAL_C_INCLUDE_PATHS}"
		"${Boost_INCLUDE_DIRS}"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)

if (WIN32 AND MSVC)
	set_target_properties(${TEST_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/bigobj")
	set_property(
		TARGET ${TEST_PROJECT_NAME}
			PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
			"${CMAKE_SOURCE_DIR}/bin/test")
endif ()




if (UNIX)
	# symlinks for data
	message(STATUS "create symlink: "
		"${CMAKE_SOURCE_DIR}/bin/app/data ->"
		"${CMAKE_BINARY_DIR}/app/data")
	execute_process(
		COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${CMAKE_BINARY_DIR}/app"
		COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${CMAKE_BINARY_DIR}/test"
		)
	execute_process(
		COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/bin/app/data" "${CMAKE_BINARY_DIR}/app/data"
		COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/bin/app/user" "${CMAKE_BINARY_DIR}/app/user"
		COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/bin/test/data" "${CMAKE_BINARY_DIR}/test/data"
	        )

	if (UNIX AND NOT APPLE)
		# Create different sized icons
		message(STATUS "create icons")
		execute_process(
			COMMAND "${CMAKE_SOURCE_DIR}/setup/Linux/createIcons.sh" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}"
		)

		# Linux package
		include(cmake/linux_package.cmake)
	endif ()
endif ()
