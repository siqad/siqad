include_guard(GLOBAL)

# Accept old command lines on their first configure; BUILD_TESTING is authoritative
# when supplied. Do not create contradictory cached aliases.
if(NOT DEFINED BUILD_TESTING)
    if(DEFINED SKIP_SIQAD_TESTS AND SKIP_SIQAD_TESTS)
        set(BUILD_TESTING OFF CACHE BOOL "Build SiQAD tests")
    elseif(DEFINED BUILD_TEST)
        set(BUILD_TESTING "${BUILD_TEST}" CACHE BOOL "Build SiQAD tests")
    endif()
endif()
include(CTest)
option(SIQAD_NATIVE_TESTS "Run tests using the native system clipboard/display" OFF)
option(SIQAD_SANITIZERS "Instrument GUI and tests with address/undefined sanitizers" OFF)

if(SIQAD_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "SIQAD_SANITIZERS requires Clang or GCC")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
