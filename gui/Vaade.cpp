#include "Vaade.hpp"
#include "Kiri.hpp"
#include "Teema.hpp"

#include <cmath>

void joonistaNupp(Louend& louend, const Nupp& nupp, bool hover, bool vajutatud) {
	const float x = nupp.ala.position.x;
	const float y = nupp.ala.position.y;
	const float w = nupp.ala.size.x;
	const float h = nupp.ala.size.y;

	sf::Color taust = nupp.valitud ? teema::NUPP_VALITUD : teema::NUPP;
	if (vajutatud) taust = teema::NUPP_VAJUTATUD;
	else if (hover) taust = nupp.valitud ? teema::HOVER : teema::NUPP_HOVER;

	// Tume serv, sisu ja 1-piksline helge ülaserv (vajutatult nihkub sisu alla)
	louend.ristkylik(x, y, w, h, teema::NUPP_SERV);
	louend.ristkylik(x + 1, y + 1, w - 2, h - 2, taust);
	if (!vajutatud) louend.ristkylik(x + 1, y + 1, w - 2, 1, teema::SULETUD_HELE);

	const float nihe = vajutatud ? 1.f : 0.f;
	const float tekstY = std::floor(y + (h - kiri::TAHE_KORGUS) / 2.f) + nihe;
	if (nupp.parem.empty()) {
		kiri::kirjutaKeskele(louend, nupp.tekst, x + w / 2.f, tekstY, teema::TEKST);
	}
	else {
		kiri::kirjuta(louend, nupp.tekst, x + 5, tekstY, teema::TEKST);
		const float paremLaius = kiri::laius(nupp.parem);
		kiri::kirjuta(louend, nupp.parem, x + w - 5 - paremLaius, tekstY,
		              nupp.valitud ? teema::TEKST : teema::TEKST_TUHM);
	}
}
