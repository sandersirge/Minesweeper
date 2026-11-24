# Building and running

This guide covers every way to build, run and test the game: the **Makefile**, **CMake**, and
**CLion**, on Windows (MSYS2), Linux/WSL and macOS. All commands are run from the project root
(the folder that contains `Makefile` and `CMakeLists.txt`) unless stated otherwise.

- [1. Install the tools](#1-install-the-tools)
- [2. Build with the Makefile](#2-build-with-the-makefile)
- [3. Build with CMake](#3-build-with-cmake)
- [4. Set up CLion](#4-set-up-clion)
- [5. Format the code](#5-format-the-code)
- [6. Troubleshooting](#6-troubleshooting)

## 1. Install the tools

You need a C++17 compiler, SFML 3, and `make` and/or CMake.

### Windows (MSYS2)

1. Install MSYS2 (or download the installer from <https://www.msys2.org>):
   ```powershell
   winget install MSYS2.MSYS2
   ```
2. Open the **MSYS2 UCRT64** terminal from the Start menu and install the toolchain and SFML:
   ```shell
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,gdb,cmake,ninja,make,pkgconf,sfml} make
   ```
3. Add `C:\msys64\ucrt64\bin` to your **user** PATH (*Settings → System → About → Advanced system
   settings → Environment Variables*). This lets PowerShell, cmd and CLion find `g++`, `cmake` and
   the SFML DLLs. Open a new terminal afterwards.
4. Check that the MSYS2 tools are the ones that are found:
   ```powershell
   where.exe g++ mingw32-make
   g++ --version
   ```
   The first line for each must be `C:\msys64\ucrt64\bin\...`. If another installation is listed
   first (for example Strawberry Perl's `C:\Strawberry\c\bin`, which is on the *system* PATH and
   therefore searched before the user PATH), its compiler has no SFML and the build fails with
   `SFML/Graphics.hpp: No such file or directory`. Either move that entry below MSYS2 in the system
   PATH (needs administrator rights), or put MSYS2 first for the current PowerShell session:
   ```powershell
   $env:Path = "C:\msys64\ucrt64\bin;$env:Path"
   ```
   The MSYS2 UCRT64 terminal and CLion with the MSYS2 toolchain (section 4) are not affected.

### Linux and WSL (Ubuntu/Debian)

```shell
sudo apt update
sudo apt install build-essential cmake ninja-build pkg-config gdb libsfml-dev
```

Ubuntu 26.04 and newer ship SFML 3 as `libsfml-dev`. On older releases (e.g. 24.04, which has
SFML 2.6) skip `libsfml-dev`: CMake then downloads and builds SFML 3 itself, which needs these
development packages instead:

```shell
sudo apt install libxrandr-dev libxcursor-dev libxi-dev libudev-dev libfreetype-dev libgl1-mesa-dev libegl1-mesa-dev
```

Under WSL the game window opens on the Windows desktop (WSLg, Windows 11).

### macOS

```shell
brew install sfml cmake ninja pkgconf
```

## 2. Build with the Makefile

The Makefile builds the game and the tests. Output goes to three folders (all ignored by git):

| Folder | Contents |
|---|---|
| `obj/` | object files (`.o`) and header dependency files (`.d`), mirroring `src/` |
| `lib/` | the game logic as a static library, `libminesweeper.a` |
| `bin/` | the programs: `rungame` and `minesweeper_test` (`.exe` on Windows) |

### Commands

On Linux, macOS, WSL and in the MSYS2 terminal the command is `make`. In PowerShell or cmd on
Windows use `mingw32-make` (same arguments).

| Command | What it does |
|---|---|
| `make` or `make all` | builds `bin/rungame` and `bin/minesweeper_test` |
| `make rungame` | builds only the game |
| `make minesweeper_test` | builds only the tests |
| `make run` | builds and starts the game |
| `make test` | builds and runs the tests |
| `make clean` | deletes `obj/`, `lib/` and `bin/` |
| `make -j8` | builds with 8 parallel jobs (any number works) |
| `make COMPILER=clang++` | uses another compiler (default `g++`) |
| `make CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra"` | debug build (no optimisation, debug info) |

Examples:

```shell
# Linux / macOS / WSL / MSYS2 terminal
make -j8
make test
./bin/rungame
```

```powershell
# PowerShell or cmd on Windows
mingw32-make -j8
mingw32-make test
.\bin\rungame.exe
```

Only changed files are rebuilt; when a header changes, every file that includes it is rebuilt.
After switching compilers or flags run `make clean` first.

## 3. Build with CMake

CMake builds the game, the archived classic version and the tests into a separate build folder.
Inside it, programs (and on Windows the SFML DLLs) go to `bin/`, the library to `lib/`.

### Configure and build

```shell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

- `-S .` is the source folder, `-B build` the build folder (any name; `build/` is gitignored).
- `-G Ninja` uses Ninja (fast). Without Ninja use `-G "Unix Makefiles"` (Linux/macOS) or
  `-G "MinGW Makefiles"` (Windows).
- `CMAKE_BUILD_TYPE` is `Release` (optimised) or `Debug` (default, for debugging).

Configure once; afterwards `cmake --build build` is enough, even after editing `CMakeLists.txt`.

### Targets and running

| Command | What it does |
|---|---|
| `cmake --build build` | builds everything |
| `cmake --build build --target rungame` | only the game |
| `cmake --build build --target rungame_classic` | only the archived classic version |
| `cmake --build build --target minesweeper_test` | only the tests |
| `cmake --build build --parallel 8` | parallel build |
| `cmake --build build --target clean` | deletes the build output (keeps the configuration) |
| `ctest --test-dir build --output-on-failure` | runs the tests |

```shell
./build/bin/rungame                    # Linux / macOS / WSL / MSYS2
cd build/bin && ./rungame_classic      # the classic version (run it from bin/, it loads pildid/)
```

```powershell
.\build\bin\rungame.exe                # PowerShell
```

### Options

| Option | Default | Meaning |
|---|---|---|
| `-DMINESWEEPER_FETCH_SFML=ON/OFF` | `ON` | download and build SFML 3 if it is not installed |
| `-DMINESWEEPER_BUILD_ARCHIVE=ON/OFF` | `ON` | build the archived classic version |
| `-DBUILD_TESTING=ON/OFF` | `ON` | build the tests |

Example of a debug build without the classic version:

```shell
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMINESWEEPER_BUILD_ARCHIVE=OFF
cmake --build build-debug
```

To start over, delete the build folder and configure again.

## 4. Set up CLion

### Toolchain (Windows)

*Settings → Build, Execution, Deployment → Toolchains*: add a **MinGW** toolchain, set **Toolset**
to `C:\msys64\ucrt64` and move it to the top so it is the default. CMake and the debugger can stay
**Bundled**. With *Bundled MinGW* the build still works, but CMake has to download and build SFML
itself (slow the first time).

### Option A: open as a CMake project (recommended)

Right-click `CMakeLists.txt` → **Load CMake Project** (or open the folder in a fresh CLion). CLion
creates the run configurations `rungame`, `rungame_classic` and `minesweeper_test` itself; pick
one in the toolbar and press ▶. Tests can also be run from *Run → Run 'All CTest'*.

### Option B: open as a Makefile project

CLion reads the Makefile by doing a dry run of `make all`. If the build output already exists that
dry run prints nothing, so after moving or renaming files use **Tools → Makefile → Clean and
Reload Makefile Project**.

Run configurations are not created with the executable filled in. To set one up:

1. Build once (hammer icon or *Build → Build Project*) so `bin\rungame.exe` exists.
2. *Run → Edit Configurations…* and select **rungame** (under *Makefile Application* or *Native
   Application*).
3. **Target:** `rungame`.
4. **Executable:** open the dropdown → **Select other…** (or the **…** button) and choose
   `bin\rungame.exe` in the project folder.
5. **Working directory:** `$ProjectFileDir$` (optional).
6. **Before launch:** keep **Build**. Then **OK** and ▶.

Repeat with target `minesweeper_test` and `bin\minesweeper_test.exe` for the tests.

## 5. Format the code

The style is defined in `.clang-format` (LLVM based, tabs, 110 columns). In CLion use
*Code → Reformat Code* (Ctrl+Alt+L); CLion ships its own clang-format. From the command line install
it first (MSYS2 package `mingw-w64-ucrt-x86_64-clang`, apt package `clang-format`, or
`brew install clang-format`), then:

```shell
clang-format -i src/*.cpp src/gui/*.cpp inc/*.hpp inc/gui/*.hpp tests/*.cpp archive/classic/main.cpp
```

The pixel-art bitmaps in `src/gui/icons.cpp` are excluded with `// clang-format off` so each row
stays on its own line.

## 6. Troubleshooting

**"No compilation commands found" / "file does not belong to any target" in CLion** (Makefile
project): the dry run saw an up-to-date build. Use *Tools → Makefile → Clean and Reload Makefile
Project*, or open the project with CMake instead.

**"The code execution cannot proceed because libsfml-graphics-3.dll was not found"**: Windows cannot
find the SFML DLLs. Add `C:\msys64\ucrt64\bin` to your PATH and restart the terminal or CLion. CMake
also copies the DLLs next to the programs in `build/bin`.

**`SFML/Graphics.hpp: No such file or directory` on Windows, or the wrong `g++`/`cmake` is used**:
another installation earlier on the PATH wins, for example Strawberry Perl's `C:\Strawberry\c\bin`
in the *system* PATH, which Windows searches before the user PATH. Check with
`where.exe g++ mingw32-make`. For the current PowerShell session run
`$env:Path = "C:\msys64\ucrt64\bin;$env:Path"`, or use the MSYS2 UCRT64 terminal or CLion with the
MSYS2 toolchain. To fix it permanently, move `C:\Strawberry\c\bin` below MSYS2 in the system PATH
(needs administrator rights).

**`make` is not recognised in PowerShell**: MSYS2's make for Windows is called `mingw32-make`; plain
`make` exists only inside the MSYS2 terminal.

**CMake cannot find SFML 3**: install it as shown in section 1, or leave `MINESWEEPER_FETCH_SFML` on
so CMake downloads it. On Linux, the download needs the development packages listed above.

**The classic version says "Pildifailist … sisselugemine ebaõnnestus!"**: it loads its images from
`pildid/` next to where it is started. Start it from `build/bin`.
