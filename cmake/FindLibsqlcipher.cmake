#
# FindLibsqlcipher
#
# This module finds the libsqlcipher header and library files. It
# sets the following variables.
#
# Libsqlcipher_INCLUDE_DIRS -  Include directories for Libsqlcipher header files.
# Libsqlcipher_LIBRARIES -	The name of the library to link against.
#
# Libsqlcipher_FOUND - Indicates whether Libsqlcipher has been found
#

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(Libsqlcipher QUIET SQLCipher)
endif()

find_path(Libsqlcipher_INCLUDE_DIRS sqlcipher/sqlite3.h
	HINTS
	$ENV{LIBSQLCIPHER_DIR}
	PATH_SUFFIXES include/sqlcipher include
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/opt/local
	/opt
	${Libsqlcipher_INCLUDE_DIRS}
)

if(NOT EXISTS "${Libsqlcipher_INCLUDE_DIRS}/sqlcipher/sqlite3.h")
	message(SEND_ERROR "Could not find LibSqlCipher Include directory for Libsqlcipher_INCLUDE_DIRS, it should contain sqlcipher/sqlite3.h! Tried: \"${Libsqlcipher_INCLUDE_DIRS}\"")
	set(Libsqlcipher_INCLUDE_DIRS "")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
	set(_lib_suffix_win "x64")
else()
	set(_lib_suffix 32)
	set(_lib_suffix_win "x86")
endif()

IF (WIN32)
	IF (MINGW)
		SET (LIB_PREFIX "lib")
		SET (LIB_POSTFIX "so")
	ELSEIF (MSVC)
		SET (LIB_PREFIX "lib")
		SET (LIB_POSTFIX "lib")
	ENDIF(MINGW)   
ELSE (UNIX)
	SET (LIB_PREFIX "lib")
	SET (LIB_POSTFIX "so")
ENDIF (WIN32)

FIND_LIBRARY(Libsqlcipher_LIBRARIES 
  NAMES ${LIB_PREFIX}sqlcipher.${LIB_POSTFIX}
  HINTS
  $ENV{LIBSQLCIPHER_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  /sw
  /opt/local
  /opt
  /usr
)

find_package_handle_standard_args(Libsqlcipher FOUND_VAR Libsqlcipher_FOUND REQUIRED_VARS Libsqlcipher_LIBRARIES Libsqlcipher_INCLUDE_DIRS)
mark_as_advanced(Libsqlcipher_INCLUDE_DIRS)
mark_as_advanced(Libsqlcipher_LIBRARIES)
