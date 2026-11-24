#include "Rakendus.hpp"
#include "Ikoonid.hpp"
#include "Teema.hpp"

#include <algorithm>
#include <optional>

namespace {

// Suurim täisarvuline suurendus, millega vaade mahub ekraanile (vähemalt 1, kuni 4)
unsigned suurendus(sf::Vector2u loogiline) {
	const sf::Vector2u ekraan = sf::VideoMode::getDesktopMode().size;
	const unsigned mahubX = static_cast<unsigned>(ekraan.x * 0.92f) / std::max(1u, loogiline.x);
	const unsigned mahubY = static_cast<unsigned>(ekraan.y * 0.85f) / std::max(1u, loogiline.y);
	return std::clamp(std::min(mahubX, mahubY), 1u, static_cast<unsigned>(teema::MAX_SUURENDUS));
}

} // namespace

int Rakendus::kaivita() {
	const sf::Vector2u algus = menuu.loogilineSuurus();
	const unsigned s = suurendus(algus);
	aken.create(sf::VideoMode({ algus.x * s, algus.y * s }), "Minesweeper",
	            sf::Style::Titlebar | sf::Style::Close);
	aken.setFramerateLimit(60);
	aken.setKeyRepeatEnabled(false);
	const sf::Image ikoonPilt = ikoon::naguPilt(ikoon::Nagu::Tavaline, 2);
	aken.setIcon(ikoonPilt);
	naita(menuu);

	sf::Clock kell;
	while (aken.isOpen()) {
		while (const std::optional sundmus = aken.pollEvent()) {
			if (sundmus->is<sf::Event::Closed>()) {
				aken.close();
				break;
			}
			// Hiire asukoht loogilistes pikslites (iga sündmuse jaoks oma asukoht)
			sf::Vector2f hiir{ -1.f, -1.f };
			if (const auto* liigub = sundmus->getIf<sf::Event::MouseMoved>()) hiir = loogiline(liigub->position);
			else if (const auto* all = sundmus->getIf<sf::Event::MouseButtonPressed>()) hiir = loogiline(all->position);
			else if (const auto* lahti = sundmus->getIf<sf::Event::MouseButtonReleased>()) hiir = loogiline(lahti->position);
			praegune->sundmus(*sundmus, hiir);
		}
		if (!aken.isOpen()) break;

		// Vaadete vahetus
		if (praegune == &menuu) {
			if (menuu.kasValjuda()) {
				aken.close();
				break;
			}
			if (const auto seaded = menuu.votaAlustamine()) {
				mang.alusta(*seaded);
				naita(mang);
			}
		}
		else if (mang.kasTagasiMenuusse()) {
			naita(menuu);
		}

		praegune->uuenda(kell.restart().asSeconds());

		aken.clear(teema::TAUST);
		louend.puhasta();
		praegune->joonista(louend);
		louend.joonista(aken);
		aken.display();
	}
	return 0;
}

// Muudab akna suuruse vaate järgi ja paigutab akna ekraani keskele
void Rakendus::naita(Vaade& vaade) {
	praegune = &vaade;
	const sf::Vector2u loogilineSuurus = vaade.loogilineSuurus();
	const unsigned s = suurendus(loogilineSuurus);
	const sf::Vector2u suurus{ loogilineSuurus.x * s, loogilineSuurus.y * s };
	aken.setSize(suurus);
	aken.setView(sf::View(sf::FloatRect({ 0.f, 0.f },
		{ static_cast<float>(loogilineSuurus.x), static_cast<float>(loogilineSuurus.y) })));
	const sf::Vector2u ekraan = sf::VideoMode::getDesktopMode().size;
	aken.setPosition({ (static_cast<int>(ekraan.x) - static_cast<int>(suurus.x)) / 2,
	                   (static_cast<int>(ekraan.y) - static_cast<int>(suurus.y)) / 2 });
	// Vana vaate hiire asukoht ei kehti enam: annab uuele vaatele teada, et hiir lahkus
	vaade.sundmus(sf::Event::MouseLeft{}, { -1.f, -1.f });
}

sf::Vector2f Rakendus::loogiline(sf::Vector2i piksel) const {
	return aken.mapPixelToCoords(piksel);
}
