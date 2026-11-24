#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

// Game logic. It does not depend on SFML: the GUI (gui/) and the archived classic version
// (archive/classic/) draw the board through cell().

// Thrown when input ends (Ctrl+D / Ctrl+Z or the end of piped input), so the program can close
// cleanly instead of asking the same question forever.
struct InputEnded : std::runtime_error {
	InputEnded() : std::runtime_error("input ended") {}
};

// Asks for an integer in [min, max] until it gets one. Invalid input (e.g. letters) is skipped and
// the question is asked again; at the end of input InputEnded is thrown.
int readInt(const std::string& prompt, int min, int max);

class Minesweeper {
public:
	// Board cells as the player sees them
	static constexpr char HIDDEN = '?';     // not opened yet
	static constexpr char EMPTY = ' ';      // opened, no mines around
	static constexpr char FLAG = '!';       // flagged by the player
	static constexpr char MINE = 'X';       // mine (shown after a loss)
	static constexpr char EXPLODED = '*';   // the mine the player opened
	static constexpr char WRONG_FLAG = 'V'; // flag on a cell without a mine (shown after a loss)
	// '1'...'8': opened cell, the number of mines around it

	// Limits for the terminal version: the window is 640x480 pixels, at least 1 pixel per cell
	static constexpr int MAX_ROWS = 480;
	static constexpr int MAX_COLUMNS = 640;

	// Asks for the board settings in the terminal and starts a new game.
	void askBoardSettings();
	// Starts a new game with the mine count given as a percentage. Mines are placed on the first
	// opened cell, so the first move is always safe.
	void newGame(int rows, int columns, int minePercent, bool graphical);
	// Same, with an exact mine count (GUI difficulty levels). The count is capped so that at least
	// one cell stays free.
	void newGameWithMines(int rows, int columns, size_t mines);
	// Places mines at fixed cells right away (for tests); the first move is not protected then.
	void setMines(const std::vector<bool>& mineCells);
	// Whether game messages (e.g. "See ruut on juba avatud.") are printed to the terminal. The GUI
	// turns them off.
	void setMessages(bool print) { printMessages = print; }

	// Opens a cell or places/removes a flag. Returns false if the move was not allowed.
	bool makeMove(int row, int column, bool flag);
	// "Chord": if an opened number has as many flags around it as it shows, all other neighbours
	// are opened (a wrongly placed flag can open a mine). Returns whether anything was opened.
	bool chord(int row, int column);
	void moveFromTerminal();
	void printBoard() const;

	char cell(int row, int column) const;
	int rows() const { return rowCount; }
	int columns() const { return columnCount; }
	size_t mineCount() const { return totalMines; }
	size_t flagCount() const;
	bool hasStarted() const { return minesPlaced; } // the first cell has been opened
	bool isOver() const { return over; }
	bool isLost() const { return lost; }
	bool wantsGraphics() const { return graphics; }

private:
	// Indices of up to 8 neighbours without allocating (called hundreds of thousands of times on
	// a large board)
	struct Neighbours {
		std::array<size_t, 8> indices{};
		size_t count{0};
		const size_t* begin() const { return indices.data(); }
		const size_t* end() const { return indices.data() + count; }
		size_t size() const { return count; }
	};

	bool over{false};
	bool lost{false};
	bool graphics{false};
	bool minesPlaced{false};
	bool printMessages{true};
	int rowCount{0};
	int columnCount{0};
	size_t cellCount{0};
	size_t totalMines{0};
	size_t openedCount{0};

	// The grids are stored as one long vector: cell (row, column) is at index row * columns + column
	std::vector<bool> mines;   // where the mines are
	std::vector<int> adjacent; // number of mines around each cell (-1 on a mine)
	std::vector<char> board;   // what the player sees (see the constants above)

	Neighbours neighbours(size_t index) const;
	void message(const char* text) const;
	void placeMines(size_t safeIndex);
	void countAdjacent();
	// Opens a cell (and its empty area). Returns false if a mine was opened.
	bool openCell(size_t index);
	void floodOpen(size_t startIndex);
	void checkWin();
	void lose(size_t explodedIndex);
};
