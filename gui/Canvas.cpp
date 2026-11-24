#include "Canvas.hpp"

using namespace std;

void Canvas::rect(float x, float y, float width, float height, sf::Color color) {
	if (width <= 0.f || height <= 0.f || color.a == 0) {
		return;
	}
	const sf::Vector2f topLeft{x, y};
	const sf::Vector2f topRight{x + width, y};
	const sf::Vector2f bottomRight{x + width, y + height};
	const sf::Vector2f bottomLeft{x, y + height};
	vertices.append({topLeft, color});
	vertices.append({topRight, color});
	vertices.append({bottomRight, color});
	vertices.append({topLeft, color});
	vertices.append({bottomRight, color});
	vertices.append({bottomLeft, color});
}
