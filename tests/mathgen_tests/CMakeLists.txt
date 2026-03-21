cmake_minimum_required(VERSION 3.11)

set(MATHGEN_TESTS_SOURCES
        test.cpp
)

add_executable(test ${MATHGEN_TESTS_SOURCES})

set_target_properties(test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/mathgen_tests/bin
)

target_link_libraries(test PRIVATE
        mathgen_lib
)