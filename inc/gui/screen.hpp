#pragma once

#include "gui/canvas.hpp"

#include <string>
#include <utility>

// Common interface of the menu and game screens. Mouse positions are in logical pixels.
class Screen {
public:
	virtual ~Screen() = default;
	virtual sf::Vector2u logicalSize() const = 0;
	virtual void handleEvent(const sf::Event& event, sf::Vector2f mouse) = 0;
	virtual void update(float seconds) = 0;
	virtual void draw(Canvas& canvas) const = 0;
};

// A simple button: a rectangle with a label on the left and optional detail text on the right
struct Button {
	sf::FloatRect area;
	std::string label;
	std::string detail;    // e.g. the size of a difficulty level
	bool selected = false; // permanently highlighted (the chosen difficulty)

	Button() = default;
	Button(sf::FloatRect buttonArea, std::string buttonLabel, std::string buttonDetail = {})
	    : area(buttonArea), label(std::move(buttonLabel)), detail(std::move(buttonDetail)) {}

	bool contains(sf::Vector2f point) const { return area.contains(point); }
};

// Draws a button: hovered = the mouse is over it, pressed = the mouse button is down on it
void drawButton(Canvas& canvas, const Button& button, bool hovered, bool pressed);
