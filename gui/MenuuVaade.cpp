#include "MenuuVaade.hpp"
#include "Ikoonid.hpp"
#include "Kiri.hpp"
#include "Teema.hpp"

#include <algorithm>
#include <string>

namespace {

struct Raskusaste {
	const char* nimi;
	Seaded seaded;
};

// Klassikalised raskusastmed: algaja 9x9/10, keskmine 16x16/40, ekspert 16x30/99
constexpr Raskusaste RASKUSASTMED[3] = {
	{ "ALGAJA", { 9, 9, 10 } },
	{ "KESKMINE", { 16, 16, 40 } },
	{ "EKSPERT", { 16, 30, 99 } },
};

std::string moodud(const Seaded& s) {
	return std::to_string(s.korgus) + "x" + std::to_string(s.laius) + " · " +
	       std::to_string(s.pommid);
}

bool vordne(const Seaded& a, const Seaded& b) {
	return a.korgus == b.korgus && a.laius == b.laius && a.pommid == b.pommid;
}

constexpr float SAMMU_Y[3] = { 166.f, 182.f, 198.f }; // KÕRGUS, LAIUS, POMME read

} // namespace

MenuuVaade::MenuuVaade() {
	for (std::size_t i = 0; i < 3; i++) {
		nupud[ALGAJA + i] = { sf::FloatRect({ 12.f, 88.f + 20.f * i }, { 152.f, 16.f }),
		                      RASKUSASTMED[i].nimi, moodud(RASKUSASTMED[i].seaded) };
	}
	for (std::size_t rida = 0; rida < 3; rida++) {
		nupud[KORGUS_MIINUS + 2 * rida] = { sf::FloatRect({ 100.f, SAMMU_Y[rida] }, { 14.f, 12.f }), "-" };
		nupud[KORGUS_PLUSS + 2 * rida] = { sf::FloatRect({ 150.f, SAMMU_Y[rida] }, { 14.f, 12.f }), "+" };
	}
	nupud[ALUSTA] = { sf::FloatRect({ 12.f, 214.f }, { 104.f, 16.f }), "ALUSTA" };
	nupud[VALJU] = { sf::FloatRect({ 120.f, 214.f }, { 44.f, 16.f }), "VÄLJU" };
	seaded = RASKUSASTMED[0].seaded;
	uuendaValitud();
}

std::optional<Seaded> MenuuVaade::votaAlustamine() {
	if (!alusta) return std::nullopt;
	alusta = false;
	return seaded;
}

std::optional<std::size_t> MenuuVaade::nuppKohal(sf::Vector2f hiir) const {
	for (std::size_t i = 0; i < NUPPE; i++) {
		if (nupud[i].sees(hiir)) return i;
	}
	return std::nullopt;
}

void MenuuVaade::sundmus(const sf::Event& sundmus, sf::Vector2f hiir) {
	if (sundmus.is<sf::Event::MouseMoved>()) {
		hoverNupp = nuppKohal(hiir);
	}
	else if (sundmus.is<sf::Event::MouseLeft>()) {
		hoverNupp.reset();
	}
	else if (const auto* vajutus = sundmus.getIf<sf::Event::MouseButtonPressed>()) {
		if (vajutus->button != sf::Mouse::Button::Left) return;
		vajutatudNupp = nuppKohal(hiir);
		hoidmiseAeg = 0.f;
		kordusAeg = 0.f;
		// +/- muudab kohe vajutamisel (ja hoides korduvalt), teised nupud lahti laskmisel
		if (vajutatudNupp && *vajutatudNupp >= KORGUS_MIINUS && *vajutatudNupp <= POMMID_PLUSS) {
			muuda(*vajutatudNupp);
		}
	}
	else if (const auto* lahti = sundmus.getIf<sf::Event::MouseButtonReleased>()) {
		if (lahti->button != sf::Mouse::Button::Left) return;
		const auto all = nuppKohal(hiir);
		if (vajutatudNupp && all == vajutatudNupp) vajuta(*vajutatudNupp);
		vajutatudNupp.reset();
	}
	else if (const auto* klahv = sundmus.getIf<sf::Event::KeyPressed>()) {
		switch (klahv->code) {
		case sf::Keyboard::Key::Enter: alusta = true; break;
		case sf::Keyboard::Key::Escape: valjuda = true; break;
		case sf::Keyboard::Key::Num1: valiRaskusaste(ALGAJA); break;
		case sf::Keyboard::Key::Num2: valiRaskusaste(KESKMINE); break;
		case sf::Keyboard::Key::Num3: valiRaskusaste(EKSPERT); break;
		default: break;
		}
	}
}

