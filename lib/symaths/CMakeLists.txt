# lib/symaths

add_library(symaths_lib STATIC)

set(SYMATHS_LIB_SOURCES
        src/symaths.cpp
        src/base_functions.cpp
        src/differentiation.cpp
        src/expression.cpp
        src/expressions_manip.cpp
        src/polynomial.cpp
        src/detail/nodes.cpp
        src/parsing/lexer.cpp
        src/parsing/parser.cpp
        src/utils/maths.cpp
)

target_include_directories(symaths_lib PUBLIC include)
target_sources(symaths_lib PUBLIC ${SYMATHS_LIB_SOURCES})