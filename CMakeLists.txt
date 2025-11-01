cmake_minimum_required(VERSION 3.10)
project(libqt5sql5-sqlcipher)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")
set(QT_NO_PRIVATE_MODULE_WARNING ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	SET(CMAKE_PREFIX_PATH "C:/Qt/5.11.2/msvc2017_64/lib/cmake/")
else()
	SET(CMAKE_PREFIX_PATH "C:/Qtx86/5.11.2/msvc2015/lib/cmake/")
endif()

SET(QSQLCIPHER_CMAKE_SEARCH_PATH "C:/Qt/5.11.2/msvc2017_64" CACHE PATH "Additional Qt5 search path" )

message(STATUS "QSQLCIPHER_CMAKE_SEARCH_PATH: ${QSQLCIPHER_CMAKE_SEARCH_PATH}")
SET(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${QSQLCIPHER_CMAKE_SEARCH_PATH}/lib/cmake/")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql)

if (NOT WIN32)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
	if (DEFINED SQLCIPHER_LIBRARY_DIRS)
		link_directories(${SQLCIPHER_LIBRARY_DIRS})
		message(STATUS "Added ${SQLCIPHER_LIBRARY_DIRS} to link directories...")
	endif()
else()
	find_package(Libsqlcipher REQUIRED)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Arrange output paths so that the plugin is found in the default search path
# relative to the test binary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
if (NOT WIN32)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqldrivers)
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/sqldrivers)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/sqldrivers)
endif()

option(QSQLCIPHER_STATIC "Build plugin as a static library" OFF)
option(QSQLCIPHER_BUILD_TESTS "Build the test binary" ON)
option(QSQLCIPHER_INSTALL_INTO_QT_PLUGIN_DIRECTORY "Find the Qt plugin directory and install the driver there" OFF)
set(QSQLCIPHER_COPYTO_DIR "" CACHE PATH "If set, the build artifact of the library will be copied there")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCC)
	# Set standard flags for GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

if(QSQLCIPHER_STATIC)
	set(LIBTYPE STATIC)
	add_definitions(-DQT_STATICPLUGIN)
	set(TEST_DIR test-static)
else()
	if (APPLE)
		set(LIBTYPE SHARED)
	else()
		set(LIBTYPE MODULE)
	endif()
	add_definitions(-DQT_PLUGIN)
	set(TEST_DIR test-shared)
endif()