void MenuuVaade::uuenda(float sekundid) {
	if (!vajutatudNupp || *vajutatudNupp < KORGUS_MIINUS || *vajutatudNupp > POMMID_PLUSS) return;
	if (hoverNupp != vajutatudNupp) return;
	hoidmiseAeg += sekundid;
	if (hoidmiseAeg < 0.4f) return;
	kordusAeg += sekundid;
	while (kordusAeg >= 0.05f) {
		kordusAeg -= 0.05f;
		muuda(*vajutatudNupp);
	}
}

void MenuuVaade::vajuta(std::size_t id) {
	if (id <= EKSPERT) valiRaskusaste(id);
	else if (id == ALUSTA) alusta = true;
	else if (id == VALJU) valjuda = true;
}

void MenuuVaade::muuda(std::size_t id) {
	switch (id) {
	case KORGUS_MIINUS: seaded.korgus--; break;
	case KORGUS_PLUSS: seaded.korgus++; break;
	case LAIUS_MIINUS: seaded.laius--; break;
	case LAIUS_PLUSS: seaded.laius++; break;
	case POMMID_MIINUS: seaded.pommid--; break;
	case POMMID_PLUSS: seaded.pommid++; break;
	default: return;
	}
	seaded.korgus = std::clamp(seaded.korgus, MIN_KULG, MAX_KORGUS);
	seaded.laius = std::clamp(seaded.laius, MIN_KULG, MAX_LAIUS);
	piiraPomme();
	uuendaValitud();
}

void MenuuVaade::valiRaskusaste(std::size_t id) {
	seaded = RASKUSASTMED[id - ALGAJA].seaded;
	uuendaValitud();
}

// Vähemalt üks pomm ja vähemalt üks vaba ruut
void MenuuVaade::piiraPomme() {
	seaded.pommid = std::clamp(seaded.pommid, 1, seaded.korgus * seaded.laius - 1);
}

void MenuuVaade::uuendaValitud() {
	for (std::size_t i = 0; i < 3; i++) {
		nupud[ALGAJA + i].valitud = vordne(seaded, RASKUSASTMED[i].seaded);
	}
}

void MenuuVaade::joonista(Louend& louend) const {
	const float kesk = LAIUS / 2.f;
	louend.ristkylik(0, 0, LAIUS, KORGUS, teema::TAUST);

	kiri::kirjutaVarjuga(louend, "MINESWEEPER", 23, 12, teema::TEKST, teema::VARI, 2.f);
	const ikoon::Nagu nagu = vajutatudNupp ? ikoon::Nagu::Vajutab : ikoon::Nagu::Tavaline;
	ikoon::nagu(louend, nagu, kesk - 16, 36, 2.f);

	kiri::kirjutaKeskele(louend, "RASKUSASTE", kesk, 77, teema::TEKST_TUHM);
	kiri::kirjutaKeskele(louend, "KOHANDATUD", kesk, 153, teema::TEKST_TUHM);

	const char* sildid[3] = { "KÕRGUS", "LAIUS", "POMME" };
	const int vaartused[3] = { seaded.korgus, seaded.laius, seaded.pommid };
	for (int rida = 0; rida < 3; rida++) {
		kiri::kirjuta(louend, sildid[rida], 14, SAMMU_Y[rida] + 2, teema::TEKST);
		kiri::kirjutaKeskele(louend, std::to_string(vaartused[rida]), 132, SAMMU_Y[rida] + 2,
		                     teema::TEKST);
	}

	for (std::size_t i = 0; i < NUPPE; i++) {
		const bool hover = hoverNupp == i;
		joonistaNupp(louend, nupud[i], hover, hover && vajutatudNupp == i);
	}
}
