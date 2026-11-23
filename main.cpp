#include "Minesweeper.h"

#include <iostream>
#include <optional>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

// Terminali mäng: laud prinditakse ja käigud küsitakse, kuni mäng saab läbi.
void mangiTerminalis(Minesweeper& ms) {
	while (!ms.kasMangOnLabi()) {
		ms.prindiMangulaud();
		ms.kaikTerminalis();
	}
	ms.prindiMangulaud();
}

// Graafiline mäng: aken on avatud ainult mängu ajal, nii et terminalis küsimise ajal ei jää
// vastamata aken ekraanile. Mängu lõpus jääb laud nähtavaks, kuni mängija klõpsab või vajutab
// klahvi. Tagastab false, kui mängija sulges akna enne mängu lõppu.
bool mangiGraafiliselt(Minesweeper& ms) {
	sf::RenderWindow aken(sf::VideoMode({ 640, 480 }), "Minesweeper");
	aken.setFramerateLimit(30);
	std::cout << "Mäng käib aknas: vasak klõps avab ruudu, parem klõps paneb või võtab lipu.\n";

	bool pealkiriMuudetud = false;
	while (aken.isOpen()) {
		while (const std::optional sundmus = aken.pollEvent()) {
			if (sundmus->is<sf::Event::Closed>()) {
				aken.close();
			}
			else if (const auto* muudetud = sundmus->getIf<sf::Event::Resized>()) {
				// Vaade akna suuruseks, et laud täidaks alati kogu akna
				aken.setView(sf::View(sf::FloatRect({ 0.f, 0.f },
					{ static_cast<float>(muudetud->size.x), static_cast<float>(muudetud->size.y) })));
			}
			else if (const auto* hiir = sundmus->getIf<sf::Event::MouseButtonPressed>()) {
				if (ms.kasMangOnLabi()) aken.close(); // lõpuekraanilt tagasi terminali
				else ms.handleEvent(aken, hiir->button, hiir->position);
			}
			else if (sundmus->is<sf::Event::KeyPressed>() && ms.kasMangOnLabi()) {
				aken.close();
			}
		}
		if (!aken.isOpen()) break;

		if (ms.kasMangOnLabi() && !pealkiriMuudetud) {
			const std::string pealkiri = ms.kasOliKaotus()
				? "Minesweeper - Kaotasid :( (klõpsa jätkamiseks)"
				: "Minesweeper - Võitsid! (klõpsa jätkamiseks)";
			aken.setTitle(sf::String::fromUtf8(pealkiri.begin(), pealkiri.end()));
			pealkiriMuudetud = true;
		}
		aken.clear();
		ms.kuvaMangulaud(aken);
		aken.display();
	}
	return ms.kasMangOnLabi();
}

} // namespace

int main() {
#ifdef _WIN32
	// Tekstid on UTF-8; ilma selleta näitaks Windowsi konsool täpitähti valesti
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	Minesweeper ms;
	try {
		ms.laeTekstuurid("Pildid");
	}
	catch (const std::string& fail) {
		std::cout << "Pildifailist " << fail << " sisselugemine ebaõnnestus!\n";
		return 1;
	}

	try {
		while (true) {
			ms.kusiManguLauaAndmed();

			if (ms.getKuvaGraafika()) {
				if (!mangiGraafiliselt(ms)) break; // aken suleti mängu ajal - lõpetame
			}
			else {
				mangiTerminalis(ms);
			}

			std::cout << (ms.kasOliKaotus() ? "Kaotasid :(\n" : "Võitsid!\n");
			if (loeTaisarv("Kas mängida uuesti? (1/0): ", 0, 1) == 0) break;
		}
	}
	catch (const SisendLopetatud&) {
		std::cout << "\nSisend lõppes, mäng suletakse.\n";
	}
	return 0;
}
