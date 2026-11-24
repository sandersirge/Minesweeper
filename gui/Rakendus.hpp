#pragma once
#include "Louend.hpp"
#include "ManguVaade.hpp"
#include "MenuuVaade.hpp"

// Aken ja vaadete vahetamine (menüü <-> mäng). Aken muudab suurust vaate järgi ja suurendab
// loogilisi piksleid täisarvu korda, nii et piksel-graafika jääb teravaks.
class Rakendus {
public:
	int kaivita();

private:
	sf::RenderWindow aken;
	Louend louend;
	MenuuVaade menuu;
	ManguVaade mang;
	Vaade* praegune = &menuu;

	void naita(Vaade& vaade);
	sf::Vector2f loogiline(sf::Vector2i piksel) const;
};
