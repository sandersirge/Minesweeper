// Arhiveeritud klassikaline versioon: mängulaua andmed küsitakse terminalis ja mängitakse kas
// terminalis või lihtsas aknas Pildid/ kausta piltidega. Uus graafiline liides on gui/ kaustas.
// Ehitatakse CMake'iga sihtmärgina rungame_klassikaline.
#include "Minesweeper.h"

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>

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

// Ruutude pildid; laaditakse kord programmi alguses
struct Tekstuurid {
	sf::Texture pomm, avamata, tuhi, lipp;
	sf::Texture numbrid[8]; // 1...8
	std::map<char, const sf::Texture*> charTekstuuriks;

	// Kui mõni pilt ei laadi, visatakse selle faili nimi
	void lae(const std::string& kaust) {
		const std::filesystem::path pildid{ kaust };
		const auto lae = [&](sf::Texture& tekstuur, const char* nimi) {
			const std::string fail = (pildid / nimi).string();
			if (!tekstuur.loadFromFile(fail)) throw fail;
		};
		lae(lipp, "lipp.png");
		lae(pomm, "pomm.png");
		lae(tuhi, "tuhi.png");
		lae(avamata, "avamata.png");
		const char* numbriPildid[8] = { "yks.png", "kaks.png", "kolm.png", "neli.png",
			"viis.png", "kuus.png", "seitse.png", "kaheksa.png" };
		for (int i = 0; i < 8; i++) lae(numbrid[i], numbriPildid[i]);

		charTekstuuriks[Minesweeper::AVAMATA] = &avamata;
		charTekstuuriks[Minesweeper::TUHI] = &tuhi;
		charTekstuuriks[Minesweeper::LIPP] = &lipp;
		charTekstuuriks[Minesweeper::POMM] = &pomm;
		charTekstuuriks[Minesweeper::PLAHVATANUD] = &pomm;
		charTekstuuriks[Minesweeper::VALE_LIPP] = &lipp;
		for (int i = 0; i < 8; i++) charTekstuuriks[static_cast<char>('1' + i)] = &numbrid[i];
	}
};

// Akna suurust saab muuta, seega arvutatakse ruudu suurus vaate järgi, et kogu laud oleks näha
void kuvaMangulaud(const Minesweeper& ms, const Tekstuurid& tekstuurid, sf::RenderWindow& aken) {
	const sf::Vector2f suurus = aken.getView().getSize();
	const sf::Vector2f ruuduSuurus{ suurus.x / ms.getLaius(), suurus.y / ms.getKorgus() };
	sf::RectangleShape kujund(ruuduSuurus);
	for (int r = 0; r < ms.getKorgus(); r++) {
		for (int v = 0; v < ms.getLaius(); v++) {
			const auto tekstuur = tekstuurid.charTekstuuriks.find(ms.ruut(r, v));
			if (tekstuur == tekstuurid.charTekstuuriks.end()) continue;
			kujund.setTexture(tekstuur->second, true);
			kujund.setPosition({ v * ruuduSuurus.x, r * ruuduSuurus.y });
			aken.draw(kujund);
		}
	}
}

// Hiireklõps: vasak nupp avab, parem paneb lipu. Asukoht on akna pikslites.
void klops(Minesweeper& ms, const sf::RenderWindow& aken, sf::Mouse::Button nupp, sf::Vector2i asukoht) {
	if (nupp != sf::Mouse::Button::Left && nupp != sf::Mouse::Button::Right) return;
	const sf::Vector2f punkt = aken.mapPixelToCoords(asukoht);
	const sf::Vector2f suurus = aken.getView().getSize();
	if (punkt.x < 0 || punkt.y < 0 || suurus.x <= 0 || suurus.y <= 0) return;
	const int veerg = static_cast<int>(punkt.x * ms.getLaius() / suurus.x);
	const int rida = static_cast<int>(punkt.y * ms.getKorgus() / suurus.y);
	if (rida >= ms.getKorgus() || veerg >= ms.getLaius()) return;
	ms.teeKaik(rida, veerg, nupp == sf::Mouse::Button::Right);
}

// Terminali mäng: laud prinditakse ja käigud küsitakse, kuni mäng saab läbi.
void mangiTerminalis(Minesweeper& ms) {
	while (!ms.kasMangOnLabi()) {
		ms.prindiMangulaud();
		ms.kaikTerminalis();
	}
	ms.prindiMangulaud();
}

// Graafiline mäng: aken on avatud ainult mängu ajal. Mängu lõpus jääb laud nähtavaks, kuni
// mängija klõpsab või vajutab klahvi. Tagastab false, kui aken suleti enne mängu lõppu.
bool mangiGraafiliselt(Minesweeper& ms, const Tekstuurid& tekstuurid) {
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
				aken.setView(sf::View(sf::FloatRect({ 0.f, 0.f },
					{ static_cast<float>(muudetud->size.x), static_cast<float>(muudetud->size.y) })));
			}
			else if (const auto* hiir = sundmus->getIf<sf::Event::MouseButtonPressed>()) {
				if (ms.kasMangOnLabi()) aken.close();
				else klops(ms, aken, hiir->button, hiir->position);
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
		kuvaMangulaud(ms, tekstuurid, aken);
		aken.display();
	}
	return ms.kasMangOnLabi();
}

} // namespace

int main() {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	Tekstuurid tekstuurid;
	try {
		tekstuurid.lae("Pildid");
	}
	catch (const std::string& fail) {
		std::cout << "Pildifailist " << fail << " sisselugemine ebaõnnestus!\n";
		return 1;
	}

	Minesweeper ms;
	try {
		while (true) {
			ms.kusiManguLauaAndmed();
			if (ms.getKuvaGraafika()) {
				if (!mangiGraafiliselt(ms, tekstuurid)) break;
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
