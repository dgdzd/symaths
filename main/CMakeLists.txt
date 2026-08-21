# main/

include(FetchContent)
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v7.0.1
)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ftxui)

set(SYMATHS_CMD_EXE_SOURCES
        main.cpp
        src/app.cpp
        src/utils/strings.cpp
)

add_executable(symaths_cmd_exe ${SYMATHS_CMD_EXE_SOURCES})


target_include_directories(symaths_cmd_exe PRIVATE
        include
)

target_sources(symaths_cmd_exe PRIVATE
        ${SYMATHS_CMD_EXE_SOURCES}
)

target_link_libraries(symaths_cmd_exe PRIVATE
        symaths::core
        ftxui::ftxui
)

set_target_properties(symaths_cmd_exe PROPERTIES OUTPUT_NAME symaths)

if (SYMTOOLS_CMDLINE_INSTALL)
    install(TARGETS symaths_cmd_exe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()