#pragma once

#include "Canvas.hpp"

// 16x16 pixel-art icons (flag, mine, face), drawn for this project.
namespace icons {

inline constexpr int SIZE = 16;

enum class Face { Normal, Pressing, Lost, Won };

void flag(Canvas& canvas, float x, float y, float scale = 1.f);
void mine(Canvas& canvas, float x, float y, float scale = 1.f);
void cross(Canvas& canvas, float x, float y, float scale = 1.f); // marks a wrong flag
void face(Canvas& canvas, Face face, float x, float y, float scale = 1.f);
// The face as an image for the window icon, scaled by a whole number
sf::Image faceImage(Face face, unsigned scale);

} // namespace icons
