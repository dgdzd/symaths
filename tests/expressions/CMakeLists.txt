# tests/expressions

# API tests

add_executable(basic_exprs_computing_test
        basic_exprs_computing.cpp
)

set_target_properties(basic_exprs_computing_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/bin
)

target_link_libraries(basic_exprs_computing_test PRIVATE
        gtest_main
        symaths_lib
        mathgen_lib
)

include(GoogleTest)
gtest_discover_tests(basic_exprs_computing_test)


##### Internal functions tests


add_executable(internal_funcs_test internal_funcs.cpp)

set_target_properties(internal_funcs_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/bin
)

target_link_libraries(internal_funcs_test PRIVATE
        gtest_main
        symaths_lib
        mathgen_lib
)

include(GoogleTest)
gtest_discover_tests(internal_funcs_test)