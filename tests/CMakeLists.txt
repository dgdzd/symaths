# tests/

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# For Windows: avoid overriding CMake's own test main
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(SYMATHS_BUILD AND SYMATHS_BUILD_TESTS)
    add_subdirectory(expressions)
    add_subdirectory(parsing)
endif()

if(MATHGEN_BUILD AND MATHGEN_BUILD_TESTS)
    add_subdirectory(mathgen/test0)
endif()