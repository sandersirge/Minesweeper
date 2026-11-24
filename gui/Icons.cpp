#include "Icons.hpp"

#include "Theme.hpp"

#include <array>
#include <string>

using namespace std;

namespace icons {
namespace {

using Bitmap = array<string, SIZE>;

// Colours for the letters in the bitmaps ('.' is transparent)
sf::Color paletteColor(char c) {
	switch (c) {
	case 'K': // mine body, face outline, flag pole
		return {20, 20, 28};
	case 'H': // highlight on the mine and the sunglasses
		return {236, 236, 246};
	case 'R': // flag
		return {232, 36, 36};
	case 'r': // flag shadow
		return {156, 16, 22};
	case 'Y': // face
		return {255, 214, 48};
	case 'y': // face shadow
		return {220, 162, 20};
	case 'W': // face highlight
		return {255, 246, 196};
	case 'B': // eyes, mouth
		return {24, 18, 18};
	case 'G': // sunglasses
		return {40, 40, 64};
	default:
		return sf::Color::Transparent;
	}
}

// clang-format off
const Bitmap FLAG = {
	"................",
	"................",
	"......RRR#......",
	"....RRRRR#......",
	"..RRRRRRR#......",
	"...rrRRRR#......",
	"......rrr#......",
	".........#......",
	".........#......",
	".........#......",
	".........#......",
	"......#####.....",
	".....#######....",
	"................",
	"................",
	"................",
};
// clang-format on

// clang-format off
const Bitmap MINE = {
	"................",
	".......##.......",
	".......##.......",
	"...#..####..#...",
	"....########....",
	"...##HH######...",
	"...##HH######...",
	".##############.",
	".##############.",
	"...##########...",
	"...##########...",
	"....########....",
	"...#..####..#...",
	".......##.......",
	".......##.......",
	"................",
};
// clang-format on

// The base of the face; eyes and mouth are added per state
// clang-format off
const Bitmap FACE = {
	".....KKKKKK.....",
	"...KKYYYYYYKK...",
	"..KYYWWYYYYYYK..",
	".KYYWWYYYYYYYyK.",
	".KYYYYYYYYYYYyK.",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	".KYYYYYYYYYYYyyK",
	".KyYYYYYYYYYyyK.",
	"..KyyYYYYYYyyK..",
	"...KKyyyyyyKK...",
	".....KKKKKK.....",
};
// clang-format on

// '#' in a bitmap gets the given palette letter, other letters use their own colour
void drawBitmap(Canvas& canvas, const Bitmap& bitmap, float x, float y, float scale, char hashColor) {
	for (int row = 0; row < SIZE; row++) {
		for (int column = 0; column < SIZE; column++) {
			const char c = bitmap[row][column] == '#' ? hashColor : bitmap[row][column];
			const sf::Color color = paletteColor(c);
			if (color.a != 0) {
				canvas.pixel(x + column * scale, y + row * scale, color, scale);
			}
		}
	}
}

Bitmap faceBitmap(Face state) {
	Bitmap bitmap = FACE;
	const auto set = [&](int column, int row, char c = 'B') { bitmap[row][column] = c; };
	switch (state) {
	case Face::Normal: // smile
		set(5, 5);
		set(5, 6);
		set(10, 5);
		set(10, 6);
		set(4, 10);
		set(11, 10);
		for (int c = 5; c <= 10; c++) {
			set(c, 11);
		}
		break;
	case Face::Pressing: // surprised "O"
		set(5, 5);
		set(5, 6);
		set(10, 5);
		set(10, 6);
		set(7, 9);
		set(8, 9);
		set(6, 10);
		set(9, 10);
		set(6, 11);
		set(9, 11);
		set(7, 12);
		set(8, 12);
		break;
	case Face::Lost: // X eyes and a sad mouth
		for (int d = 0; d < 3; d++) {
			set(4 + d, 4 + d);
			set(6 - d, 4 + d);
			set(9 + d, 4 + d);
			set(11 - d, 4 + d);
		}
		set(4, 12);
		set(11, 12);
		for (int c = 5; c <= 10; c++) {
			set(c, 11);
		}
		break;
	case Face::Won: // sunglasses and a wide smile
		for (int c = 3; c <= 12; c++) {
			set(c, 5, 'G');
		}
		for (int c = 3; c <= 6; c++) {
			set(c, 6, 'G');
		}
		for (int c = 9; c <= 12; c++) {
			set(c, 6, 'G');
		}
		set(4, 7, 'G');
		set(5, 7, 'G');
		set(10, 7, 'G');
		set(11, 7, 'G');
		set(4, 6, 'H');
		set(9, 6, 'H');
		set(3, 10);
		set(12, 10);
		for (int c = 4; c <= 11; c++) {
			set(c, 11);
		}
		for (int c = 6; c <= 9; c++) {
			set(c, 12);
		}
		break;
	}
	return bitmap;
}

} // namespace

void flag(Canvas& canvas, float x, float y, float scale) {
	drawBitmap(canvas, FLAG, x, y, scale, 'K');
}

void mine(Canvas& canvas, float x, float y, float scale) {
	drawBitmap(canvas, MINE, x, y, scale, 'K');
}

void cross(Canvas& canvas, float x, float y, float scale) {
	for (int i = 2; i <= 13; i++) {
		canvas.rect(x + i * scale, y + i * scale, 2 * scale, scale, theme::CROSS);
		canvas.rect(x + (14 - i) * scale, y + i * scale, 2 * scale, scale, theme::CROSS);
	}
}

void face(Canvas& canvas, Face state, float x, float y, float scale) {
	drawBitmap(canvas, faceBitmap(state), x, y, scale, 'K');
}

sf::Image faceImage(Face state, unsigned scale) {
	const Bitmap bitmap = faceBitmap(state);
	sf::Image image({SIZE * scale, SIZE * scale}, sf::Color::Transparent);
	for (unsigned row = 0; row < SIZE * scale; row++) {
		for (unsigned column = 0; column < SIZE * scale; column++) {
			image.setPixel({column, row}, paletteColor(bitmap[row / scale][column / scale]));
		}
	}
	return image;
}

} // namespace icons
