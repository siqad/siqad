cmake_minimum_required(VERSION 3.10)

# It is highly recommended not to run cmake in this directory. Create a `build`
# directory first and run `cmake [OPTIONS] ..`.
# Add flag `-DCMAKE_BUILD_TYPE=Release` (without back-ticks) for release builds.
# Add flag "-DCMAKE_INSTALL_PREFIX=/path/to/target" to define installation location.

project(siqad_suite)



####################
#  Configurations  #
####################

# Set FORCE_CMAKE_SIQAD_GUI to TRUE if you want to force compile the GUI 
# through CMAKE on Windows, which I have tested to work through Visual Studio 
# 2017 + Qt 5.14.0. You'll need to add a few environmental variables to make 
# it work (CMAKE_PREFIX_PATH). Qt's documentation also seems to indicate the
# the possibility of using CMake with Qt Creator, read their documentation if
# you want to find out more about that. Feel free to contribute code that makes
# the build process easier on Windows.
set(WIN32_FORCE_CMAKE_SIQAD_GUI FALSE)

# Set default CMAKE_BUILD_TYPE to Release if unspecified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type.")
endif()

# Set the root subdirectory at which SiQAD will be installed to. Add a `debug`
# level by default if making a debug build.
# Relative paths are parsed to be relative to CMAKE_INSTALL_PREFIX.
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SIQAD_INSTALL_ROOT "./debug" CACHE STRING "SiQAD install root directory name")
else()
    set(SIQAD_INSTALL_ROOT "./" CACHE STRING "SiQAD install root directory name")
endif()



#######################
#  Compile SiQAD GUI  #
#######################

if(UNIX AND NOT APPLE)
    set(LINUX TRUE CACHE STRING "Use the LINUX Boolean to check for LINUX system.")
endif()

add_subdirectory(src)



###################################################
#  Build SiQADConnector for Python-based Plugins  #
###################################################

# NOTE for now, the compilation and installation of SiQADConnector will not 
# be included in the main CMakeLists.



#################################
#  Officially included plugins  #
#################################

# Exhaustive GS (implements ground state model of SimAnneal and searches 
# for the ground state charge configuration exhaustively)
add_subdirectory(src/phys/exhaustive_gs)

# HoppingDynamics
add_subdirectory(src/phys/afmmarcus)

# PoisSolver
add_subdirectory(src/phys/poissolver)

# SimAnneal
add_subdirectory(src/phys/simanneal)