if (Qt${QT_VERSION_MAJOR}Sql_FOUND)
	message(STATUS "Found Qt${QT_VERSION_MAJOR} version ${Qt${QT_VERSION_MAJOR}Sql_VERSION}.")
	if (${QT_VERSION_MAJOR} EQUAL 6)
		find_package(Qt6 REQUIRED COMPONENTS SqlPrivate)
	endif()

	set(_PRIVATE_SOURCES_DIRECTORY "")
	if (EXISTS "${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
		message(STATUS "Using cached/shipped versions of required header/source files.")
		set(_PRIVATE_SOURCES_DIRECTORY "${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
		include_directories("${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
	elseif (CMAKE_BUILD_TYPE MATCHES "^Gentoo")
		# No network access during Gentoo package build
		message(SEND_ERROR "Header/source files for Qt${QT_VERSION_MAJOR} version ${Qt${QT_VERSION_MAJOR}Sql_VERSION} not found. Please report this error!")
	else()
		if (Qt${QT_VERSION_MAJOR}Sql_VERSION VERSION_LESS 5.8.0)
			set(DOWNLOAD_PATH "src/sql/drivers/sqlite")
		else()
			set(DOWNLOAD_PATH "src/plugins/sqldrivers/sqlite")
		endif()

		set(URL_HEADER "https://raw.githubusercontent.com/qt/qtbase/v${Qt${QT_VERSION_MAJOR}Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite_p.h")
		set(URL_CPP "https://raw.githubusercontent.com/qt/qtbase/v${Qt${QT_VERSION_MAJOR}Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite.cpp")
		message(STATUS "URL = ${URL_HEADER}")

		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
		set(_PRIVATE_SOURCES_DIRECTORY "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
		include_directories("${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt${QT_VERSION_MAJOR}Sql_VERSION}")

		file(DOWNLOAD "${URL_HEADER}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt${QT_VERSION_MAJOR}Sql_VERSION}/qsql_sqlite_p.h" STATUS STATUS_HEADER_LIST TLS_VERIFY on LOG LOG_HEADER)
		file(DOWNLOAD "${URL_CPP}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt${QT_VERSION_MAJOR}Sql_VERSION}/qsql_sqlite.cpp" STATUS STATUS_CPP_LIST TLS_VERIFY on LOG LOG_CPP)

		list(GET STATUS_HEADER_LIST 0 STATUS_HEADER)
		list(GET STATUS_CPP_LIST 0 STATUS_CPP)
		if ((NOT "${STATUS_HEADER}" STREQUAL "0") OR (NOT "${STATUS_CPP}" STREQUAL "0"))
			list(GET STATUS_HEADER_LIST 1 STATUS_HEADER_ERROR)
			list(GET STATUS_CPP_LIST 1 STATUS_CPP_ERROR)
			message(SEND_ERROR "Could not download required Qt${QT_VERSION_MAJOR} private header/source files. Please report this error! Qt Version: ${Qt${QT_VERSION_MAJOR}Sql_VERSION}. Result Header: ${STATUS_HEADER}. Error: ${STATUS_HEADER_ERROR}. Result Source: ${STATUS_CPP}. Error: ${STATUS_CPP_ERROR}. Log Header: ${LOG_HEADER}. Log Source: ${LOG_CPP}.")
		else()
			message(STATUS "Downloaded required header/source files for your Qt${QT_VERSION_MAJOR} version.")
		endif()
	endif()
endif()

#
# Determine Qt Plugin Location for Linux/Mac to allow installer generation to work
#
if (NOT WIN32)
	try_run(QSQLCIPHER_PLUGIN_DIR_TEST_RUN_RESULT QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_RESULT "${PROJECT_BINARY_DIR}/testPluginLocation" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/getPluginFolder.cpp" LINK_LIBRARIES Qt${QT_VERSION_MAJOR}::Core COMPILE_OUTPUT_VARIABLE QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_OUTPUT RUN_OUTPUT_VARIABLE QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT)
	if (NOT ${QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_RESULT})
		message(FATAL_ERROR "Failed to compile plugin location testing program, please contact a developer! Extended Information: ${QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_OUTPUT}") 
	elseif(NOT "${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_RESULT}" STREQUAL "0")
		message(FATAL_ERROR "Failed to run plugin location testing program, please contact a developer! Extended Information: ${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT}") 
	else()
		set(QSQLCIPHER_QT_PLUGIN_DIR "${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT}")
		message(STATUS "Determined plugin folder: ${QSQLCIPHER_QT_PLUGIN_DIR}/sqldrivers")
	endif()
endif()

if(QSQLCIPHER_INSTALL_INTO_QT_PLUGIN_DIRECTORY)
	if (NOT WIN32)
		try_run(QSQLCIPHER_PLUGIN_DIR_TEST_RUN_RESULT QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_RESULT "${PROJECT_BINARY_DIR}/testPluginLocation" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/getPluginFolder.cpp" LINK_LIBRARIES Qt${QT_VERSION_MAJOR}::Core COMPILE_OUTPUT_VARIABLE QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_OUTPUT RUN_OUTPUT_VARIABLE QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT)
		if (NOT ${QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_RESULT})
			message(FATAL_ERROR "Failed to compile plugin location testing program, please contact a developer! Extended Information: ${QSQLCIPHER_PLUGIN_DIR_TEST_COMPILE_OUTPUT}") 
		elseif(NOT "${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_RESULT}" STREQUAL "0")
			message(FATAL_ERROR "Failed to run plugin location testing program, please contact a developer! Extended Information: ${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT}") 
		else()
			set(QSQLCIPHER_PLUGIN_DIR "${QSQLCIPHER_PLUGIN_DIR_TEST_RUN_OUTPUT}")
			message(STATUS "QSQLCIPHER_INSTALL_INTO_QT_PLUGIN_DIRECTORY is set, will install plugin to folder ${QSQLCIPHER_PLUGIN_DIR}/sqldrivers")
		endif()
	else()
		string(REPLACE "\\" "/" Qt${QT_VERSION_MAJOR}Core_DIR "${Qt${QT_VERSION_MAJOR}Core_DIR}")
		if ("${Qt${QT_VERSION_MAJOR}Core_DIR}" STREQUAL "")
			message(SEND_ERROR "Qt${QT_VERSION_MAJOR}Core_DIR is not set and can not be used to derive the location of the Qt installation.")
		elseif(NOT "${Qt${QT_VERSION_MAJOR}Core_DIR}" MATCHES "^(.*)/lib/cmake/Qt${QT_VERSION_MAJOR}Core")
			message(SEND_ERROR "Qt${QT_VERSION_MAJOR}Core_DIR does not end in lib/cmake/Qt${QT_VERSION_MAJOR}Core, which is unexpected and therefore can not be used to derive the location of the Qt installation.")
		else()
			string(REGEX REPLACE "^(.*)/lib/cmake/Qt${QT_VERSION_MAJOR}Core" "\\1" QT${QT_VERSION_MAJOR}_BASE_DIR "${Qt${QT_VERSION_MAJOR}Core_DIR}")
			set(QSQLCIPHER_PLUGIN_DIR "${QT${QT_VERSION_MAJOR}_BASE_DIR}/plugins")
			message(STATUS "QSQLCIPHER_INSTALL_INTO_QT_PLUGIN_DIRECTORY is set, will install plugin to folder ${QSQLCIPHER_PLUGIN_DIR}/sqldrivers")
		endif()
	endif()
endif()

add_library(qsqlcipher ${LIBTYPE}
    smain.cpp
    "${_PRIVATE_SOURCES_DIRECTORY}/qsql_sqlite.cpp"
)

# We need QStringView -> QString conversions to happen
target_compile_definitions(qsqlcipher PRIVATE QT_USE_QSTRINGBUILDER)
target_compile_definitions(qsqlcipher PRIVATE QT_RESTRICTED_CAST_FROM_ASCII)
target_compile_definitions(qsqlcipher PRIVATE QT_NO_CAST_TO_ASCII)

target_include_directories(qsqlcipher PRIVATE
    ${Qt${QT_VERSION_MAJOR}Sql_PRIVATE_INCLUDE_DIRS}
    ${SQLCIPHER_INCLUDE_DIRS} ${Libsqlcipher_INCLUDE_DIRS}/sqlcipher
)

target_link_libraries(qsqlcipher
    Qt${QT_VERSION_MAJOR}::Sql
    ${SQLCIPHER_LIBRARIES} ${Libsqlcipher_LIBRARIES}
)

if (${QT_VERSION_MAJOR} EQUAL 6)
	target_link_libraries(qsqlcipher
		Qt${QT_VERSION_MAJOR}::SqlPrivate
	)
endif()

#include(CTest)
if(QSQLCIPHER_BUILD_TESTS)
    add_subdirectory(${TEST_DIR})
endif()

if (NOT ("${QSQLCIPHER_COPYTO_DIR}" STREQUAL ""))
	add_custom_command(TARGET qsqlcipher POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qsqlcipher> ${QSQLCIPHER_COPYTO_DIR}/$<TARGET_FILE_NAME:qsqlcipher>)
endif()

if (NOT ("${QSQLCIPHER_PLUGIN_DIR}" STREQUAL ""))
	add_custom_command(TARGET qsqlcipher POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qsqlcipher> ${QSQLCIPHER_PLUGIN_DIR}/sqldrivers/$<TARGET_FILE_NAME:qsqlcipher>)
endif()

find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if (NOT WIN32)
	if (DPKG_PROGRAM)
		if (NOT QSQLCIPHER_QT_PLUGIN_DIR)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				INSTALL(TARGETS qsqlcipher DESTINATION lib/x86_64-linux-gnu/qt${QT_VERSION_MAJOR}/plugins/sqldrivers)
			else()
				INSTALL(TARGETS qsqlcipher DESTINATION lib/i386-linux-gnu/qt${QT_VERSION_MAJOR}/plugins/sqldrivers)
			endif()
		else()
			string(REGEX REPLACE "^/usr/(.*)$" "\\1" QT_PLUGIN_DIR_SUFFIX "${QSQLCIPHER_QT_PLUGIN_DIR}")
			message(STATUS "Stripped plugin path to ${QT_PLUGIN_DIR_SUFFIX} from ${QSQLCIPHER_QT_PLUGIN_DIR}")
			INSTALL(TARGETS qsqlcipher DESTINATION "${QT_PLUGIN_DIR_SUFFIX}/sqldrivers")
		endif()
	elseif (APPLE)
		message(STATUS "Packaging for Mac OSX is done through homebrew, if you are not using it right now, you should.")
		INSTALL(TARGETS qsqlcipher DESTINATION sqldrivers)
		if (QSQLCIPHER_BUILD_TESTS)
			INSTALL(TARGETS qsqlcipher-test DESTINATION bin)
		endif()
	else()
		message(WARNING "Packaging is not set up for this platform, either submit a ticket or change/add pathes yourself, if packaging is required.")
		INSTALL(TARGETS qsqlcipher DESTINATION sqldrivers)
		if (QSQLCIPHER_BUILD_TESTS)
			INSTALL(TARGETS qsqlcipher-test DESTINATION bin)
		endif()
	endif()
else()
	INSTALL(TARGETS qsqlcipher DESTINATION sqldrivers)
endif()

# Packaging
SET(CPACK_PACKAGE_NAME "libqt${QT_VERSION_MAJOR}sql${QT_VERSION_MAJOR}-sqlcipher")
SET(CPACK_PACKAGE_CONTACT "packaging@openmittsu.de")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "3")
SET(CPACK_PACKAGE_VERSION_REVISION "1-${Qt${QT_VERSION_MAJOR}Sql_VERSION}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(DISTRO_CODENAME $ENV{DISTRO_CODENAME})
find_program(LSB_RELEASE_CMD lsb_release)
mark_as_advanced(LSB_RELEASE_CMD)
if (LSB_RELEASE_CMD)
	execute_process(COMMAND "${LSB_RELEASE_CMD}" --codename --short
			OUTPUT_VARIABLE DISTRO_CODENAME
			OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()
if (DISTRO_CODENAME)
	set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}~${DISTRO_CODENAME})
else ()
	message(STATUS "Could not find lsb_release nor is DISTRO_CODENAME set.")
endif ()

if (DPKG_PROGRAM)
	execute_process(COMMAND ${DPKG_PROGRAM} --print-architecture OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_VERSION_REVISION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
else (DPKG_PROGRAM)
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_VERSION_REVISION}_${CMAKE_SYSTEM_NAME}")
	SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif(DPKG_PROGRAM)

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}sql${QT_VERSION_MAJOR}, libsqlcipher0 (>= 3.4.1-1)")
SET(DEBIAN_PACKAGE_BUILDS_DEPENDS "debhelper (>= 9), cmake (>= 3), qtbase5-dev, qtbase5-private-dev, qtbase5-dev-tools, qtmultimedia5-dev, gcc, g++")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Qt${QT_VERSION_MAJOR} wrapper for using SqlCipher as a QSqlDriver.")
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
 
INCLUDE(CPack)
