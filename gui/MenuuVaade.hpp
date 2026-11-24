#pragma once
#include "Vaade.hpp"

#include <array>
#include <cstddef>
#include <optional>

// Mängulaua seaded, mille menüü annab mänguvaatele
struct Seaded {
	int korgus = 9;
	int laius = 9;
	int pommid = 10;
};

// Algusmenüü: raskusastmed, kohandatud laua suurus ja pommide arv, ALUSTA ja VÄLJU.
class MenuuVaade : public Vaade {
public:
	static constexpr int MIN_KULG = 5;
	static constexpr int MAX_KORGUS = 24;
	static constexpr int MAX_LAIUS = 40;

	MenuuVaade();

	sf::Vector2u loogilineSuurus() const override { return { LAIUS, KORGUS }; }
	void sundmus(const sf::Event& sundmus, sf::Vector2f hiir) override;
	void uuenda(float sekundid) override;
	void joonista(Louend& louend) const override;

	// Kui mängija vajutas ALUSTA, tagastab seaded (üks kord)
	std::optional<Seaded> votaAlustamine();
	bool kasValjuda() const { return valjuda; }
	const Seaded& getSeaded() const { return seaded; }

private:
	static constexpr unsigned LAIUS = 176;
	static constexpr unsigned KORGUS = 236;

	// Juhtnupud: 3 raskusastet, 3 x (-, +), ALUSTA, VÄLJU
	enum Id : std::size_t {
		ALGAJA, KESKMINE, EKSPERT,
		KORGUS_MIINUS, KORGUS_PLUSS, LAIUS_MIINUS, LAIUS_PLUSS, POMMID_MIINUS, POMMID_PLUSS,
		ALUSTA, VALJU, NUPPE
	};

	Seaded seaded;
	std::array<Nupp, NUPPE> nupud;
	std::optional<std::size_t> hoverNupp;
	std::optional<std::size_t> vajutatudNupp;
	float hoidmiseAeg = 0.f; // +/- all hoidmisel kordub muutus
	float kordusAeg = 0.f;
	bool alusta = false;
	bool valjuda = false;

	void vajuta(std::size_t id);
	void muuda(std::size_t id);
	void valiRaskusaste(std::size_t id);
	void piiraPomme();
	void uuendaValitud();
	std::optional<std::size_t> nuppKohal(sf::Vector2f hiir) const;
};
