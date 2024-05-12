cmake_minimum_required(VERSION 3.10)

# Only set the CMAKE_CXX_STANDARD if it is not set by someone else
if(NOT DEFINED CMAKE_CXX_STANDARD)
    # Set C++ standard; at least C++17 is required
    set(CMAKE_CXX_STANDARD 17)
endif()

# It is highly recommended not to run cmake in this directory. Create a `build`
# directory first and run `cmake [OPTIONS] ..`.
# Add flag `-DCMAKE_BUILD_TYPE=Release` (without back-ticks) for release builds.
# Add flag "-DCMAKE_INSTALL_PREFIX=/path/to/target" to define installation location.

# Sets cached variables:
# ${SIQAD_INSTALL_ROOT}     : SiQAD install prefix root
# ${SIQAD_PLUGINS_ROOT}     : Default plugin installation root, ${SIQAD_INSTALL_ROOT}/plugins

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

# Set default install prefix path to /opt/siqad if CMAKE_INSTALL_PREFIX is not set.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/siqad" CACHE PATH "Default SiQAD installation path if none specified" FORCE)
endif()

# Set default CMAKE_BUILD_TYPE to Release if unspecified.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type.")
endif()

# Set the root subdirectory at which SiQAD will be installed to.
set(SIQAD_INSTALL_ROOT "${CMAKE_INSTALL_PREFIX}" CACHE STRING "SiQAD install root directory name")
message(STATUS "SIQAD_INSTALL_ROOT is ${SIQAD_INSTALL_ROOT}")

# Directories for plugins and libs
set(SIQAD_PLUGINS_ROOT "${SIQAD_INSTALL_ROOT}/sqplugins" CACHE STRING "SiQAD plugins root directory name")
set(SIQAD_LIBS_ROOT "${SIQAD_INSTALL_ROOT}/libs" CACHE STRING "SiQAD libraries root directory name")
set(SIQADCONN_ROOT "${SIQAD_LIBS_ROOT}/siqadtools" CACHE STRING "SiQADConnector root directory name")
set(SQCONN_REL_SQBIN "libs/siqadtools" CACHE STRING "SiQADConnector root directory relative to SiQAD binary directory")



#######################
#  Compile SiQAD GUI  #
#######################

add_subdirectory(src)
#add_subdirectory(tests)



###################################################
#  Build SiQADConnector for Python-based Plugins  #
###################################################

#if(NOT WIN32)
#    add_subdirectory(libs/siqadconn)
#endif()
#
## macro for symlinking to siqadtools if not compiling for Windows
#macro(symlink_siqadtools SQBIN_REL_PLUG LINK_BASE)
#    if(DEFINED SIQADCONN_ROOT AND NOT WIN32)
#        install(CODE "execute_process( \
#            COMMAND ${CMAKE_COMMAND} -E create_symlink \
#            ${SQBIN_REL_PLUG}/${SQCONN_REL_SQBIN} \
#            ${LINK_BASE}/siqadtools \
#            )"
#        )
#    endif()
#endmacro()


#################################
#  Officially included plugins  #
#################################

add_subdirectory(plugins)


##############
#  Licenses  #
##############

add_subdirectory(licenses)


# CPack

#set(CPACK_GENERATOR "DEB")
#set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Samuel S. H. Ng")
#include(CPack)
