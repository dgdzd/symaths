cmake_minimum_required(VERSION 3.31)
project(symaths)

set(CMAKE_CXX_STANDARD 23)

option(SYMLIB_INSTALL
        "Specifies whether to install the core library or not"
        OFF
)

option(SYMLIB_INSTALL_DIR
        "Specifies the directory to install the core library to"
        ${CMAKE_INSTALL_PREFIX}/symaths
)

option(SYMTOOLS_CMDLINE_BUILD
        "Specifies whether to build the command line tool or not"
        ON
)

option(SYMTOOLS_CMDLINE_INSTALL
        "Specifies whether to install the command line tool or not"
        ON
)

option(SYMTOOLS_CMDLINE_INSTALL_DIR
        "Specifies the directory to install the command line tool to"
        ${CMAKE_INSTALL_PREFIX}/symaths
)

add_subdirectory(lib)
add_subdirectory(tests)
if (SYMTOOLS_CMDLINE_BUILD)
    add_subdirectory(main)
endif()
