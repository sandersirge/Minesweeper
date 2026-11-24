// The archived classic version: the board settings are asked in the terminal and the game is
// played either in the terminal or in a simple window using the images in pildid/. The new
// graphical interface is in src/gui. Built with CMake as the target rungame_classic.
#include "minesweeper.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace std;

namespace {

// The cell images; loaded once at the start of the program
struct Textures {
	sf::Texture mine, hidden, empty, flag;
	sf::Texture numbers[8]; // 1...8
	map<char, const sf::Texture*> byCell;

	// Throws the file name if an image cannot be loaded
	void load(const string& folder) {
		const filesystem::path images{folder};
		const auto loadOne = [&](sf::Texture& texture, const char* name) {
			const string file = (images / name).string();
			if (!texture.loadFromFile(file)) {
				throw file;
			}
		};
		loadOne(flag, "lipp.png");
		loadOne(mine, "pomm.png");
		loadOne(empty, "tuhi.png");
		loadOne(hidden, "avamata.png");
		const char* numberImages[8] = {"yks.png",  "kaks.png", "kolm.png",   "neli.png",
		                               "viis.png", "kuus.png", "seitse.png", "kaheksa.png"};
		for (int i = 0; i < 8; i++) {
			loadOne(numbers[i], numberImages[i]);
		}

		byCell[Minesweeper::HIDDEN] = &hidden;
		byCell[Minesweeper::EMPTY] = &empty;
		byCell[Minesweeper::FLAG] = &flag;
		byCell[Minesweeper::MINE] = &mine;
		byCell[Minesweeper::EXPLODED] = &mine;
		byCell[Minesweeper::WRONG_FLAG] = &flag;
		for (int i = 0; i < 8; i++) {
			byCell[static_cast<char>('1' + i)] = &numbers[i];
		}
	}
};

// The window can be resized, so the cell size follows the view and the whole board is visible
void drawBoard(const Minesweeper& game, const Textures& textures, sf::RenderWindow& window) {
	const sf::Vector2f viewSize = window.getView().getSize();
	const sf::Vector2f cellSize{viewSize.x / game.columns(), viewSize.y / game.rows()};
	sf::RectangleShape shape(cellSize);
	for (int r = 0; r < game.rows(); r++) {
		for (int c = 0; c < game.columns(); c++) {
			const auto texture = textures.byCell.find(game.cell(r, c));
			if (texture == textures.byCell.end()) {
				continue;
			}
			shape.setTexture(texture->second, true);
			shape.setPosition({c * cellSize.x, r * cellSize.y});
			window.draw(shape);
		}
	}
}

// Mouse click: left opens, right places a flag. The position is in window pixels.
void click(Minesweeper& game, const sf::RenderWindow& window, sf::Mouse::Button button,
           sf::Vector2i position) {
	if (button != sf::Mouse::Button::Left && button != sf::Mouse::Button::Right) {
		return;
	}
	const sf::Vector2f point = window.mapPixelToCoords(position);
	const sf::Vector2f viewSize = window.getView().getSize();
	if (point.x < 0 || point.y < 0 || viewSize.x <= 0 || viewSize.y <= 0) {
		return;
	}
	const int column = static_cast<int>(point.x * game.columns() / viewSize.x);
	const int row = static_cast<int>(point.y * game.rows() / viewSize.y);
	if (row >= game.rows() || column >= game.columns()) {
		return;
	}
	game.makeMove(row, column, button == sf::Mouse::Button::Right);
}

// Terminal game: the board is printed and moves are asked until the game is over
void playInTerminal(Minesweeper& game) {
	while (!game.isOver()) {
		game.printBoard();
		game.moveFromTerminal();
	}
	game.printBoard();
}

// Window game: the window is open only during the game. At the end the board stays visible until
// the player clicks or presses a key. Returns false if the window was closed before the end.
bool playInWindow(Minesweeper& game, const Textures& textures) {
	sf::RenderWindow window(sf::VideoMode({640, 480}), "Minesweeper");
	window.setFramerateLimit(30);
	cout << "Mäng käib aknas: vasak klõps avab ruudu, parem klõps paneb või võtab lipu.\n";

	bool titleChanged = false;
	while (window.isOpen()) {
		while (const optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			} else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				window.setView(sf::View(sf::FloatRect(
				    {0.f, 0.f}, {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)})));
			} else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (game.isOver()) {
					window.close();
				} else {
					click(game, window, mouse->button, mouse->position);
				}
			} else if (event->is<sf::Event::KeyPressed>() && game.isOver()) {
				window.close();
			}
		}
		if (!window.isOpen()) {
			break;
		}

		if (game.isOver() && !titleChanged) {
			const string title = game.isLost() ? "Minesweeper - Kaotasid :( (klõpsa jätkamiseks)"
			                                   : "Minesweeper - Võitsid! (klõpsa jätkamiseks)";
			window.setTitle(sf::String::fromUtf8(title.begin(), title.end()));
			titleChanged = true;
		}
		window.clear();
		drawBoard(game, textures, window);
		window.display();
	}
	return game.isOver();
}

} // namespace

int main() {
#ifdef _WIN32
	// The texts are UTF-8; without this the Windows console shows Estonian letters wrongly
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	Textures textures;
	try {
		textures.load("pildid");
	} catch (const string& file) {
		cout << "Pildifailist " << file << " sisselugemine ebaõnnestus!\n";
		return 1;
	}

	Minesweeper game;
	try {
		while (true) {
			game.askBoardSettings();
			if (game.wantsGraphics()) {
				if (!playInWindow(game, textures)) {
					break; // the window was closed during the game
				}
			} else {
				playInTerminal(game);
			}
			cout << (game.isLost() ? "Kaotasid :(\n" : "Võitsid!\n");
			if (readInt("Kas mängida uuesti? (1/0): ", 0, 1) == 0) {
				break;
			}
		}
	} catch (const InputEnded&) {
		cout << "\nSisend lõppes, mäng suletakse.\n";
	}
	return 0;
}
