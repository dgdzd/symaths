# lib/mathgen

add_library(mathgen_lib STATIC)

set(MATHGEN_LIB_SOURCES
        src/fitness.cpp
        src/model_manager.cpp
        src/tree_utils.cpp
        src/island_manager.cpp
        src/convergence.cpp
)

target_include_directories(mathgen_lib PUBLIC include)
target_sources(mathgen_lib PUBLIC ${MATHGEN_LIB_SOURCES})