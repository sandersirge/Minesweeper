#include "Louend.hpp"

void Louend::ristkylik(float x, float y, float laius, float korgus, sf::Color varv) {
	if (laius <= 0.f || korgus <= 0.f || varv.a == 0) return;
	const sf::Vector2f a{ x, y };
	const sf::Vector2f b{ x + laius, y };
	const sf::Vector2f c{ x + laius, y + korgus };
	const sf::Vector2f d{ x, y + korgus };
	tipud.append({ a, varv });
	tipud.append({ b, varv });
	tipud.append({ c, varv });
	tipud.append({ a, varv });
	tipud.append({ c, varv });
	tipud.append({ d, varv });
}
