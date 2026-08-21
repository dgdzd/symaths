<p align="center">
  <img width="700" alt="logo-transparent" src="https://github.com/user-attachments/assets/43898759-afd4-40ef-b5e7-ec9c967e668f" />
</p>

# symaths

symaths is a C++ mathematical toolkit consisting of three components:

- **[symaths core](lib/symaths/README.md)** -- a symbolic math expression engine providing parsing, evaluation, differentiation, equation solving, polynomial decomposition, and a mini programming language
- **[MathGen](lib/mathgen/README.md)** -- a symbolic regression engine based on genetic programming that evolves mathematical expression trees to fit datasets
- **symaths CLI** -- an interactive terminal math shell (TUI) for evaluating symbolic math expressions in real time

## Requirements

- A C++23 compatible compiler
- [CMake](https://cmake.org/) >= 3.31

## Building

```bash
git clone https://github.com/dgdzd/symaths.git
cd symaths
cmake -B build
cmake --build build
```

### CMake options

| Option                         | Default                           | Description                            |
|--------------------------------|-----------------------------------|----------------------------------------|
| `SYMLIB_INSTALL`               | `OFF`                             | Install the core library               |
| `SYMLIB_INSTALL_DIR`           | `${CMAKE_INSTALL_PREFIX}/symaths` | Install path for the core library      |
| `SYMTOOLS_CMDLINE_BUILD`       | `ON`                              | Build the command-line tool            |
| `SYMTOOLS_CMDLINE_INSTALL`     | `ON`                              | Install the command-line tool          |
| `SYMTOOLS_CMDLINE_INSTALL_DIR` | `${CMAKE_INSTALL_PREFIX}/symaths` | Install path for the command-line tool |

To build only the libraries without the CLI:

```bash
cmake -B build -DSYMTOOLS_CMDLINE_BUILD=OFF
cmake --build build
```

## Installing

```bash
cmake -B build -DSYMLIB_INSTALL=ON
cmake --build build
cmake --install build
```

To install to a custom prefix:

```bash
cmake --install build --prefix /path/to/install
```

> **Note (Windows):** The default install prefix (`C:/Program Files (x86)/symaths`) requires administrator privileges. Use `--prefix` to install to a user-writable location:
> ```bash
> cmake --install build --prefix "$HOME/symaths"
> ```

## Using the core library in your project

### Using FetchContent (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(symaths
        GIT_REPOSITORY https://github.com/dgdzd/symaths
        GIT_TAG v26.0.1b # Or any tag or branch you want
)
FetchContent_MakeAvailable(symaths)

target_link_libraries(your_target PRIVATE symaths::core)
```

### As a CMake subdirectory

```cmake
add_subdirectory(path/to/symaths)
target_link_libraries(your_target PRIVATE symaths::core)
```

### After installing

```cmake
find_package(symaths CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE symaths::core)
```

## Documentation

- [symaths core library](lib/symaths/README.md) -- symbolic math engine API and usage
- [MathGen](lib/mathgen/README.md) -- symbolic regression engine, operators, Island Model, and full API reference

## License

This project is licensed under the GNU General Public License v2. See [LICENSE.txt](LICENSE.txt) for details.
