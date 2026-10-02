set(test_results "${CMAKE_BINARY_DIR}/test-results")
file(MAKE_DIRECTORY "${test_results}")

function(siqad_test target labels)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_compile_definitions(${target} PRIVATE
        SIQAD_TEST_ARTIFACT_DIR="${test_results}"
        SIQAD_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
    add_test(NAME ${target} COMMAND ${target}
        -o "${test_results}/${target}.xml,junitxml" -o "-,txt")
    set_tests_properties(${target} PROPERTIES TIMEOUT 90 LABELS "${labels}")
endfunction()

add_executable(siqad_tests siqad_tests.cpp)
target_link_libraries(siqad_tests PRIVATE Qt6::Test ${BIN_LINKS})
siqad_test(siqad_tests "gui")
set_tests_properties(siqad_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(gui_workflow_tests gui_workflow_tests.cpp)
target_link_libraries(gui_workflow_tests PRIVATE Qt6::Test ${BIN_LINKS})
siqad_test(gui_workflow_tests "gui;integration")
set_tests_properties(gui_workflow_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# A compiled QtCore helper avoids shell/interpreter differences across platforms.
add_executable(fake_simulator fake_simulator.cpp)
target_link_libraries(fake_simulator PRIVATE Qt6::Core)
add_executable(sim_job_workflow_tests sim_job_workflow_tests.cpp)
target_link_libraries(sim_job_workflow_tests PRIVATE Qt6::Test ${BIN_LINKS})
target_compile_definitions(sim_job_workflow_tests PRIVATE
    SIQAD_FAKE_SIMULATOR="$<TARGET_FILE:fake_simulator>")
add_dependencies(sim_job_workflow_tests fake_simulator)
siqad_test(sim_job_workflow_tests "gui;integration")
set_tests_properties(sim_job_workflow_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(document_export_tests document_export_tests.cpp)
target_link_libraries(document_export_tests PRIVATE Qt6::Test ${BIN_LINKS})
siqad_test(document_export_tests "gui;integration")
set_tests_properties(document_export_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

if(SIQAD_NATIVE_TESTS)
    add_executable(native_clipboard_tests native_clipboard_tests.cpp)
    target_link_libraries(native_clipboard_tests PRIVATE Qt6::Test ${BIN_LINKS})
    siqad_test(native_clipboard_tests "native;integration")
    set_tests_properties(native_clipboard_tests PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)
endif()

add_executable(clipboard_codec_tests clipboard_codec_tests.cpp ../gui/clipboard_codec.cc)
target_link_libraries(clipboard_codec_tests PRIVATE Qt6::Test Qt6::Core)
siqad_test(clipboard_codec_tests "unit")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME developer_script_tests COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/developer_script_tests.py --junit "${test_results}/developer_script_tests.xml")
set_tests_properties(developer_script_tests PROPERTIES TIMEOUT 60 LABELS "unit;build-script")
add_test(NAME report_tests_tests COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/report_tests_tests.py --junit "${test_results}/report_tests_tests.xml")
set_tests_properties(report_tests_tests PROPERTIES TIMEOUT 10 LABELS "unit")
