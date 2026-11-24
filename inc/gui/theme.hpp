#pragma once

#include <SFML/Graphics/Color.hpp>

// Sizes and colours of the GUI. Everything is in "logical pixels": the window scales them by a
// whole number, so the pixel art stays sharp.
namespace theme {

inline constexpr int CELL = 16;       // size of one board cell
inline constexpr int BAR = 24;        // height of the bar above the board
inline constexpr int MIN_WIDTH = 160; // minimum width of the game screen (so the bar fits)
inline constexpr int MAX_SCALE = 4;

inline constexpr sf::Color BACKGROUND{8, 16, 48}; // lines between cells, window background
inline constexpr sf::Color BAR_BACKGROUND{14, 28, 82};
inline constexpr sf::Color CLOSED{16, 80, 240};
inline constexpr sf::Color CLOSED_LIGHT{70, 132, 255}; // top/left edge of a closed cell
inline constexpr sf::Color CLOSED_DARK{8, 46, 172};    // bottom/right edge of a closed cell
inline constexpr sf::Color HOVER{48, 114, 255};
inline constexpr sf::Color PRESSED{10, 54, 200};
inline constexpr sf::Color OPENED{150, 186, 255};
inline constexpr sf::Color EXPLOSION{228, 46, 32};
inline constexpr sf::Color CROSS{228, 30, 30}; // cross over a wrong flag

inline constexpr sf::Color TEXT{236, 242, 255};
inline constexpr sf::Color TEXT_DIM{132, 160, 222};
inline constexpr sf::Color SHADOW{4, 10, 32};

inline constexpr sf::Color BUTTON{26, 62, 172};
inline constexpr sf::Color BUTTON_HOVER{44, 94, 222};
inline constexpr sf::Color BUTTON_PRESSED{12, 40, 132};
inline constexpr sf::Color BUTTON_SELECTED{72, 132, 255};
inline constexpr sf::Color BUTTON_BORDER{4, 14, 56};

inline constexpr sf::Color WAVE_WIN{255, 255, 255};
inline constexpr sf::Color WAVE_LOSS{255, 52, 24};
inline constexpr sf::Color BANNER{0, 0, 0, 176};

// Colours of the numbers 1...8 (index 0 is unused)
inline constexpr sf::Color NUMBERS[9] = {
    {0, 0, 0},    {18, 44, 168}, {14, 118, 40}, {196, 24, 24}, {70, 10, 140},
    {132, 36, 0}, {0, 112, 124}, {24, 24, 24},  {92, 92, 104},
};

} // namespace theme
