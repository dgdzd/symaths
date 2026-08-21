cmake_minimum_required(VERSION 3.31)
project(symaths VERSION 26.0.1)

set(CMAKE_CXX_STANDARD 23)

option(SYMATHS_BUILD
        "Specifies whether to build the symaths library and executable or not."
        ON
)

option(MATHGEN_BUILD
        "Specifies whether to build the MathGen library or not."
        OFF
)

option(SYMATHS_BUILD_TESTS
        "Specifies whether to build symaths unit tests or not."
        ON
)

option(MATHGEN_BUILD_TESTS
        "Specifies whether to build MathGen unit tests or not."
        ON
)

option(SYMATHS_INSTALL
        "Specifies whether to install the core library or not."
        ON
)

set(SYMATHS_INSTALL_DIR
        "${CMAKE_INSTALL_PREFIX}"
        CACHE PATH "Specifies the directory to install the core library to."
)

option(SYMTOOLS_CMDLINE_BUILD
        "Specifies whether to build the command line tool or not."
        ON
)

option(SYMTOOLS_CMDLINE_INSTALL
        "Specifies whether to install the command line tool or not."
        ON
)

message("-- ┌─ symaths ${CMAKE_PROJECT_VERSION} ──────────────────────────")
message("-- │ SYMATHS_BUILD            : ${SYMATHS_BUILD}")
message("-- │ MATHGEN_BUILD            : ${MATHGEN_BUILD}")
message("-- │ SYMATHS_BUILD_TESTS      : ${SYMATHS_BUILD_TESTS}")
message("-- │ MATHGEN_BUILD_TESTS      : ${MATHGEN_BUILD_TESTS}")
message("-- │ SYMATHS_INSTALL          : ${SYMATHS_INSTALL}")
message("-- │ SYMATHS_INSTALL_DIR      : ${SYMATHS_INSTALL_DIR}")
message("-- │ SYMTOOLS_CMDLINE_BUILD   : ${SYMTOOLS_CMDLINE_BUILD}")
message("-- │ SYMTOOLS_CMDLINE_INSTALL : ${SYMTOOLS_CMDLINE_INSTALL}")
message("-- └───────────────────────────────────────────")

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(tests)
if (SYMTOOLS_CMDLINE_BUILD)
    add_subdirectory(main)
endif()

if (SYMATHS_INSTALL)
    if (NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "${SYMATHS_INSTALL_DIR}")
        set(CMAKE_INSTALL_PREFIX ${SYMATHS_INSTALL_DIR})
    endif()

    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

        # detect OS first
        if (APPLE)
            if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
                message(STATUS "Detected platform: macOS ARM64")
                set(new_path "/opt/homebrew/symaths")
            else()
                message(STATUS "Detected platform: macOS x86_64")
                set(new_path "/usr/local/symaths")
            endif()

        elseif (WIN32)
            if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
                message(STATUS "Detected platform: Windows ARM64")
                set(new_path "C:/Program Files/symaths")
            else()
                message(STATUS "Detected platform: Windows x86_64")
                set(new_path "C:/Program Files (x86)/symaths")
            endif()

        elseif (UNIX)
            # sous UNIX non-Apple → Linux
            if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
                message(STATUS "Detected platform: Linux ARM64")
                set(new_path "/usr/local/symaths")
            else()
                message(STATUS "Detected platform: Linux x86_64")
                set(new_path "/usr/local/symaths")
            endif()

        else()
            message(WARNING "Unknown platform; leaving CMAKE_INSTALL_PREFIX unchanged")
        endif()

        # on installe si on a bien défini new_path
        if (DEFINED new_path)
            message(STATUS "CMAKE_INSTALL_PREFIX will be set to: ${new_path}")
            set(CMAKE_INSTALL_PREFIX
                    ${new_path}
                    CACHE PATH "Where the library will be installed to" FORCE
            )
        endif()

    else()
        message(
                STATUS
                "CMAKE_INSTALL_PREFIX was already set\n"
                "Current value: ${CMAKE_INSTALL_PREFIX}"
        )
    endif()

    include(CMakePackageConfigHelpers)

    install(EXPORT symathsTargets
            FILE symathsTargets.cmake
            NAMESPACE symaths::
            DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/symathsConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/symathsConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/symathsConfigVersion.cmake"
            COMPATIBILITY SameMajorVersion
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/symathsConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/symathsConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
    )
endif()
