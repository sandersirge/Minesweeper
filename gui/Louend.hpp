#pragma once
#include <SFML/Graphics.hpp>

// Kogub ühe kaadri jooksul kõik värvilised ristkülikud ühte tipumassiivi ja joonistab need ühe
// kutsega. Kogu liides (ruudud, tekst, ikoonid) koosneb sellistest ristkülikutest.
class Louend {
public:
	void puhasta() { tipud.clear(); }
	void ristkylik(float x, float y, float laius, float korgus, sf::Color varv);
	void piksel(float x, float y, sf::Color varv, float suurendus = 1.f) {
		ristkylik(x, y, suurendus, suurendus, varv);
	}
	void joonista(sf::RenderTarget& siht) const { siht.draw(tipud); }

private:
	sf::VertexArray tipud{ sf::PrimitiveType::Triangles };
};
