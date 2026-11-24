// Minesweeper with a graphical interface: a menu (difficulty level or a custom board) and the
// game screen. The game logic is in inc/minesweeper.hpp and src/minesweeper.cpp, the interface in src/gui and
// inc/gui.
#include "gui/app.hpp"

int main() {
	App app;
	return app.run();
}
