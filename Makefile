# Builds the game and the tests. Needs SFML 3, a C++17 compiler and make.
# (The archived classic version in archive/classic builds with CMake.)
#   make            build bin/rungame and bin/minesweeper_test
#   make run        build and start the game
#   make test       build and run the logic tests
#   make clean      remove obj/, lib/ and bin/ (for every platform)
# Change the compiler with: make COMPILER=clang++
#
# Layout: sources in src/, headers in inc/. Build output goes to obj/<platform>/ (object files and
# header dependencies), lib/<platform>/ (the game logic as a static library) and bin/ (programs);
# all are gitignored. <platform> is the compiler's target, e.g. x86_64-w64-mingw32 or
# x86_64-linux-gnu, so a Windows build and a WSL build of the same folder do not mix their files.
#
# Works with GNU make in a Unix shell (Linux, macOS, WSL, MSYS2) and with mingw32-make running
# recipes through Windows cmd.exe (e.g. CLion with an MSYS2 toolchain).

COMPILER ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CPPFLAGS += -Iinc
DEPFLAGS = -MMD -MP

# The platform the compiler builds for (asking the compiler works in cmd, PowerShell and Unix
# shells alike)
MACHINE := $(strip $(shell $(COMPILER) -dumpmachine))

SRC_DIR := src
OBJ_ROOT := obj
LIB_ROOT := lib
BIN_DIR := bin
OBJ_DIR := $(OBJ_ROOT)/$(MACHINE)
LIB_DIR := $(LIB_ROOT)/$(MACHINE)

# Windows compilers (MinGW) write name.exe even for "-o name"
ifneq ($(findstring mingw,$(MACHINE)),)
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
else
CMD_SHELL :=
NULL_DEVICE := /dev/null
endif

# pkg-config finds SFML's include and library paths (apt, MSYS2, Homebrew); without it, fall
# back to the plain library names.
SFML_CFLAGS := $(shell pkg-config --cflags sfml-graphics 2>$(NULL_DEVICE))
SFML_LIBS := $(shell pkg-config --libs sfml-graphics 2>$(NULL_DEVICE))
ifeq ($(strip $(SFML_LIBS)),)
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system
endif

GAME := $(BIN_DIR)/rungame$(EXE)
TESTS := $(BIN_DIR)/minesweeper_test$(EXE)
CORE_LIB := $(LIB_DIR)/libminesweeper.a

CORE_OBJECTS := $(OBJ_DIR)/minesweeper.o
GUI_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_DIR)/main.cpp $(wildcard $(SRC_DIR)/gui/*.cpp))
TEST_OBJECTS := $(OBJ_DIR)/tests/minesweeper_test.o
ALL_OBJECTS := $(CORE_OBJECTS) $(GUI_OBJECTS) $(TEST_OBJECTS)

# Folder creation, running and cleaning differ between cmd.exe and a Unix shell
ifeq ($(CMD_SHELL),1)
make_dir = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
run_program = $(subst /,\,$(1))
remove_build = (if exist $(OBJ_ROOT) rmdir /S /Q $(OBJ_ROOT)) & (if exist $(LIB_ROOT) rmdir /S /Q $(LIB_ROOT)) & (if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR))
else
make_dir = mkdir -p $(1)
run_program = ./$(1)
remove_build = rm -rf $(OBJ_ROOT) $(LIB_ROOT) $(BIN_DIR)
endif

# Default target; IDEs such as CLion run "make all" when importing a Makefile project.
all: $(GAME) $(TESTS)

# Short names: "make rungame", "make minesweeper_test"
rungame: $(GAME)
minesweeper_test: $(TESTS)

$(GAME): $(GUI_OBJECTS) $(CORE_LIB)
	@$(call make_dir,$(BIN_DIR))
	$(COMPILER) $(GUI_OBJECTS) -L$(LIB_DIR) -lminesweeper -o $@ $(SFML_LIBS) $(GUI_LDFLAGS)

$(TESTS): $(TEST_OBJECTS) $(CORE_LIB)
	@$(call make_dir,$(BIN_DIR))
	$(COMPILER) $(TEST_OBJECTS) -L$(LIB_DIR) -lminesweeper -o $@

$(CORE_LIB): $(CORE_OBJECTS)
	@$(call make_dir,$(LIB_DIR))
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call make_dir,$(dir $@))
	$(COMPILER) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) $(SFML_CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp
	@$(call make_dir,$(dir $@))
	$(COMPILER) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

run: $(GAME)
	$(call run_program,$(GAME))

test: $(TESTS)
	$(call run_program,$(TESTS))

clean:
	$(remove_build)

# Header dependencies written by -MMD: a changed header rebuilds the files that include it
-include $(ALL_OBJECTS:.o=.d)

# CLion (and other IDEs) learn the compile commands from a dry run: make --just-print all. When the
# build is up to date that would print nothing ("No compilation commands found"), so in a dry run
# every object counts as out of date. Real builds stay incremental.
ifneq ($(findstring n,$(firstword -$(MAKEFLAGS))),)
$(ALL_OBJECTS): force_listing
force_listing:
.PHONY: force_listing
endif

.PHONY: all rungame minesweeper_test run test clean
