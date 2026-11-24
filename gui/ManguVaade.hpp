#pragma once
#include "Ikoonid.hpp"
#include "MenuuVaade.hpp"
#include "Minesweeper.h"
#include "Vaade.hpp"

#include <optional>
#include <random>
#include <vector>

// Mänguvaade: ülemine riba (pomme jäänud, nägu = uus mäng, aeg) ja mängulaud.
//   vasak klõps    avab ruudu; avatud numbril avab ümbruse, kui lippe on õige arv
//   parem klõps    paneb või võtab lipu
//   keskmine klõps avab numbri ümbruse
//   Enter / F2     uus mäng samade seadetega, Esc tagasi menüüsse
class ManguVaade : public Vaade {
public:
	void alusta(const Seaded& uued);

	sf::Vector2u loogilineSuurus() const override;
	void sundmus(const sf::Event& sundmus, sf::Vector2f hiir) override;
	void uuenda(float sekundid) override;
	void joonista(Louend& louend) const override;

	bool kasTagasiMenuusse() const { return tagasi; }
	const Minesweeper& getMang() const { return mang; }

private:
	struct Ruut {
		int rida;
		int veerg;
		bool operator==(const Ruut& t) const { return rida == t.rida && veerg == t.veerg; }
	};

	Seaded seaded;
	Minesweeper mang;
	float laudX = 0.f;
	float laudY = 0.f;

	std::optional<Ruut> hover;
	bool vasakAll = false;
	std::optional<Ruut> vajutatud; // ruut, millel vasak nupp alla vajutati
	bool naguHover = false;
	bool naguAll = false;
	bool tagasi = false;

	float aeg = 0.f;           // mängu kestus (sekundid), jookseb esimesest avamisest lõpuni
	float lopuAeg = 0.f;       // aeg mängu lõpust (laine animatsioon)
	bool lainetab = false;
	std::vector<float> viivitused; // iga ruudu laine algus (sekundites mängu lõpust)
	float laineKestus = 0.f;
	std::mt19937 juhus{ std::random_device{}() };

	std::optional<Ruut> ruutKohal(sf::Vector2f hiir) const;
	sf::FloatRect naoAla() const;
	void kaik(const Ruut& ruut, bool lipp, bool umber);
	void algaLaine(const Ruut& algus);
	ikoon::Nagu naoOlek() const;
	void joonistaRiba(Louend& louend) const;
	void joonistaRuut(Louend& louend, int rida, int veerg) const;
	void joonistaLaine(Louend& louend) const;
	void joonistaTeade(Louend& louend) const;
};
