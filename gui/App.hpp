#pragma once

#include "Canvas.hpp"
#include "GameScreen.hpp"
#include "MenuScreen.hpp"

// The window and switching between screens (menu <-> game). The window resizes to each screen
// and scales logical pixels by a whole number, so the pixel art stays sharp.
class App {
public:
	int run();

private:
	sf::RenderWindow window;
	Canvas canvas;
	MenuScreen menu;
	GameScreen game;
	Screen* current = &menu;

	void show(Screen& screen);
	sf::Vector2f toLogical(sf::Vector2i pixel) const;
};
