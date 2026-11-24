// Game logic tests. No window or images needed: setMines() puts mines on fixed cells.
// Run: ctest --test-dir build (or build/bin/minesweeper_test directly), or "make test".
#include "minesweeper.hpp"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const string& description) {
	checks++;
	if (!condition) {
		failures++;
		cerr << "  FAILED: " << description << '\n';
	}
}

// A board with mines at the given (row, column) cells
Minesweeper board(int rows, int columns, const vector<pair<int, int>>& mineCells) {
	Minesweeper game;
	game.newGame(rows, columns, 10, false);
	vector<bool> cells(static_cast<size_t>(rows) * columns, false);
	for (const auto& [r, c] : mineCells) {
		cells.at(static_cast<size_t>(r) * columns + c) = true;
	}
	game.setMines(cells);
	return game;
}

// Silences the game's messages (cout) during a test
struct Quiet {
	ostringstream buffer;
	streambuf* previous = cout.rdbuf(buffer.rdbuf());
	~Quiet() { cout.rdbuf(previous); }
};

void test(const string& name, const function<void()>& body) {
	const int before = failures;
	{
		Quiet quiet;
		body();
	}
	cout << (failures == before ? "[ OK ] " : "[FAIL] ") << name << '\n';
}

} // namespace

int main() {
	test("numbers do not wrap from the end of a row to the next row", [] {
		// 3x4 board, mine at the top right (0,3). The old code counted index 3 as a neighbour of (1,0).
		Minesweeper game = board(3, 4, {{0, 3}});
		game.makeMove(2, 0, false);
		check(game.cell(1, 0) == Minesweeper::EMPTY, "(1,0) is empty, not 1");
		check(game.cell(0, 2) == '1', "(0,2) shows 1");
		check(game.cell(1, 3) == '1', "(1,3) shows 1");
	});

	test("opening an empty area next to the last cell does not crash", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		check(game.makeMove(2, 2, false), "move allowed");
		check(game.cell(2, 2) == Minesweeper::EMPTY, "(2,2) is empty");
	});

	test("an empty area also opens diagonal numbers and the game is won", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		game.makeMove(2, 2, false);
		check(game.cell(1, 1) == '1', "(1,1) opened, 1");
		check(game.cell(0, 1) == '1' && game.cell(1, 0) == '1', "edge numbers opened");
		check(game.isOver() && !game.isLost(), "all free cells open = win");
		check(game.cell(0, 0) == Minesweeper::FLAG, "after a win the mine is flagged");
	});

	test("flags survive opening an empty area", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		game.makeMove(1, 1, true);
		game.makeMove(2, 2, false);
		check(game.cell(1, 1) == Minesweeper::FLAG, "flag kept");
		check(!game.isOver(), "a free cell is still under the flag, game goes on");
		game.makeMove(1, 1, true); // removes the flag
		game.makeMove(1, 1, false);
		check(game.isOver() && !game.isLost(), "opening the last cell wins");
	});

	test("an opened cell cannot be flagged or opened again", [] {
		Minesweeper game = board(2, 3, {{0, 0}});
		game.makeMove(1, 1, false);
		check(game.cell(1, 1) == '1', "(1,1) opened");
		check(!game.makeMove(1, 1, true), "flagging an opened cell is refused");
		check(game.cell(1, 1) == '1', "cell stays open");
		check(!game.makeMove(1, 1, false), "opening again is refused");
	});

	test("a flagged cell cannot be opened", [] {
		Minesweeper game = board(2, 2, {{0, 0}});
		game.makeMove(1, 1, true);
		check(!game.makeMove(1, 1, false), "opening refused");
		check(game.cell(1, 1) == Minesweeper::FLAG, "flag kept");
	});

	test("opening a mine loses and shows mines and wrong flags", [] {
		Minesweeper game = board(3, 3, {{0, 0}, {2, 2}});
		game.makeMove(0, 0, true);  // correct flag
		game.makeMove(0, 2, true);  // wrong flag
		game.makeMove(2, 2, false); // mine
		check(game.isOver() && game.isLost(), "lost");
		check(game.cell(2, 2) == Minesweeper::EXPLODED, "the opened mine is marked");
		check(game.cell(0, 0) == Minesweeper::FLAG, "correct flag stays");
		check(game.cell(0, 2) == Minesweeper::WRONG_FLAG, "wrong flag is marked");
		check(game.cell(1, 1) == Minesweeper::HIDDEN, "other cells stay closed");
		check(!game.makeMove(1, 0, false), "no moves after the game is over");
	});

	test("unflagged mines are shown after a loss", [] {
		Minesweeper game = board(2, 3, {{0, 0}, {1, 2}});
		game.makeMove(0, 0, false);
		check(game.cell(0, 0) == Minesweeper::EXPLODED, "exploded mine");
		check(game.cell(1, 2) == Minesweeper::MINE, "other mine shown");
	});

	test("a number opens its neighbours when it has the right number of flags", [] {
		// Mine at (0,0); (0,1) shows 1
		Minesweeper game = board(3, 3, {{0, 0}});
		game.makeMove(0, 1, false); // opens only the "1"
		check(game.cell(0, 1) == '1', "(0,1) opened");
		check(!game.chord(0, 1), "nothing opens without a flag");
		game.makeMove(0, 0, true);
		check(game.chord(0, 1), "with the flag the neighbours open");
		check(game.cell(1, 1) == '1' && game.cell(1, 2) == Minesweeper::EMPTY, "neighbours opened");
		check(game.isOver() && !game.isLost(), "all free cells open = win");
	});

	test("chording with a wrong flag loses", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		game.makeMove(0, 1, false);
		game.makeMove(0, 2, true); // wrong flag, the mine at (0,0) stays closed
		check(game.chord(0, 1), "neighbours opened");
		check(game.isLost(), "a mine was opened - lost");
		check(game.cell(0, 0) == Minesweeper::EXPLODED, "exploded mine");
		check(game.cell(0, 2) == Minesweeper::WRONG_FLAG, "wrong flag marked");
	});

	test("chording does nothing on hidden, flagged or outside cells", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		check(!game.chord(1, 1), "hidden cell");
		game.makeMove(1, 1, true);
		check(!game.chord(1, 1), "flagged cell");
		check(!game.chord(5, 5), "outside the board");
	});

	test("moves outside the board are refused", [] {
		Minesweeper game = board(2, 2, {{0, 0}});
		check(!game.makeMove(2, 0, false) && !game.makeMove(0, -1, true), "outside the board");
	});

	test("the first move is never a mine and opens an area when there is room", [] {
		for (int i = 0; i < 200; i++) {
			Minesweeper game;
			game.newGame(5, 5, 90, false); // 22 mines in 25 cells - no room to keep neighbours free
			game.makeMove(2, 2, false);
			check(!game.isLost(), "dense board: first move is safe");
		}
		for (int i = 0; i < 200; i++) {
			Minesweeper game;
			game.newGame(5, 5, 50, false); // 12 mines, 16 cells outside the neighbourhood
			game.makeMove(2, 2, false);
			check(game.cell(2, 2) == Minesweeper::EMPTY, "first cell is empty");
		}
	});

	test("the mine count matches the percentage", [] {
		Minesweeper game;
		game.newGame(10, 10, 30, false);
		game.makeMove(0, 0, false);
		check(game.mineCount() == 30, "30 mines");
	});

	test("exact mine count and its limit", [] {
		Minesweeper game;
		game.newGameWithMines(16, 30, 99);
		check(!game.hasStarted(), "mines are placed on the first opened cell");
		game.makeMove(8, 15, false);
		check(game.hasStarted(), "game started");
		check(game.mineCount() == 99, "99 mines");
		Minesweeper full;
		full.newGameWithMines(3, 3, 50);
		check(full.mineCount() == 8, "at least one cell stays free");
	});

	test("flag count", [] {
		Minesweeper game = board(3, 3, {{0, 0}});
		game.makeMove(0, 0, true);
		game.makeMove(2, 2, true);
		check(game.flagCount() == 2, "two flags");
		game.makeMove(2, 2, true);
		check(game.flagCount() == 1, "removing a flag");
	});

	test("a large board with many mines is ready quickly (the old rand() looped forever)", [] {
		Minesweeper game;
		game.newGame(Minesweeper::MAX_ROWS, Minesweeper::MAX_COLUMNS, 60, false);
		game.makeMove(0, 0, false);
		check(game.mineCount() == 307200u * 60 / 100, "184320 mines");
	});

	test("readInt asks again and stops at the end of input", [] {
		istringstream input("abc\n0\n42\n7\n");
		streambuf* previous = cin.rdbuf(input.rdbuf());
		const int value = readInt("? ", 1, 9);
		check(value == 7, "letters and out-of-range numbers are skipped");
		bool thrown = false;
		try {
			readInt("? ", 1, 9);
		} catch (const InputEnded&) {
			thrown = true;
		}
		check(thrown, "InputEnded is thrown at the end of input");
		cin.rdbuf(previous);
		cin.clear();
	});

	cout << '\n' << (checks - failures) << "/" << checks << " checks passed\n";
	return failures == 0 ? 0 : 1;
}
