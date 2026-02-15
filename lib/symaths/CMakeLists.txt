# lib/symaths

add_library(symaths_lib STATIC)

set(SYMATHS_LIB_SOURCES
        src/symaths.cpp
        src/expressions_manip.cpp
        src/differentiation.cpp
        src/detail/nodes.cpp
)

target_include_directories(symaths_lib PUBLIC include)
target_sources(symaths_lib PUBLIC ${SYMATHS_LIB_SOURCES})