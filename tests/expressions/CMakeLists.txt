# tests/expressions

set(TESTS_SOURCES
        basic_exprs_computing.cpp
)


##### Compile-time evaluations test


add_executable(basic_exprs_computing_test ${TESTS_SOURCES})

set_target_properties(basic_exprs_computing_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/bin
)

target_link_libraries(basic_exprs_computing_test PRIVATE
        gtest_main
        symaths_lib
)

include(GoogleTest)
gtest_discover_tests(basic_exprs_computing_test)


##### Runtime evaluations test


add_executable(expressions_value_test ${TESTS_SOURCES})

set_target_properties(expressions_value_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/bin
)

target_link_libraries(expressions_value_test PRIVATE
        gtest_main
        symaths_lib
)

include(GoogleTest)
gtest_discover_tests(expressions_value_test)