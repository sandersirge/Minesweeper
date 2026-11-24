// Minesweeper with a graphical interface: a menu (difficulty level or a custom board) and the
// game screen. The game logic is in Minesweeper.h/.cpp, the interface in gui/.
#include "gui/App.hpp"

int main() {
	App app;
	return app.run();
}
