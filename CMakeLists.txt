cmake_minimum_required(VERSION 3.22)
project(Minesweeper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# SFML 3: use the system package (apt libsfml-dev, MSYS2 mingw-w64-ucrt-x86_64-sfml) when it is
# installed; otherwise download and build it, so the project also builds on a bare machine.
option(MINESWEEPER_FETCH_SFML "Download and build SFML if it is not installed" ON)

find_package(SFML 3 QUIET COMPONENTS Graphics Window System)
if(SFML_FOUND)
    message(STATUS "Using installed SFML ${SFML_VERSION}")
elseif(MINESWEEPER_FETCH_SFML)
    message(STATUS "SFML 3 not found; downloading and building it (first configure only)")
    include(FetchContent)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SFML)
else()
    message(FATAL_ERROR "SFML 3 not found. Install it or configure with -DMINESWEEPER_FETCH_SFML=ON.")
endif()

# Warnings for every target of this project (not for a downloaded SFML)
function(minesweeper_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

# On Windows copy the SFML DLLs next to an executable so it starts from anywhere.
function(copy_runtime_dlls target)
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
                    $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${target}>>,copy_if_different,true>
                    $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS)
    endif()
endfunction()

# Game logic (no SFML), shared by the new GUI, the archived version and the tests.
add_library(minesweeper_core STATIC Minesweeper.cpp)
target_include_directories(minesweeper_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
minesweeper_warnings(minesweeper_core)

# The game: menu and board drawn as pixel art in code (gui/), no image files needed.
add_executable(rungame
    main.cpp
    gui/App.cpp
    gui/Canvas.cpp
    gui/GameScreen.cpp
    gui/Icons.cpp
    gui/MenuScreen.cpp
    gui/PixelFont.cpp
    gui/Screen.cpp)
target_link_libraries(rungame PRIVATE minesweeper_core SFML::Graphics)
minesweeper_warnings(rungame)
if(MINGW)
    # A GUI application: no console window next to the game
    target_link_options(rungame PRIVATE -mwindows)
endif()
copy_runtime_dlls(rungame)

# The archived first version (terminal + simple window with the Pildid textures).
option(MINESWEEPER_BUILD_ARCHIVE "Build the archived classic version" ON)
if(MINESWEEPER_BUILD_ARCHIVE)
    add_executable(rungame_classic archive/classic/main.cpp)
    target_link_libraries(rungame_classic PRIVATE minesweeper_core SFML::Graphics)
    minesweeper_warnings(rungame_classic)
    add_custom_command(TARGET rungame_classic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/archive/classic/Pildid
                $<TARGET_FILE_DIR:rungame_classic>/Pildid
        COMMENT "Copying the classic version's textures")
    copy_runtime_dlls(rungame_classic)
endif()

# Logic tests (no window or SFML needed): ctest --test-dir <build folder>
include(CTest)
if(BUILD_TESTING)
    add_executable(minesweeper_test tests/MinesweeperTest.cpp)
    target_link_libraries(minesweeper_test PRIVATE minesweeper_core)
    minesweeper_warnings(minesweeper_test)
    add_test(NAME minesweeper_logic COMMAND minesweeper_test)
endif()
