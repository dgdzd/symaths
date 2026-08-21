# lib/

This directory contains the two static libraries that make up the core of symaths.

| Directory | Target | CMake alias | Description |
|-----------|--------|-------------|-------------|
| `symaths/` | `symaths_lib` | `symaths::core` | Symbolic math expression engine (parsing, evaluation, differentiation, polynomials, equations) |
| `mathgen/` | `mathgen_lib` | -- | Genetic-programming symbolic regression engine |

Both libraries are built as static libraries and can be linked into other C++ projects via `add_subdirectory` or after installation.
