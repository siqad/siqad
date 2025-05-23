cmake_minimum_required(VERSION 3.10)

# for release builds, add flag "-DCMAKE_BUILD_TYPE=Release" (without quotes) when running cmake.

project(siqad VERSION 0.3.3)
# TODO add way to push version number into source code

# Zipper library
if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()
add_subdirectory(libs/zipper EXCLUDE_FROM_ALL)  # EXCLUDE_FROM_ALL should impede zipper's install calls from working
include_directories(libs/zipper)
set(ZIPPER_LIB_PATH ${CMAKE_CURRENT_BINARY_DIR}/libs/zipper/Release)
message(STATUS "Zipper Lib path set to: ${ZIPPER_LIB_PATH}")

set(USE_QMAKE_ON_LINUX FALSE)

if (WIN32)
    # these paths are chosen for Visual Studio compilation on AppVeyor
    # local compilation or cross-compilation may require changes, especially to paths within qmake_compile.sh and SIQAD_EXE_TARGET
    set(CMPL_QMAKE_COMMAND ${WIN32_QMAKE_BIN})
    set(CMPL_MAKE_COMMAND ${WIN32_MAKE_BIN})
    set(SIQAD_Q_PRO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/siqad.pro)
    set(SIQAD_EXE_TARGET ${CMAKE_CURRENT_BINARY_DIR}/siqad.exe)
    add_custom_target(
            siqad ALL
            COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qmake_compile.sh "${CMPL_QMAKE_COMMAND}" "${CMPL_MAKE_COMMAND}" "${SIQAD_Q_PRO_FILE}" "${SIQAD_EXE_TARGET}" "${ZIPPER_LIB_PATH}"
    )
    add_dependencies(siqad staticZipper)
    install(FILES ${SIQAD_EXE_TARGET} DESTINATION ${SIQAD_INSTALL_ROOT})
    install(FILES $ENV{ZLIB_DLL} DESTINATION ${SIQAD_INSTALL_ROOT})
elseif (USE_QMAKE_ON_LINUX)
    set(CMPL_QMAKE_COMMAND qmake)
    set(CMPL_MAKE_COMMAND make)
    set(SIQAD_Q_PRO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/siqad.pro)
    set(SIQAD_EXE_TARGET ${CMAKE_CURRENT_BINARY_DIR}/siqad)
    add_custom_target(
            siqad ALL
            COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qmake_compile.sh "${CMPL_QMAKE_COMMAND}" "${CMPL_MAKE_COMMAND}" "${SIQAD_Q_PRO_FILE}" "${SIQAD_EXE_TARGET}" "${ZIPPER_LIB_PATH}"
    )
    add_dependencies(siqad Zipper)
    install(FILES ${SIQAD_EXE_TARGET} DESTINATION ${SIQAD_INSTALL_ROOT})
