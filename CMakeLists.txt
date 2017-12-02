cmake_minimum_required(VERSION 3.0)
project(qsqlcipher)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")
SET(CMAKE_PREFIX_PATH "D:/Qt/5.9.2/msvc2017_64/lib/cmake/")

find_package(Qt5Sql REQUIRED)
find_package(PkgConfig REQUIRED)

if (NOT WIN32)
	pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
else()
	find_package(Libsqlcipher REQUIRED)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Arrange output paths so that the plugin is found in the default search path
# relative to the test binary.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sqldrivers)

option(QSQLCIPHER_STATIC "Build plugin as a static library" OFF)

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
	
	set(_PRIVATE_SOURCES_DIRECTORY "")
	if (EXISTS "${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt5Sql_VERSION}")
		message(STATUS "Using cached/shipped versions of required header/source files.")
		set(_PRIVATE_SOURCES_DIRECTORY "${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt5Sql_VERSION}")
		include_directories("${PROJECT_SOURCE_DIR}/qt-file-cache/${Qt5Sql_VERSION}")
	else()
		if (Qt5Sql_VERSION VERSION_LESS 5.8.0)
			set(DOWNLOAD_PATH "src/sql/drivers/sqlite")
		else()
			set(DOWNLOAD_PATH "src/plugins/sqldrivers/sqlite")
		endif()

		set(URL_HEADER "https://raw.githubusercontent.com/qt/qtbase/v${Qt5Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite_p.h")
		set(URL_CPP "https://raw.githubusercontent.com/qt/qtbase/v${Qt5Sql_VERSION}/${DOWNLOAD_PATH}/qsql_sqlite.cpp")
		message(STATUS "URL = ${URL_HEADER}")

		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}")
		set(_PRIVATE_SOURCES_DIRECTORY "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}")
		include_directories("${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}")

		file(DOWNLOAD "${URL_HEADER}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qsql_sqlite_p.h" STATUS STATUS_HEADER_LIST TLS_VERIFY on LOG LOG_HEADER)
		file(DOWNLOAD "${URL_CPP}" "${CMAKE_BINARY_DIR}/qt-private-from-git-version-${Qt5Sql_VERSION}/qsql_sqlite.cpp" STATUS STATUS_CPP_LIST TLS_VERIFY on LOG LOG_CPP)

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
endif()

add_library(qsqlcipher ${LIBTYPE}
    smain.cpp
    "${_PRIVATE_SOURCES_DIRECTORY}/qsql_sqlite.cpp"
)

if (WIN32)
	# Change this by hand if needed
	set(SQLCIPHER_INCLUDE_DIRS "D:/CppProjects/sqlcipher-original")
endif()

target_include_directories(qsqlcipher PRIVATE
    ${Qt5Sql_PRIVATE_INCLUDE_DIRS}
    ${SQLCIPHER_INCLUDE_DIRS} ${Libsqlcipher_INCLUDE_DIRS}
)

if (WIN32)
	# Change this by hand if needed
	set(SQLCIPHER_LIBRARIES "optimized;D:/CppProjects/sqlcipher-original-build/Retail/x64/sqlite3.lib;debug;D:/CppProjects/sqlcipher-original-build/Debug/x64/sqlite3.lib")
endif()

target_link_libraries(qsqlcipher
    Qt5::Sql
    ${SQLCIPHER_LIBRARIES} ${Libsqlcipher_LIBRARIES}
)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(${TEST_DIR})
endif()
