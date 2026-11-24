#pragma once

#include <SFML/Graphics.hpp>

// Collects every coloured rectangle of a frame into one vertex array and draws them with a single
// call. The whole interface (cells, text, icons) is made of such rectangles.
class Canvas {
public:
	void clear() { vertices.clear(); }
	void rect(float x, float y, float width, float height, sf::Color color);
	void pixel(float x, float y, sf::Color color, float scale = 1.f) { rect(x, y, scale, scale, color); }
	void draw(sf::RenderTarget& target) const { target.draw(vertices); }

private:
	sf::VertexArray vertices{sf::PrimitiveType::Triangles};
};
