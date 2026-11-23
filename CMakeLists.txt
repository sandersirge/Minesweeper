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

# Game logic as a library, shared by the game and the tests.
add_library(minesweeper_core STATIC Minesweeper.cpp)
target_include_directories(minesweeper_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minesweeper_core PUBLIC SFML::Graphics)
if(MSVC)
    target_compile_options(minesweeper_core PUBLIC /W4)
else()
    target_compile_options(minesweeper_core PUBLIC -Wall -Wextra)
endif()

add_executable(rungame main.cpp)
target_link_libraries(rungame PRIVATE minesweeper_core)

# The game loads its textures from "Pildid" relative to where it runs; keep a copy next to the
# executable and run it from there (CLion: set the working directory to the build directory).
add_custom_command(TARGET rungame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Pildid $<TARGET_FILE_DIR:rungame>/Pildid
    COMMENT "Copying textures next to rungame")

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
copy_runtime_dlls(rungame)

# Logic tests (no window needed): ctest --test-dir <build folder>
include(CTest)
if(BUILD_TESTING)
    add_executable(minesweeper_test tests/MinesweeperTest.cpp)
    target_link_libraries(minesweeper_test PRIVATE minesweeper_core)
    copy_runtime_dlls(minesweeper_test)
    add_test(NAME minesweeper_logic COMMAND minesweeper_test)
endif()
