cmake_minimum_required(VERSION 3.31)
project(symaths)

set(CMAKE_CXX_STANDARD 23)

option(SYMLIB_INSTALL
        "Specifies whether to install the core library or not"
        OFF
)

option(SYMLIB_INSTALL_DIR
        "Specifies the directory to install the core library to"
        out/install
)

add_subdirectory(lib)
add_subdirectory(tests)
