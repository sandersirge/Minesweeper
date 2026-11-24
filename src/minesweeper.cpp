#include "minesweeper.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>

using namespace std;

namespace {

mt19937& randomEngine() {
	static mt19937 engine{random_device{}()};
	return engine;
}

} // namespace

int readInt(const string& prompt, int min, int max) {
	while (true) {
		cout << prompt;
		int value{};
		if (cin >> value) {
			if (value >= min && value <= max) {
				return value;
			}
			cout << "Sisesta arv vahemikus " << min << "-" << max << ".\n";
			continue;
		}
		if (cin.eof()) {
			throw InputEnded();
		}
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Sisesta täisarv.\n";
	}
}

void Minesweeper::askBoardSettings() {
	int newRows = 0;
	int newColumns = 0;
	size_t cells = 0;
	while (true) {
		newRows = readInt("Vali mängulaua kõrgus (1-" + to_string(MAX_ROWS) + "): ", 1, MAX_ROWS);
		newColumns = readInt("Vali mängulaua laius (1-" + to_string(MAX_COLUMNS) + "): ", 1, MAX_COLUMNS);
		cells = static_cast<size_t>(newRows) * static_cast<size_t>(newColumns);
		if (cells >= 2) {
			break;
		}
		cout << "Laual peab olema vähemalt 2 ruutu (üks pomm ja üks vaba ruut).\n";
	}
	const bool useGraphics = readInt("Kas kuvada graafiline liides? (1/0): ", 0, 1) == 1;

	while (true) {
		const int percent = readInt("Mitu protsenti ruutudest võiksid olla pommid (1-99): ", 1, 99);
		// At least one mine and at least one free cell, otherwise there is nothing to play
		const size_t mineTotal = cells * static_cast<size_t>(percent) / 100;
		if (mineTotal == 0) {
			cout << "Sellise protsendiga ei tuleks ühtegi pommi, vali suurem protsent.\n";
			continue;
		}
		if (mineTotal >= cells) {
			cout << "Sellise protsendiga oleksid kõik ruudud pommid, vali väiksem protsent.\n";
			continue;
		}
		newGame(newRows, newColumns, percent, useGraphics);
		return;
	}
}

void Minesweeper::newGame(int newRows, int newColumns, int minePercent, bool graphical) {
	const size_t cells = static_cast<size_t>(newRows) * static_cast<size_t>(newColumns);
	newGameWithMines(newRows, newColumns, cells * static_cast<size_t>(minePercent) / 100);
	graphics = graphical;
}

void Minesweeper::newGameWithMines(int newRows, int newColumns, size_t mineTotal) {
	if (newRows < 1 || newColumns < 1) {
		throw invalid_argument("the board must be at least 1x1");
	}
	rowCount = newRows;
	columnCount = newColumns;
	cellCount = static_cast<size_t>(rowCount) * static_cast<size_t>(columnCount);
	totalMines = min(mineTotal, cellCount - 1);

	over = false;
	lost = false;
	minesPlaced = false;
	openedCount = 0;
	mines.assign(cellCount, false);
	adjacent.assign(cellCount, 0);
	board.assign(cellCount, HIDDEN);
}

void Minesweeper::setMines(const vector<bool>& mineCells) {
	if (mineCells.size() != cellCount) {
		throw invalid_argument("mine grid has the wrong size");
	}
	mines = mineCells;
	totalMines = static_cast<size_t>(count(mines.begin(), mines.end(), true));
	minesPlaced = true;
	countAdjacent();
}

void Minesweeper::message(const char* text) const {
	if (printMessages) {
		cout << text << '\n';
	}
}

// Up to 8 neighbours; at the edges of a row we do not wrap around to the next/previous row
Minesweeper::Neighbours Minesweeper::neighbours(size_t index) const {
	Neighbours result;
	const int row = static_cast<int>(index / columnCount);
	const int column = static_cast<int>(index % columnCount);
	for (int dr = -1; dr <= 1; dr++) {
		for (int dc = -1; dc <= 1; dc++) {
			if (dr == 0 && dc == 0) {
				continue;
			}
			const int r = row + dr;
			const int c = column + dc;
			if (r < 0 || r >= rowCount || c < 0 || c >= columnCount) {
				continue;
			}
			result.indices[result.count++] = static_cast<size_t>(r) * columnCount + c;
		}
	}
	return result;
}

// Mines are placed on the first opened cell so that the first move is never a mine. If there is
// room, the neighbours of that cell stay free too, so the first move opens an area.
void Minesweeper::placeMines(size_t safeIndex) {
	vector<bool> excluded(cellCount, false);
	excluded.at(safeIndex) = true;
	const Neighbours around = neighbours(safeIndex);
	if (cellCount - 1 - around.size() >= totalMines) {
		for (size_t n : around) {
			excluded.at(n) = true;
		}
	}

	vector<size_t> candidates;
	candidates.reserve(cellCount);
	for (size_t i = 0; i < cellCount; i++) {
		if (!excluded[i]) {
			candidates.push_back(i);
		}
	}
	shuffle(candidates.begin(), candidates.end(), randomEngine());

	mines.assign(cellCount, false);
	for (size_t i = 0; i < totalMines; i++) {
		mines.at(candidates.at(i)) = true;
	}
	minesPlaced = true;
	countAdjacent();
}

