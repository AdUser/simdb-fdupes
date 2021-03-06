set(CNAME "simdb-fdupes")
set(VERSION 0.4)

project($CNAME C)
cmake_minimum_required(VERSION 2.6)

include(CTest)
include(GNUInstallDirs)

option(WITH_HARDENING "Enable hardening options" ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c99")
add_definitions("-D_XOPEN_SOURCE=500")

if (WITH_HARDENING)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat -Wformat-security -Werror=format-security" )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector --param ssp-buffer-size=4" )
  add_definitions("-D_FORTIFY_SOURCE=2")
endif ()

find_library(MAGIC_FOUND "magic")
if (NOT MAGIC_FOUND)
  message(FATAL_ERROR "libmagic not found")
endif ()

find_library(SIMDB_FOUND "simdb")
if (NOT SIMDB_FOUND)
  message(FATAL_ERROR "libsimdb not found")
endif ()

message(STATUS "Project    : ${CNAME} v${VERSION}")
message(STATUS "Compiler   : ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "- CFLAGS   : ${CMAKE_C_FLAGS}")
message(STATUS "Paths:")
message(STATUS "- prefix   : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "- binary   : ${CMAKE_INSTALL_FULL_BINDIR}")
message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Options:")
message(STATUS "- WITH_HARDENING : ${WITH_HARDENING}")

add_subdirectory("src")
add_subdirectory("tests")
