# tests/expressions

# API tests

add_executable(parsing_test
        parsing.cpp
)

set_target_properties(parsing_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/bin
)

target_link_libraries(parsing_test PRIVATE
        gtest_main
        symaths_lib
)

include(GoogleTest)
gtest_discover_tests(parsing_test)