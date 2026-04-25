set(MATHGEN_TESTS_SOURCES
        test.cpp
)

add_executable(test0 ${MATHGEN_TESTS_SOURCES})

set_target_properties(test0
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/mathgen_tests/bin
)

target_link_libraries(test0 PRIVATE
        mathgen_lib
)