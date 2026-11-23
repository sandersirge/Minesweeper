# Builds the game as ./rungame (rungame.exe on Windows). Needs SFML 3, a C++17 compiler and make.
#   make            build
#   make run        build and start the game
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

rungame: $(OBJECTS)
	$(COMPILER) $(OBJECTS) -o rungame $(SFML_LIBS)

%.o: %.cpp Minesweeper.h
	$(COMPILER) $(CXXFLAGS) $(SFML_CFLAGS) -c $< -o $@

run: rungame
	./rungame

clean:
	rm -f $(OBJECTS) rungame rungame.exe

.PHONY: run clean
