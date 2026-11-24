#include "Screen.hpp"

#include "PixelFont.hpp"
#include "Theme.hpp"

#include <cmath>

using namespace std;

void drawButton(Canvas& canvas, const Button& button, bool hovered, bool pressed) {
	const float x = button.area.position.x;
	const float y = button.area.position.y;
	const float w = button.area.size.x;
	const float h = button.area.size.y;

	sf::Color fill = button.selected ? theme::BUTTON_SELECTED : theme::BUTTON;
	if (pressed) {
		fill = theme::BUTTON_PRESSED;
	} else if (hovered) {
		fill = button.selected ? theme::HOVER : theme::BUTTON_HOVER;
	}

	// Dark border, the fill and a 1-pixel highlight on top (when pressed the content moves down)
	canvas.rect(x, y, w, h, theme::BUTTON_BORDER);
	canvas.rect(x + 1, y + 1, w - 2, h - 2, fill);
	if (!pressed) {
		canvas.rect(x + 1, y + 1, w - 2, 1, theme::CLOSED_LIGHT);
	}

	const float offset = pressed ? 1.f : 0.f;
	const float textY = floor(y + (h - pixelfont::GLYPH_HEIGHT) / 2.f) + offset;
	if (button.detail.empty()) {
		pixelfont::drawCentered(canvas, button.label, x + w / 2.f, textY, theme::TEXT);
		return;
	}
	pixelfont::draw(canvas, button.label, x + 5, textY, theme::TEXT);
	const float detailWidth = pixelfont::width(button.detail);
	pixelfont::draw(canvas, button.detail, x + w - 5 - detailWidth, textY,
	                button.selected ? theme::TEXT : theme::TEXT_DIM);
}
