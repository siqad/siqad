cmake_minimum_required(VERSION 3.10)

# for release builds, add flag "-DCMAKE_BUILD_TYPE=Release" (without quotes) when running cmake.

project(siqad VERSION 0.2.2)
# TODO add way to push version number into source code

if (WIN32)
    # these paths are chosen for Visual Studio compilation on AppVeyor
    # local compilation or cross-compilation may require changes, especially to paths within qmake_compile.sh and SIQAD_EXE_TARGET
    set(CMPL_QMAKE_COMMAND ${WIN32_QMAKE_BIN})
    set(CMPL_MAKE_COMMAND ${WIN32_MAKE_BIN})
    set(SIQAD_Q_PRO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/siqad.pro)
    set(SIQAD_EXE_TARGET ${CMAKE_CURRENT_BINARY_DIR}/siqad.exe)
    add_custom_target(
        siqad ALL
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/qmake_compile.sh "${CMPL_QMAKE_COMMAND}" "${CMPL_MAKE_COMMAND}" "${SIQAD_Q_PRO_FILE}" "${SIQAD_EXE_TARGET}"
    )
    install(FILES ${SIQAD_EXE_TARGET} DESTINATION ${SIQAD_INSTALL_ROOT})
else()
    find_package(Qt5Core ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Gui ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Widgets ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Svg ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5PrintSupport ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5UiTools ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Charts ${QT_VERSION_REQ} REQUIRED)

    set(LINK_LIBS
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        Qt5::PrintSupport
        Qt5::UiTools
        Qt5::Charts
    )

    # QtTest related: (should probably add a flag to disable testing)
    enable_testing()
    find_package(Qt5Test ${QT_VERSION_REQ} REQUIRED)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    add_definitions( -DAPP_VERSION=\"0.2.2\" -DAPPLICATION_NAME=\"SiQAD\" -DORGANIZATION_NAME=\"WalusLab\" )

    set(CMAKE_AUTOMOC ON)


    qt5_add_resources(CUSTOM_RSC resources/application.qrc)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    #set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    #set(CMAKE_CXX_FLAGS_DEBUG "-g")
    #set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(QT_VERSION_REQ "5.2")

    # This explicit loading of header files, rather than include_directories, is to
    # apeace qmake which also reads the following file lists.
    file(STRINGS source_files SOURCES)
    file(STRINGS header_files HEADERS)
    include_directories(.)

    # SiQAD binary:
    if (WIN32)
        set(ENV{SIQAD_INSTALL_ROOT} ${SIQAD_INSTALL_ROOT})
        add_executable(siqad WIN32 main.cc ${SOURCES} ${HEADERS} ${CUSTOM_RSC})
    else()
        #add_executable(siqad main.cc ${SOURCES} ${HEADERS} ${CUSTOM_RSC})
        add_library(siqad_lib OBJECT ${SOURCES} ${HEADERS} ${CUSTOM_RSC})
        add_executable(siqad main.cc)
    endif()

    target_link_libraries(siqad_lib ${LINK_LIBS})
    target_link_libraries(${PROJECT_NAME} siqad_lib ${LINK_LIBS})

    # SiQAD unit tests:
    add_executable(siqad_tests tests/siqad_tests.cpp)
    target_link_libraries(siqad_tests siqad_lib Qt5::Test ${LINK_LIBS})
    add_test(siqad_tests siqad_tests)
    add_custom_command(TARGET siqad_tests
        POST_BUILD
        COMMAND ctest -C $<CONFIGURATION> --output-on-failure)

    install(TARGETS siqad RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    install(TARGETS siqad_lib RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
endif()

install(FILES helpers/is_python3.py DESTINATION ${SIQAD_INSTALL_ROOT}/helpers)
