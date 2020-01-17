cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
    cmake_policy(SET CMP0078 OLD)
endif()

#set_property(TARGET ${SWIG_MODULE_siqadconn_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED 11)
add_definitions(-Wall -g -std=c++11)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_PATH})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set(CMAKE_SWIG_FLAGS "")

set_property(SOURCE src/siqadconn.i PROPERTY CPLUSPLUS ON)
#set_property(TARGET ${SWIG_MODULE_siqadconn_REAL_NAME} PROPERTY SWIG_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
#set_property(SOURCE src/siqadconn.cc PROPERTY SWIG_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
#set_source_files_properties(src/siqadconn.i PROPERTIES SWIG_FLAGS "-includeall")


# build the SWIG src and C++ code
# TODO figure out why TYPE STATIC compiles but doesn't work in practice (can't import _siqadconn)
#swig_add_library(siqadconn TYPE STATIC LANGUAGE python SOURCES src/siqadconn.i src/siqadconn.cc)
swig_add_library(siqadconn LANGUAGE python SOURCES src/siqadconn.i src/siqadconn.cc)

swig_link_libraries(siqadconn ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS ${SWIG_MODULE_siqadconn_REAL_NAME} 
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../cmake-install/siqadtools)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/siqadconn.py 
              ${CMAKE_CURRENT_SOURCE_DIR}/src/__init__.py
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../cmake-install/siqadtools)
