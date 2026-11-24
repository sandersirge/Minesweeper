#pragma once

#include "gui/screen.hpp"

#include <array>
#include <cstddef>
#include <optional>

// Board settings the menu hands to the game screen
struct Settings {
	int rows = 9;
	int columns = 9;
	int mines = 10;
};

// Start menu: difficulty levels, custom board size and mine count, START and QUIT.
class MenuScreen : public Screen {
public:
	static constexpr int MIN_SIDE = 5;
	static constexpr int MAX_ROWS = 24;
	static constexpr int MAX_COLUMNS = 40;

	MenuScreen();

	sf::Vector2u logicalSize() const override { return {WIDTH, HEIGHT}; }
	void handleEvent(const sf::Event& event, sf::Vector2f mouse) override;
	void update(float seconds) override;
	void draw(Canvas& canvas) const override;

	// The settings once the player pressed START (returned only once)
	std::optional<Settings> takeStartRequest();
	bool wantsQuit() const { return quit; }
	const Settings& settings() const { return current; }

private:
	static constexpr unsigned WIDTH = 176;
	static constexpr unsigned HEIGHT = 236;

	// Controls: 3 difficulty levels, 3 x (-, +), START, QUIT
	enum ButtonId : std::size_t {
		BEGINNER,
		INTERMEDIATE,
		EXPERT,
		ROWS_MINUS,
		ROWS_PLUS,
		COLUMNS_MINUS,
		COLUMNS_PLUS,
		MINES_MINUS,
		MINES_PLUS,
		START,
		QUIT,
		BUTTON_COUNT
	};

	Settings current;
	std::array<Button, BUTTON_COUNT> buttons;
	std::optional<std::size_t> hoveredButton;
	std::optional<std::size_t> pressedButton;
	float holdTime = 0.f; // holding +/- repeats the change
	float repeatTime = 0.f;
	bool startRequested = false;
	bool quit = false;

	void activate(std::size_t id);
	void step(std::size_t id);
	void selectDifficulty(std::size_t id);
	void clampMines();
	void updateSelection();
	std::optional<std::size_t> buttonAt(sf::Vector2f mouse) const;
	static bool isStepper(std::size_t id) { return id >= ROWS_MINUS && id <= MINES_PLUS; }
};
