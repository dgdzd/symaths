# symaths core library

The symaths core library is a symbolic mathematics engine for C++. It provides expression construction, manipulation, evaluation, symbolic differentiation, equation solving, polynomial decomposition, and an interactive mini programming language with parsing.

## Features

- **Symbolic expressions** -- build, simplify, expand, and sort mathematical expressions
- **Symbolic differentiation** -- compute derivatives with respect to any variable
- **Expression parsing** -- convert strings like `"x^2 + sin(x)"` into ASTs
- **Evaluation** -- evaluate expressions numerically with variable bindings
- **Equation solving** -- solve symbolic equations and retrieve solution sets
- **Polynomial decomposition** -- extract coefficients and degrees from polynomial expressions
- **Number tower** -- automatic type promotion through natural, integer, rational, real, complex, and NaN
- **Predicate logic** -- equalities, inequalities, set membership, logical connectives
- **Set theory** -- intervals, integer sets, conditional sets, intersections, unions
- **Mini programming language** -- assignments, conditionals, function definitions, sequential execution
- **Hash-consed AST** -- all nodes are interned for O(1) equality checks and memory efficiency
- **Built-in math functions** -- sin, cos, tan, exp, ln, sqrt, abs, and more, with evaluation, simplification, and derivative rules

## Using the library

### As a CMake subdirectory

```cmake
add_subdirectory(lib/symaths)
target_link_libraries(your_target PRIVATE symaths::core)
```

### After installing

```cmake
find_package(symaths REQUIRED)
target_link_libraries(your_target PRIVATE symaths::core)
```

## Quick example

```cpp
#include <symaths/symaths.hpp>

int main() {
    sym::library ctx;
    sym::make_context_current(ctx);

    // Create symbols and expressions
    sym::symbol x("x");
    sym::expression expr = x * x + sym::make_constant(3.0) * x + sym::make_constant(1.0);

    // Differentiate
    sym::expression derivative = sym::differentiate(expr, x); // 2*x + 3

    // Parse from string
    sym::expression parsed = sym::parse_expression("sin(x) * exp(x)");

    return 0;
}
```

## Public API overview

| Header | Purpose |
|--------|---------|
| `symaths.hpp` | Master header -- library context, node factories, built-in function wrappers |
| `expression.hpp` | `expression` class -- the primary symbolic expression type |
| `symbol.hpp` | `symbol` class -- named variables |
| `differentiation.hpp` | `differentiate(expr, symbol)` -- symbolic differentiation |
| `expressions_manip.hpp` | `reduce()`, `sort()`, `expand()` -- expression simplification and transformation |
| `polynomial.hpp` | `polynomial` class -- decomposition into coefficient form |
| `equation.hpp` | `equation` class -- equation representation and solving |
| `predicate.hpp` | `predicate` class -- logical and relational expressions |
| `set.hpp` | `set` class -- intervals, integer/conditional/intersection sets |
| `numbers.hpp` | `number` class -- multi-rank numeric tower with arithmetic |
| `program.hpp` | `program` class -- mini programming language with evaluation |
| `context_table.hpp` | `context_table_t` -- named variable binding storage |
| `parsing/lexer.hpp` | Tokenizer for math expression strings |
| `parsing/parser.hpp` | Recursive descent parser (string to AST) |
