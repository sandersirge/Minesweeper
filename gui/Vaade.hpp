#pragma once
#include "Louend.hpp"

#include <string>
#include <utility>

// Ühine liides menüü- ja mänguvaatele. Hiire asukoht antakse loogilistes pikslites.
class Vaade {
public:
	virtual ~Vaade() = default;
	virtual sf::Vector2u loogilineSuurus() const = 0;
	virtual void sundmus(const sf::Event& sundmus, sf::Vector2f hiir) = 0;
	virtual void uuenda(float sekundid) = 0;
	virtual void joonista(Louend& louend) const = 0;
};

// Lihtne nupp: ristkülik, tekst vasakul ja soovi korral teine tekst paremal
struct Nupp {
	sf::FloatRect ala;
	std::string tekst;
	std::string parem;     // nt raskusastme mõõdud
	bool valitud = false;  // püsivalt esile tõstetud (valitud raskusaste)

	Nupp() = default;
	Nupp(sf::FloatRect uusAla, std::string uusTekst, std::string uusParem = {})
		: ala(uusAla), tekst(std::move(uusTekst)), parem(std::move(uusParem)) {}

	bool sees(sf::Vector2f p) const { return ala.contains(p); }
};

// Joonistab nupu: hover = hiir on peal, vajutatud = hiire nupp on selle peal all
void joonistaNupp(Louend& louend, const Nupp& nupp, bool hover, bool vajutatud);
