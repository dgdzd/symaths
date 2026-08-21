# lib/
# Add working submodules

if (SYMATHS_BUILD)
    add_subdirectory(symaths)
endif()

if (MATHGEN_BUILD)
    add_subdirectory(mathgen)
endif()
