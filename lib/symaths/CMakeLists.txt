# lib/symaths

add_library(symaths_lib STATIC)

set(SYMATHS_LIB_SOURCES
        src/expressions_manip.cpp
        src/symaths.cpp
        src/detail/nodes.cpp
)

target_include_directories(symaths_lib PUBLIC include)
target_sources(symaths_lib PUBLIC ${SYMATHS_LIB_SOURCES})