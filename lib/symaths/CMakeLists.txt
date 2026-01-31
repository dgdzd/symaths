# lib/symaths

add_library(symaths_lib STATIC
        src/expressions_manip.cpp)

set(SYMATHS_LIB_SOURCES
        src/base_operations.cpp
)

target_include_directories(symaths_lib PUBLIC include)
target_sources(symaths_lib PUBLIC ${SYMATHS_LIB_SOURCES})