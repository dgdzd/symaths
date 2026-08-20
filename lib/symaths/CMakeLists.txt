# lib/symaths/

add_library(symaths_lib STATIC)

set(SYMATHS_LIB_SOURCES
        src/symaths.cpp
        src/base_functions.cpp
        src/builtin_functions.cpp
        src/context_table.cpp
        src/differentiation.cpp
        src/expression.cpp
        src/expressions_manip.cpp
        src/numbers.cpp
        src/polynomial.cpp
        src/predicate.cpp
        src/program.cpp
        src/set.cpp
        src/detail/mathexpr_node.cpp
        src/detail/node_manager.cpp
        src/detail/predicate_node.cpp
        src/detail/set_node.cpp
        src/detail/statement_node.cpp
        src/parsing/lexer.cpp
        src/parsing/parser.cpp
        src/utils/exceptions.cpp
        src/utils/maths.cpp
)

target_include_directories(symaths_lib PUBLIC include)
target_sources(symaths_lib PUBLIC ${SYMATHS_LIB_SOURCES})

add_library(symaths::core ALIAS symaths_lib)