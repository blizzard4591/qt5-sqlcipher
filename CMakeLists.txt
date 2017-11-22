cmake_minimum_required(VERSION 3.0)
project(libqt5sql5-sqlcipher)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	SET(CMAKE_PREFIX_PATH "C:/Qt/5.9.2/msvc2017_64/lib/cmake/")
else()
	SET(CMAKE_PREFIX_PATH "C:/Qtx86/5.9.2/msvc2015/lib/cmake/")
endif()

find_package(Qt5Sql REQUIRED)

if (NOT WIN32)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
else()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_lib_suffix_win "x64")
	else()
		set(_lib_suffix_win "x86")
	endif()

	# Change this by hand if needed
	set(Libsqlcipher_INCLUDE_DIRS "C:/CppProjects/sqlcipher/include")
	# Change this by hand if needed
	set(Libsqlcipher_LIBRARIES "optimized;C:/CppProjects/sqlcipher/compile/${_lib_suffix_win}/Release/sqlite3.lib;debug;C:/CppProjects/sqlcipher/compile/${_lib_suffix_win}/Debug/sqlite3.lib")

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
set(QSQLCIPHER_COPYTO_DIR "" CACHE PATH "If set, the build artifact of the library will be copied there")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCC)
	# Set standard flags for GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

if(QSQLCIPHER_STATIC)
    set(LIBTYPE STATIC)
    add_definitions(-DQT_STATICPLUGIN)
    set(TEST_DIR test-static)
else()
    set(LIBTYPE MODULE)
	add_definitions(-DQT_PLUGIN)
    set(TEST_DIR test-shared)
endif()

if (Qt5Sql_FOUND)
	message(STATUS "Found Qt5 version ${Qt5Sql_VERSION}.")
	if (Qt5Sql_VERSION VERSION_LESS 5.8.0)
		set(DOWNLOAD_PATH "src/sql/drivers/sqlite")
	else()
		set(DOWNLOAD_PATH "src/plugins/sqldrivers/sqlite")
	endif()

	set(URL_HEADER "https://raw.githubusercontent.com/qt/qtbase/v${Qt5Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite_p.h")
	set(URL_CPP "https://raw.githubusercontent.com/qt/qtbase/v${Qt5Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite.cpp")
	message(STATUS "URL = ${URL_HEADER}")

	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qt-private")
	include_directories("${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}")

	file(DOWNLOAD "${URL_HEADER}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qt-private/qsql_sqlite_p.h" STATUS STATUS_HEADER_LIST TLS_VERIFY on LOG LOG_HEADER)
	file(DOWNLOAD "${URL_CPP}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qt-private/qsql_sqlite.cpp" STATUS STATUS_CPP_LIST TLS_VERIFY on LOG LOG_CPP)

	list(GET STATUS_HEADER_LIST 0 STATUS_HEADER)
	list(GET STATUS_CPP_LIST 0 STATUS_CPP)
	if ((NOT "${STATUS_HEADER}" STREQUAL "0") OR (NOT "${STATUS_CPP}" STREQUAL "0"))
		list(GET STATUS_HEADER_LIST 1 STATUS_HEADER_ERROR)
		list(GET STATUS_CPP_LIST 1 STATUS_CPP_ERROR)
		message(SEND_ERROR "Could not download required Qt5 private header/source files. Please report this error! Qt Version: ${Qt5Sql_VERSION}. Result Header: ${STATUS_HEADER}. Error: ${STATUS_HEADER_ERROR}. Result Source: ${STATUS_CPP}. Error: ${STATUS_CPP_ERROR}. Log Header: ${LOG_HEADER}. Log Source: ${LOG_CPP}.")
	else()
		message(STATUS "Downloaded required header/source files for your Qt5 version.")
	endif()
endif()

add_library(qsqlcipher ${LIBTYPE}
    smain.cpp
    "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qt-private/qsql_sqlite.cpp"
)

target_include_directories(qsqlcipher PRIVATE
    ${Qt5Sql_PRIVATE_INCLUDE_DIRS}
    ${SQLCIPHER_INCLUDE_DIRS} ${Libsqlcipher_INCLUDE_DIRS}
)

target_link_libraries(qsqlcipher
    Qt5::Sql
    ${SQLCIPHER_LIBRARIES} ${Libsqlcipher_LIBRARIES}
)

if (NOT ("${QSQLCIPHER_COPYTO_DIR}" STREQUAL ""))
	add_custom_command(TARGET qsqlcipher POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qsqlcipher> ${QSQLCIPHER_COPYTO_DIR}/$<TARGET_FILE_NAME:qsqlcipher>)
endif()

if (NOT WIN32)
	INSTALL(TARGETS qsqlcipher DESTINATION lib/qt5/plugins/sqldrivers)
else()
	INSTALL(TARGETS qsqlcipher DESTINATION sqldrivers)
endif()

#include(CTest)
if(QSQLCIPHER_BUILD_TESTS)
    add_subdirectory(${TEST_DIR})
endif()

# Packaging 
SET(CPACK_PACKAGE_VERSION "0.1.0")
SET(CPACK_PACKAGE_CONTACT "packaging@openmittsu.de")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5sql5, libsqlcipher0, qtbase5-dev, qtbase5-private-dev")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Qt5 wrapper for using SqlCipher as a QSqlDriver.")
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
 
INCLUDE(CPack)
