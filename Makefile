# Builds the game as ./rungame (rungame.exe on Windows). Needs SFML 3, a C++17 compiler and make.
#   make            build
#   make run        build and start the game
#   make test       build and run the logic tests
#   make clean      remove build output
# Change the compiler with: make COMPILER=clang++

COMPILER ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

# pkg-config finds SFML's include and library paths (apt, MSYS2, Homebrew); without it, fall
# back to the plain library names.
SFML_CFLAGS := $(shell pkg-config --cflags sfml-graphics 2>/dev/null)
SFML_LIBS   := $(shell pkg-config --libs sfml-graphics 2>/dev/null)
ifeq ($(strip $(SFML_LIBS)),)
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system
endif

OBJECTS := main.o Minesweeper.o
TEST_OBJECTS := tests/MinesweeperTest.o Minesweeper.o

rungame: $(OBJECTS)
	$(COMPILER) $(OBJECTS) -o rungame $(SFML_LIBS)

minesweeper_test: $(TEST_OBJECTS)
	$(COMPILER) $(TEST_OBJECTS) -o minesweeper_test $(SFML_LIBS)

%.o: %.cpp Minesweeper.h
	$(COMPILER) $(CXXFLAGS) $(SFML_CFLAGS) -c $< -o $@

run: rungame
	./rungame

test: minesweeper_test
	./minesweeper_test

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) rungame rungame.exe minesweeper_test minesweeper_test.exe

.PHONY: run test clean