void Minesweeper::countAdjacent() {
	adjacent.assign(cellCount, 0);
	for (size_t i = 0; i < cellCount; i++) {
		if (mines[i]) {
			adjacent[i] = -1;
			continue;
		}
		int total = 0;
		for (size_t n : neighbours(i)) {
			if (mines[n]) {
				total++;
			}
		}
		adjacent[i] = total;
	}
}

// Opens a cell; if it is empty, all cells around it are opened too (8 directions) until numbers are
// reached. Uses a work list instead of recursion so a large empty area cannot overflow the stack.
void Minesweeper::floodOpen(size_t startIndex) {
	vector<size_t> pending{startIndex};
	while (!pending.empty()) {
		const size_t index = pending.back();
		pending.pop_back();
		if (board[index] != HIDDEN || mines[index]) {
			continue; // flags stay where they are
		}

		const int number = adjacent[index];
		board[index] = number == 0 ? EMPTY : static_cast<char>('0' + number);
		openedCount++;
		if (number == 0) {
			for (size_t n : neighbours(index)) {
				if (board[n] == HIDDEN) {
					pending.push_back(n);
				}
			}
		}
	}
}

bool Minesweeper::openCell(size_t index) {
	if (!minesPlaced) {
		placeMines(index);
	}
	if (mines[index]) {
		lose(index);
		return false;
	}
	floodOpen(index);
	return true;
}

// After a loss the mines are shown (correct flags stay, wrong flags are marked); all other hidden
// cells stay closed, like in classic Minesweeper.
void Minesweeper::lose(size_t explodedIndex) {
	over = true;
	lost = true;
	for (size_t i = 0; i < cellCount; i++) {
		if (mines[i] && board[i] == HIDDEN) {
			board[i] = MINE;
		} else if (!mines[i] && board[i] == FLAG) {
			board[i] = WRONG_FLAG;
		}
	}
	board[explodedIndex] = EXPLODED;
	message("Avasid pommi!");
}

// Win: every cell except the mines is open (flags are not required)
void Minesweeper::checkWin() {
	if (over || openedCount != cellCount - totalMines) {
		return;
	}
	over = true;
	lost = false;
	for (size_t i = 0; i < cellCount; i++) {
		if (mines[i]) {
			board[i] = FLAG;
		}
	}
}

// Changes the player's board; drawing happens separately (printBoard or the GUI)
bool Minesweeper::makeMove(int row, int column, bool flag) {
	if (over) {
		return false;
	}
	if (row < 0 || row >= rowCount || column < 0 || column >= columnCount) {
		message("Valitud käik ei ole mängulaua sees!");
		return false;
	}
	const size_t index = static_cast<size_t>(row) * columnCount + column;
	const char current = board[index];

	if (flag) {
		if (current == HIDDEN) {
			board[index] = FLAG;
		} else if (current == FLAG) {
			board[index] = HIDDEN; // flagging again removes the flag
		} else {
			message("Avatud ruudule ei saa lippu panna!");
			return false;
		}
		return true;
	}

	if (current == FLAG) {
		message("Lipuga ruutu ei saa avada! (võta enne lipp ära)");
		return false;
	}
	if (current != HIDDEN) {
		message("See ruut on juba avatud.");
		return false;
	}

	if (openCell(index)) {
		checkWin();
	}
	return true;
}

bool Minesweeper::chord(int row, int column) {
	if (over || row < 0 || row >= rowCount || column < 0 || column >= columnCount) {
		return false;
	}
	const size_t index = static_cast<size_t>(row) * columnCount + column;
	const char current = board[index];
	if (current < '1' || current > '8') {
		return false;
	}

	const Neighbours around = neighbours(index);
	int flags = 0;
	for (size_t n : around) {
		if (board[n] == FLAG) {
			flags++;
		}
	}
	if (flags != current - '0') {
		return false;
	}

	bool opened = false;
	for (size_t n : around) {
		if (board[n] != HIDDEN) {
			continue;
		}
		opened = true;
		if (!openCell(n)) {
			return true; // a wrongly placed flag: a mine was opened and the game is over
		}
	}
	checkWin();
	return opened;
}

size_t Minesweeper::flagCount() const {
	return static_cast<size_t>(count(board.begin(), board.end(), FLAG));
}

void Minesweeper::moveFromTerminal() {
	const int flag = readInt("Kas panna lipp? (1/0) ", 0, 1);
	const int row = readInt("Vali rida (0-" + to_string(rowCount - 1) + "): ", 0, rowCount - 1);
	const int column = readInt("Vali veerg (0-" + to_string(columnCount - 1) + "): ", 0, columnCount - 1);
	makeMove(row, column, flag == 1);
}

char Minesweeper::cell(int row, int column) const {
	return board.at(static_cast<size_t>(row) * columnCount + column);
}

void Minesweeper::printBoard() const {
	// Column numbers on top and row numbers on the left make moves easy to type
	cout << "\n    ";
	for (int c = 0; c < columnCount; c++) {
		cout << (c % 10) << ' ';
	}
	cout << '\n';
	for (int r = 0; r < rowCount; r++) {
		cout << (r < 10 ? "  " : r < 100 ? " " : "") << r << ' ';
		for (int c = 0; c < columnCount; c++) {
			cout << cell(r, c) << ' ';
		}
		cout << '\n';
	}
	cout << '\n';
}
