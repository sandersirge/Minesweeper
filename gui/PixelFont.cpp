#include "PixelFont.hpp"

#include <array>
#include <cmath>
#include <map>

using namespace std;

namespace pixelfont {
namespace {

using Glyph = array<const char*, GLYPH_HEIGHT>;

// Letter shapes: '#' is a filled pixel. All drawn for this project.
const map<char32_t, Glyph>& glyphs() {
	static const map<char32_t, Glyph> table = {
	    {U'A', {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
	    {U'B', {"####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."}},
	    {U'C', {".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."}},
	    {U'D', {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
	    {U'E', {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
	    {U'F', {"#####", "#....", "#....", "####.", "#....", "#....", "#...."}},
	    {U'G', {".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".###."}},
	    {U'H', {"#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
	    {U'I', {".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."}},
	    {U'J', {"..###", "...#.", "...#.", "...#.", "#..#.", "#..#.", ".##.."}},
	    {U'K', {"#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"}},
	    {U'L', {"#....", "#....", "#....", "#....", "#....", "#....", "#####"}},
	    {U'M', {"#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"}},
	    {U'N', {"#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"}},
	    {U'O', {".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
	    {U'P', {"####.", "#...#", "#...#", "####.", "#....", "#....", "#...."}},
	    {U'Q', {".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"}},
	    {U'R', {"####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"}},
	    {U'S', {".####", "#....", "#....", ".###.", "....#", "....#", "####."}},
	    {U'T', {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."}},
	    {U'U', {"#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
	    {U'V', {"#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
	    {U'W', {"#...#", "#...#", "#...#", "#.#.#", "#.#.#", "#.#.#", ".#.#."}},
	    {U'X', {"#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"}},
	    {U'Y', {"#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."}},
	    {U'Z', {"#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"}},
	    {U'0', {".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."}},
	    {U'1', {"..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."}},
	    {U'2', {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
	    {U'3', {"####.", "....#", "....#", ".###.", "....#", "....#", "####."}},
	    {U'4', {"...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."}},
	    {U'5', {"#####", "#....", "####.", "....#", "....#", "#...#", ".###."}},
	    {U'6', {".###.", "#....", "#....", "####.", "#...#", "#...#", ".###."}},
	    {U'7', {"#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."}},
	    {U'8', {".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."}},
	    {U'9', {".###.", "#...#", "#...#", ".####", "....#", "....#", ".###."}},
	    {U' ', {".....", ".....", ".....", ".....", ".....", ".....", "....."}},
	    {U':', {".....", "..#..", "..#..", ".....", "..#..", "..#..", "....."}},
	    {U'!', {"..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."}},
	    {U'?', {".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."}},
	    {U'-', {".....", ".....", ".....", "#####", ".....", ".....", "....."}},
	    {U'+', {".....", "..#..", "..#..", "#####", "..#..", "..#..", "....."}},
	    {U'.', {".....", ".....", ".....", ".....", ".....", ".....", "..#.."}},
	    {U',', {".....", ".....", ".....", ".....", ".....", "..#..", ".#..."}},
	    {U'/', {"....#", "....#", "...#.", "..#..", ".#...", "#....", "#...."}},
	    {U'(', {"...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#."}},
	    {U')', {".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..."}},
	    {U'<', {"...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#."}},
	    {U'>', {".#...", "..#..", "...#.", "....#", "...#.", "..#..", ".#..."}},
	    {U'×', {".....", ".....", "#...#", ".#.#.", "..#..", ".#.#.", "#...#"}}, // ×
	    {U'·', {".....", ".....", ".....", "..#..", ".....", ".....", "....."}}, // ·
	};
	return table;
}

enum class Accent { None, Umlaut, Tilde };

struct Letter {
	const Glyph* glyph = nullptr;
	Accent accent = Accent::None;
};

Letter findLetter(char32_t c) {
	Accent accent = Accent::None;
	switch (c) {
	case U'x': // "9x9" -> 9×9
		c = U'×';
		break;
	case U'ä': // ä
	case U'Ä': // Ä
		c = U'A';
		accent = Accent::Umlaut;
		break;
	case U'ö': // ö
	case U'Ö': // Ö
		c = U'O';
		accent = Accent::Umlaut;
		break;
	case U'ü': // ü
	case U'Ü': // Ü
		c = U'U';
		accent = Accent::Umlaut;
		break;
	case U'õ': // õ
	case U'Õ': // Õ
		c = U'O';
		accent = Accent::Tilde;
		break;
	default:
		if (c >= U'a' && c <= U'z') {
			c = c - U'a' + U'A';
		}
	}
	const auto& table = glyphs();
	auto found = table.find(c);
	if (found == table.end()) {
		found = table.find(U'?');
	}
	return {&found->second, accent};
}

// Reads the next character of UTF-8 text and moves past it
char32_t nextChar(string_view text, size_t& i) {
	const auto first = static_cast<unsigned char>(text[i]);
	int length = 1;
	char32_t c = first;
	if (first >= 0xF0) {
		length = 4;
		c = first & 0x07;
	} else if (first >= 0xE0) {
		length = 3;
		c = first & 0x0F;
	} else if (first >= 0xC0) {
		length = 2;
		c = first & 0x1F;
	}
	for (int k = 1; k < length && i + k < text.size(); k++) {
		c = (c << 6) | (static_cast<unsigned char>(text[i + k]) & 0x3F);
	}
	i += length;
	return c;
}

size_t charCount(string_view text) {
	size_t total = 0;
	for (size_t i = 0; i < text.size();) {
		nextChar(text, i);
		total++;
	}
	return total;
}

} // namespace

float width(string_view text, float scale) {
	const size_t total = charCount(text);
	if (total == 0) {
		return 0.f;
	}
	return (static_cast<float>(total) * (GLYPH_WIDTH + SPACING) - SPACING) * scale;
}

void draw(Canvas& canvas, string_view text, float x, float y, sf::Color color, float s) {
	for (size_t i = 0; i < text.size();) {
		const Letter letter = findLetter(nextChar(text, i));
		for (int row = 0; row < GLYPH_HEIGHT; row++) {
			for (int column = 0; column < GLYPH_WIDTH; column++) {
				if ((*letter.glyph)[row][column] == '#') {
					canvas.pixel(x + column * s, y + row * s, color, s);
				}
			}
		}
		if (letter.accent == Accent::Umlaut) {
			canvas.pixel(x + 1 * s, y - 2 * s, color, s);
			canvas.pixel(x + 3 * s, y - 2 * s, color, s);
		} else if (letter.accent == Accent::Tilde) {
			canvas.pixel(x + 1 * s, y - 3 * s, color, s);
			canvas.pixel(x + 4 * s, y - 3 * s, color, s);
			canvas.pixel(x + 0 * s, y - 2 * s, color, s);
			canvas.pixel(x + 2 * s, y - 2 * s, color, s);
			canvas.pixel(x + 3 * s, y - 2 * s, color, s);
		}
		x += (GLYPH_WIDTH + SPACING) * s;
	}
}

void drawCentered(Canvas& canvas, string_view text, float centerX, float y, sf::Color color, float s) {
	// A whole-pixel position so the letters do not land on half pixels (blurry text)
	draw(canvas, text, floor(centerX - width(text, s) / 2.f), y, color, s);
}

void drawShadowed(Canvas& canvas, string_view text, float x, float y, sf::Color color, sf::Color shadow,
                  float s) {
	draw(canvas, text, x + s, y + s, shadow, s);
	draw(canvas, text, x, y, color, s);
}

} // namespace pixelfont
