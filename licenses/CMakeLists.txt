cmake_minimum_required(VERSION 3.10)

if(DEFINED SIQAD_INSTALL_ROOT)
    set(LICENSE_INSTALL_DIR "${SIQAD_INSTALL_ROOT}/licenses")
else()
    set(LICENSE_INSTALL_DIR "licenses")
endif()

set(LICENSE_FILES
    license_boost.txt
    license_qt5.txt
)

install(FILES ${LICENSE_FILES} DESTINATION ${LICENSE_INSTALL_DIR})
