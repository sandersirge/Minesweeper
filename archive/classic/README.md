# Classic version (archive)

The first graphical version of the game: the board settings are asked in the terminal, and the game
is played either in the terminal or in a simple window that uses the images in `pildid/`.

<img width="498" height="417" alt="The classic version" src="https://github.com/user-attachments/assets/81f23ec5-67b7-48a9-ab8d-bc5fdde7edcf" />

The current graphical interface (menu, difficulty levels, pixel art) is in `src/gui` and `inc/gui`.
Both versions use the same game logic (`inc/minesweeper.hpp`, `src/minesweeper.cpp`).

Build it from the project root:

```shell
cmake -S . -B build -G Ninja
cmake --build build --target rungame_classic
cd build/bin && ./rungame_classic
```

CMake copies `pildid/` next to the program; start it from `build/bin` so the images are found. The
Makefile does not build this version. In the terminal, type the answers and press Enter; Ctrl+D
(Ctrl+Z on Windows) ends the input and closes the game.
