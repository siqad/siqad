cmake_minimum_required(VERSION 3.10)

# for release builds, add flag "-DCMAKE_BUILD_TYPE=Release" (without quotes) when running cmake.

project(siqad VERSION 0.2.2)
# TODO add way to push version number into source code

# Zipper library
if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()
add_subdirectory(libs/zipper)
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
    find_package(Qt5Core ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Gui ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Widgets ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Svg ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5PrintSupport ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5UiTools ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Charts ${QT_VERSION_REQ} REQUIRED)

    set(LIB_LINKS
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        Qt5::PrintSupport
        Qt5::UiTools
        Qt5::Charts
        staticZipper
    )

    # QtTest related: (should probably add a flag to disable testing)
    if(NOT SKIP_SIQAD_TESTS)
        enable_testing()
        find_package(Qt5Test ${QT_VERSION_REQ} REQUIRED)
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()

    add_definitions( -DAPP_VERSION=\"0.2.2\" -DAPPLICATION_NAME=\"SiQAD\" -DORGANIZATION_NAME=\"WalusLab\" )

    set(CMAKE_AUTOMOC ON)


    qt5_add_resources(CUSTOM_RSC resources/application.qrc)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(QT_VERSION_REQ "5.2")

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
    else()
        add_executable(siqad main.cc ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC ${BIN_LINKS})

    # SiQAD unit tests:
    if(NOT SKIP_SIQAD_TESTS)
        add_executable(siqad_tests tests/siqad_tests.cpp ${BIN_SOURCES} ${BIN_HEADERS} ${BIN_CUSTOM_RSC})
        target_link_libraries(siqad_tests Qt5::Test ${BIN_LINKS})
        add_test(siqad_tests siqad_tests)
	set_tests_properties(siqad_tests PROPERTIES
		ENVIRONMENT QT_QPA_PLATFORM=offscreen)
        add_custom_command(TARGET siqad_tests
            POST_BUILD
            COMMAND ctest -C $<CONFIGURATION> --output-on-failure)
    endif()

    install(TARGETS siqad RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    if (USE_SIQAD_LIB)
        install(TARGETS siqad_lib RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    endif()
endif()

install(FILES helpers/is_python3.py DESTINATION ${SIQAD_INSTALL_ROOT}/helpers)