else()
    # Set hints for common Qt5 installation paths
    set(QT5_HINTS
            "/opt/homebrew/Cellar/qt@5"        # Apple Silicon Homebrew with qt@5
            "/opt/homebrew/Cellar/qt"          # Apple Silicon Homebrew standard
            "/usr/local/Cellar/qt@5"           # Intel macOS Homebrew with qt@5
            "/usr/local/Cellar/qt"             # Intel macOS Homebrew standard
            "/usr/local/opt/qt"                # Homebrew symlink (if applicable)
            "/usr/lib/x86_64-linux-gnu"        # Ubuntu x86_64 default
            "/usr/lib/aarch64-linux-gnu"       # Ubuntu ARM64 default
            "/usr/lib64"                       # Other Linux distros (e.g., Fedora)
            "/usr/lib"                         # Generic Linux fallback
            "C:/Qt"                            # Windows default
            "$ENV{HOME}/Qt"                    # User-local Qt installation
    )

    # If Qt5_DIR is not set, try to find Qt5 automatically
    if(NOT Qt5_DIR)
        foreach(hint ${QT5_HINTS})
            file(GLOB QT_VERSIONS "${hint}/qt5/*" "${hint}/5.*")  # Look for Qt5 versions or qt5 directory
            if(QT_VERSIONS)
                list(GET QT_VERSIONS 0 QT5_PATH)  # Pick the first found version
                if(EXISTS "${QT5_PATH}/lib/cmake/Qt5")
                    set(Qt5_DIR "${QT5_PATH}/lib/cmake/Qt5" CACHE PATH "Path to Qt5 config directory")
                elseif(EXISTS "${hint}/cmake/Qt5")
                    set(Qt5_DIR "${hint}/cmake/Qt5" CACHE PATH "Path to Qt5 config directory")
                elseif(EXISTS "${hint}/Qt5Config.cmake")
                    get_filename_component(Qt5_DIR "${hint}" ABSOLUTE CACHE PATH "Path to Qt5 config directory")
                endif()
                if(Qt5_DIR)
                    break()
                endif()
            endif()
        endforeach()

        # If still not found, check the system PATH or qmake location
        if(NOT Qt5_DIR)
            find_program(QMAKE_EXECUTABLE qmake)
            if(QMAKE_EXECUTABLE)
                get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
                get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}/.." ABSOLUTE)
                if(EXISTS "${QT_ROOT_DIR}/lib/cmake/Qt5")
                    set(Qt5_DIR "${QT_ROOT_DIR}/lib/cmake/Qt5" CACHE PATH "Path to Qt5 config directory")
                endif()
            endif()
        endif()

        if(NOT Qt5_DIR)
            message(FATAL_ERROR "Could not find Qt5. Please install Qt5 (e.g., 'brew install qt@5' on macOS, 'apt-get install qtbase5-dev qttools5-dev' on Ubuntu) or specify -DQt5_DIR=<path> manually.")
        else()
            message(STATUS "Found Qt5_DIR: ${Qt5_DIR}")
        endif()
    endif()

    # Find Qt5 components using the detected or specified Qt5_DIR
    find_package(Qt5 ${QT_VERSION_REQ} REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport UiTools Charts PATHS ${Qt5_DIR})
    if(NOT SKIP_SIQAD_TESTS)
        find_package(Qt5 ${QT_VERSION_REQ} REQUIRED COMPONENTS Test PATHS ${Qt5_DIR})
    endif()

    # Find additional system libraries
    find_package(OpenGL REQUIRED)
    find_package(ZLIB REQUIRED)

    # Define library links in a more explicit order
    set(LIB_LINKS
            Qt5::Charts
            Qt5::UiTools
            Qt5::PrintSupport
            Qt5::Svg
            Qt5::Widgets
            Qt5::Gui
            Qt5::Core
            staticZipper
            ${OPENGL_LIBRARIES}
            ${ZLIB_LIBRARIES}
    )

    # QtTest related: Add option to disable testing
    option(SKIP_SIQAD_TESTS "Skip building and running SiQAD tests" OFF)
    if(NOT SKIP_SIQAD_TESTS)
        enable_testing()
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()

    add_definitions(-DAPP_VERSION=\"0.3.3\" -DAPPLICATION_NAME=\"SiQAD\" -DORGANIZATION_NAME=\"WalusLab\")

    set(CMAKE_AUTOMOC ON)
    qt5_add_resources(CUSTOM_RSC resources/application.qrc)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(QT_VERSION_REQ "5.2")

    # Explicit loading of source and header files for qmake compatibility
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

    # SiQAD binary
    if (WIN32)
        set(ENV{SIQAD_INSTALL_ROOT} ${SIQAD_INSTALL_ROOT})
        add_executable(siqad WIN32 main.cc ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
    else()
        add_executable(siqad main.cc ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC ${BIN_LINKS})

    set(ZIPPER_LIBS
        staticZipper
        Zipper-static
    )
    if (BUILD_TEST)
        list(APPEND ZIPPER_LIBS Zipper-test)
    endif()
    add_dependencies(siqad ${ZIPPER_LIBS})
    if (USE_SIQAD_LIB)
        add_dependencies(siqad_lib ${ZIPPER_LIBS})
    endif()

    # SiQAD unit tests:
    option(BUILD_TEST "Build the test program." ON)
    if(BUILD_TEST)
        add_executable(siqad_tests tests/siqad_tests.cpp ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
        target_link_libraries(siqad_tests PUBLIC Qt5::Test ${BIN_LINKS})
        add_test(NAME siqad_tests COMMAND siqad_tests)
        set_tests_properties(siqad_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        add_custom_command(TARGET siqad_tests
            POST_BUILD
            COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
    endif()

    # Installation
    install(TARGETS siqad RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    if (USE_SIQAD_LIB)
        install(TARGETS siqad_lib RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    endif()
endif()

install(FILES helpers/is_python3.py DESTINATION ${SIQAD_INSTALL_ROOT}/helpers)
