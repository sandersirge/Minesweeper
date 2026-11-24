# Builds the game as rungame (rungame.exe on Windows). Needs SFML 3, a C++17 compiler and make.
# (The archived classic version in archive/classic builds with CMake.)
#   make            build the game and the tests
#   make run        build and start the game
#   make test       build and run the logic tests
#   make clean      remove build output
# Change the compiler with: make COMPILER=clang++
#
# Works with GNU make in a Unix shell (Linux, macOS, WSL, MSYS2) and with mingw32-make running
# recipes through Windows cmd.exe (e.g. CLion with an MSYS2 toolchain).

COMPILER ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CPPFLAGS += -I.

# Windows compilers (MinGW) write name.exe even for "-o name"; ask the compiler which platform
# it builds for, so this is right in cmd, PowerShell and Unix shells alike.
ifneq ($(findstring mingw,$(shell $(COMPILER) -dumpmachine)),)
EXE := .exe
# A GUI application: no console window next to the game
GUI_LDFLAGS := -mwindows
else
EXE :=
GUI_LDFLAGS :=
endif

# cmd.exe keeps the quotes in `echo "x"`, a Unix shell drops them.
ifeq ($(shell echo "x"),"x")
CMD_SHELL := 1
NULL_DEVICE := NUL
RUN_PREFIX := .\\
else
CMD_SHELL :=
NULL_DEVICE := /dev/null
RUN_PREFIX := ./
endif

# pkg-config finds SFML's include and library paths (apt, MSYS2, Homebrew); without it, fall
# back to the plain library names.
SFML_CFLAGS := $(shell pkg-config --cflags sfml-graphics 2>$(NULL_DEVICE))
SFML_LIBS   := $(shell pkg-config --libs sfml-graphics 2>$(NULL_DEVICE))
ifeq ($(strip $(SFML_LIBS)),)
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system
endif

GAME := rungame$(EXE)
TESTS := minesweeper_test$(EXE)
GUI_SOURCES := $(wildcard gui/*.cpp)
HEADERS := Minesweeper.h $(wildcard gui/*.hpp)
OBJECTS := main.o Minesweeper.o $(GUI_SOURCES:.cpp=.o)
TEST_OBJECTS := tests/MinesweeperTest.o Minesweeper.o
CLEAN_FILES := $(sort $(OBJECTS) $(TEST_OBJECTS)) $(GAME) $(TESTS)

# Default target; IDEs such as CLion run "make all" when importing a Makefile project.
all: $(GAME) $(TESTS)

ifneq ($(EXE),)
# Short names, so "make rungame" also works where the file is rungame.exe
rungame: $(GAME)
minesweeper_test: $(TESTS)
.PHONY: rungame minesweeper_test
endif

$(GAME): $(OBJECTS)
	$(COMPILER) $(OBJECTS) -o $@ $(SFML_LIBS) $(GUI_LDFLAGS)

$(TESTS): $(TEST_OBJECTS)
	$(COMPILER) $(TEST_OBJECTS) -o $@

%.o: %.cpp $(HEADERS)
	$(COMPILER) $(CXXFLAGS) $(CPPFLAGS) $(SFML_CFLAGS) -c $< -o $@

run: $(GAME)
	$(RUN_PREFIX)$(GAME)

test: $(TESTS)
	$(RUN_PREFIX)$(TESTS)

clean:
ifeq ($(CMD_SHELL),1)
	-del /Q $(subst /,\,$(CLEAN_FILES)) 2>NUL
else
	rm -f $(CLEAN_FILES)
endif

.PHONY: all run test clean
