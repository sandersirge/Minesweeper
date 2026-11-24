#pragma once

#include "Icons.hpp"
#include "MenuScreen.hpp"
#include "Minesweeper.h"
#include "Screen.hpp"

#include <optional>
#include <random>
#include <vector>

// The game screen: a bar on top (mines left, face = new game, time) and the board.
//   left click    opens a cell; on an opened number opens its neighbours if enough flags are placed
//   right click   places or removes a flag
//   middle click  opens the neighbours of a number
//   Enter / F2    new game with the same settings, Esc back to the menu
class GameScreen : public Screen {
public:
	void start(const Settings& newSettings);

	sf::Vector2u logicalSize() const override;
	void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
	void update(float seconds) override;
	void draw(Canvas& canvas) const override;

	bool wantsMenu() const { return backToMenu; }
	const Minesweeper& game() const { return minesweeper; }

private:
	struct CellPosition {
		int row;
		int column;
		bool operator==(const CellPosition& other) const {
			return row == other.row && column == other.column;
		}
	};

	Settings settings;
	Minesweeper minesweeper;
	float boardX = 0.f;
	float boardY = 0.f;

	std::optional<CellPosition> hoveredCell;
	std::optional<CellPosition> pressedCell; // the cell the left button went down on
	bool leftDown = false;
	bool faceHovered = false;
	bool facePressed = false;
	bool backToMenu = false;

	float elapsed = 0.f;  // game time in seconds, runs from the first opened cell to the end
	float sinceEnd = 0.f; // time since the game ended (for the wave animation)
	bool waveActive = false;
	std::vector<float> waveDelays; // when the wave reaches each cell (seconds after the end)
	float waveLength = 0.f;
	std::mt19937 rng{std::random_device{}()};

	std::optional<CellPosition> cellAt(sf::Vector2f mouse) const;
	sf::FloatRect faceArea() const;
	void play(const CellPosition& cell, bool flag, bool chord);
	void startWave(const CellPosition& origin);
	icons::Face faceState() const;
	void drawBar(Canvas& canvas) const;
	void drawCell(Canvas& canvas, int row, int column) const;
	void drawWave(Canvas& canvas) const;
	void drawBanner(Canvas& canvas) const;
};
