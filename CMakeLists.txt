cmake_minimum_required(VERSION 3.1)

project(unblending CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

if(APPLE AND EXISTS /usr/local/opt/qt)
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt")
endif()
if(APPLE AND EXISTS /usr/local/opt/qt5)
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt5")
endif()

######################################################################
# Options
######################################################################

option(UNBLENDING_BUILD_CLI_APP "Build CLI app" ON )
option(UNBLENDING_BUILD_GUI_APP "Build GUI app" OFF)

######################################################################
# Add sub-directories for external libraries
######################################################################

set(TINYCOLORMAP_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/tinycolormap)

set(TIMER_BUILD_TEST         OFF CACHE INTERNAL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/timer)

set(PARALLELUTIL_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/parallel-util)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nlopt-util)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(NLOPT_GUILE       OFF CACHE INTERNAL "" FORCE)
set(NLOPT_LINK_PYTHON OFF CACHE INTERNAL "" FORCE)
set(NLOPT_MATLAB      OFF CACHE INTERNAL "" FORCE)
set(NLOPT_OCTAVE      OFF CACHE INTERNAL "" FORCE)
set(NLOPT_PYTHON      OFF CACHE INTERNAL "" FORCE)
set(NLOPT_SWIG        OFF CACHE INTERNAL "" FORCE)
set(NLOPT_TESTS       OFF CACHE INTERNAL "" FORCE)
set(NLOPT_CXX         ON  CACHE INTERNAL "" FORCE)
set(NLOPT_FORTRAN     OFF CACHE INTERNAL "" FORCE)
set(WITH_THREADLOCAL  OFF CACHE INTERNAL "" FORCE)
set(INSTALL_BIN_DIR     "bin"             CACHE INTERNAL "" FORCE)
set(INSTALL_CMAKE_DIR   "lib/cmake/nlopt" CACHE INTERNAL "" FORCE)
set(INSTALL_DATA_DIR    "share/nlopt"     CACHE INTERNAL "" FORCE)
set(INSTALL_INCLUDE_DIR "include"         CACHE INTERNAL "" FORCE)
set(INSTALL_LIB_DIR     "lib"             CACHE INTERNAL "" FORCE)
set(INSTALL_MAN_DIR     "share/man"       CACHE INTERNAL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nlopt)

set(JSON11_BUILD_TESTS          OFF CACHE INTERNAL "" FORCE)
set(JSON11_ENABLE_DR1467_CANARY ON  CACHE INTERNAL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/json11)

set(CXXOPTS_BUILD_EXAMPLES   OFF                 CACHE INTERNAL "" FORCE)
set(CXXOPTS_BUILD_TESTS      OFF                 CACHE INTERNAL "" FORCE)
set(CXXOPTS_CMAKE_DIR        "lib/cmake/cxxopts" CACHE INTERNAL "" FORCE)
set(CXXOPTS_USE_UNICODE_HELP OFF                 CACHE INTERNAL "" FORCE)
add_subdirectory(external/cxxopts)

######################################################################
# Add sub-directories
######################################################################

add_subdirectory(unblending)
if(UNBLENDING_BUILD_CLI_APP)
	add_subdirectory(unblending-cli)
endif()
if(UNBLENDING_BUILD_GUI_APP)
	add_subdirectory(unblending-gui)
endif()
