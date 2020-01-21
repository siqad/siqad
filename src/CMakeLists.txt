cmake_minimum_required(VERSION 3.10)

# for release builds, add flag "-DCMAKE_BUILD_TYPE=Release" (without quotes) when running cmake.

project(siqad VERSION 0.2.0)
# TODO add way to push version number into source code

if (WIN32)
    add_custom_target(
        siqad ALL
        COMMAND CMPL_QMAKE_COMMAND=${WIN32_QMAKE_BIN} CMPL_MAKE_COMMAND=make SIQAD_Q_PRO_FILE=${CMAKE_CURRENT_SOURCE_DIR}/siqad.pro ${CMAKE_CURRENT_SOURCE_DIR}/qmake_compile.sh 
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/siqad.exe DESTINATION ${SIQAD_INSTALL_ROOT})
else()
    find_package(Qt5Core ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Gui ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Widgets ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Svg ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5PrintSupport ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5UiTools ${QT_VERSION_REQ} REQUIRED)
    find_package(Qt5Charts ${QT_VERSION_REQ} REQUIRED)

    add_definitions( -DAPP_VERSION=\"0.1.0\" -DAPPLICATION_NAME=\"SiQAD\" -DORGANIZATION_NAME=\"WalusLab\" )

    set(CMAKE_AUTOMOC ON)
    #set(CMAKE_AUTORCC ON)

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

    if (WIN32)
        set(ENV{SIQAD_INSTALL_ROOT} ${SIQAD_INSTALL_ROOT})
        add_executable(siqad WIN32 ${SOURCES} ${HEADERS} ${CUSTOM_RSC})
    else()
        add_executable(siqad ${SOURCES} ${HEADERS} ${CUSTOM_RSC})
    endif()

    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        Qt5::PrintSupport
        Qt5::UiTools
        Qt5::Charts
    )

    install(TARGETS siqad RUNTIME DESTINATION ${SIQAD_INSTALL_ROOT})
    install(FILES helpers/is_python3.py DESTINATION ${SIQAD_INSTALL_ROOT}/helpers)
endif()