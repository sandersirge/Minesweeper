#pragma once

#include "gui/canvas.hpp"

#include <string_view>

// A small 5x7 pixel font (capital letters, digits, Õ Ä Ö Ü and some symbols). Text is UTF-8.
// Umlauts and the tilde take up to 3 pixels above a letter.
namespace pixelfont {

inline constexpr int GLYPH_WIDTH = 5;
inline constexpr int GLYPH_HEIGHT = 7;
inline constexpr int SPACING = 1;

// Width of the text in pixels at the given scale
float width(std::string_view text, float scale = 1.f);
// (x, y) is the top left corner of the first letter
void draw(Canvas& canvas, std::string_view text, float x, float y, sf::Color color, float scale = 1.f);
void drawCentered(Canvas& canvas, std::string_view text, float centerX, float y, sf::Color color,
                  float scale = 1.f);
// Text with a one-pixel shadow (down and right) so it stands out on any background
void drawShadowed(Canvas& canvas, std::string_view text, float x, float y, sf::Color color, sf::Color shadow,
                  float scale = 1.f);

} // namespace pixelfont
