cmake_minimum_required(VERSION 3.20)

# for release builds, add flag "-DCMAKE_BUILD_TYPE=Release" (without quotes) when running cmake.

project(siqad VERSION 0.3.3)
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Testing.cmake)
# TODO add way to push version number into source code

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(QT_VERSION_REQ "6.0")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport UiTools Charts Xml)
if(COMMAND qt_standard_project_setup)
  # Qt >= 6.3
  qt_standard_project_setup()
else()
  # Qt < 6.3 fallback
  set(CMAKE_AUTOMOC   ON)
  set(CMAKE_AUTOUIC   ON)
  set(CMAKE_AUTORCC   ON)
endif()

set(LIB_LINKS
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::PrintSupport
    Qt6::UiTools
    Qt6::Charts
    Qt6::Xml
)

# QtTest related:
if(BUILD_TESTING)
    find_package(Qt6 COMPONENTS Test REQUIRED)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_definitions( -DAPP_VERSION=\"0.3.3\" -DAPPLICATION_NAME=\"SiQAD\" -DORGANIZATION_NAME=\"WalusLab\" )

set(CMAKE_AUTOMOC ON)


qt6_add_resources(CUSTOM_RSC resources/application.qrc)

# This explicit loading of header files, rather than include_directories, is to
# apeace qmake (windows routine) which also reads the following file lists.
file(STRINGS source_files LIB_SOURCES)
file(STRINGS header_files LIB_HEADERS)
if(${CMAKE_VERSION} VERSION_LESS "3.12")
    set(USE_SIQAD_LIB FALSE)
    set(BIN_SOURCES ${LIB_SOURCES})
    set(BIN_HEADERS ${LIB_HEADERS})
    set(BIN_CUSTOM_RSC ${CUSTOM_RSC})
    set(BIN_LINKS ${LIB_LINKS})
else()
    set(USE_SIQAD_LIB TRUE)
    set(BIN_LINKS siqad_lib)
endif()
include_directories(.)

if (USE_SIQAD_LIB)
    add_library(siqad_lib OBJECT ${LIB_SOURCES} ${LIB_HEADERS} ${CUSTOM_RSC})
    target_link_libraries(siqad_lib PUBLIC ${LIB_LINKS})
endif()

# SiQAD binary:
if (WIN32)
    set(ENV{SIQAD_INSTALL_ROOT} ${SIQAD_INSTALL_ROOT})
    add_executable(siqad WIN32 main.cc ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})

    set(deploy_tool_options_args --no-system-d3d-compiler --libdir ${SIQAD_INSTALL_ROOT})

    # Generate windeployqt script and run it
    qt_generate_deploy_app_script(
        TARGET siqad
        OUTPUT_SCRIPT QT_DEPLOY_SCRIPT
        NO_TRANSLATIONS
        # NO_COMPILER_RUNTIME
        NO_UNSUPPORTED_PLATFORM_ERROR
        DEPLOY_TOOL_OPTIONS ${deploy_tool_options_args}
    )
    install(SCRIPT ${QT_DEPLOY_SCRIPT})
else()
    add_executable(siqad main.cc ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
endif()

set_target_properties(siqad PROPERTIES
    WIN32_EXECUTABLE ON
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${BIN_LINKS})

# Tests run explicitly through CTest, including on unchanged builds.
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS siqad RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
if (USE_SIQAD_LIB)
    install(TARGETS siqad_lib RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
endif()

install(FILES helpers/is_python3.py DESTINATION ${SIQAD_INSTALL_ROOT}/helpers)
