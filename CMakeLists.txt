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

option(STATIC "Build plugin as a static library" OFF)
if(STATIC)
    set(LIBTYPE STATIC)
    add_definitions(-DQT_STATICPLUGIN)
    set(TEST_DIR test-static)
else()
    set(LIBTYPE MODULE)
	add_definitions(-DQT_PLUGIN)
    set(TEST_DIR test-shared)
endif()

add_library(qsqlcipher ${LIBTYPE}
    smain.cpp
    qt-private/qsql_sqlite.cpp
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
