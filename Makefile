# Builds the game and the tests. Needs SFML 3, a C++17 compiler and make.
# (The archived classic version in archive/classic builds with CMake.)
#   make            build bin/rungame and bin/minesweeper_test
#   make run        build and start the game
#   make test       build and run the logic tests
#   make clean      remove obj/, lib/ and bin/
# Change the compiler with: make COMPILER=clang++
#
# Layout: sources in src/, headers in inc/. Build output goes to obj/ (object files and header
# dependencies), lib/ (the game logic as a static library) and bin/ (programs); all are gitignored.
#
# Works with GNU make in a Unix shell (Linux, macOS, WSL, MSYS2) and with mingw32-make running
# recipes through Windows cmd.exe (e.g. CLion with an MSYS2 toolchain).

COMPILER ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CPPFLAGS += -Iinc
DEPFLAGS = -MMD -MP

SRC_DIR := src
OBJ_DIR := obj
LIB_DIR := lib
BIN_DIR := bin

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
remove_build = (if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR)) & (if exist $(LIB_DIR) rmdir /S /Q $(LIB_DIR)) & (if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR))
else
make_dir = mkdir -p $(1)
run_program = ./$(1)
remove_build = rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
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

.PHONY: all rungame minesweeper_test run test clean
