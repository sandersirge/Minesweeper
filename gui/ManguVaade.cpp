#include "ManguVaade.hpp"
#include "Kiri.hpp"
#include "Teema.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace {
constexpr float RUUT = static_cast<float>(teema::RUUT);
constexpr float LAINE_SAMM = 0.035f;   // sekundit ühe ruudu kauguse kohta
constexpr float LAINE_JUHUS = 0.05f;   // juhuslik lisaviivitus, et laine oleks elavam
constexpr float LAINE_RUUT = 0.28f;    // ühe ruudu efekti kestus
} // namespace

void ManguVaade::alusta(const Seaded& uued) {
	seaded = uued;
	mang.naitaTeateid(false);
	mang.uusMangPommidega(seaded.korgus, seaded.laius, static_cast<size_t>(seaded.pommid));
	const sf::Vector2u suurus = loogilineSuurus();
	laudX = std::floor((suurus.x - seaded.laius * RUUT) / 2.f);
	laudY = static_cast<float>(teema::RIBA);
	hover.reset();
	vajutatud.reset();
	vasakAll = naguAll = naguHover = false;
	tagasi = false;
	aeg = lopuAeg = 0.f;
	lainetab = false;
	viivitused.clear();
}

sf::Vector2u ManguVaade::loogilineSuurus() const {
	const unsigned laius = std::max<unsigned>(seaded.laius * teema::RUUT, teema::MIN_LAIUS);
	return { laius, static_cast<unsigned>(teema::RIBA + seaded.korgus * teema::RUUT) };
}

std::optional<ManguVaade::Ruut> ManguVaade::ruutKohal(sf::Vector2f hiir) const {
	const float x = hiir.x - laudX;
	const float y = hiir.y - laudY;
	if (x < 0 || y < 0) return std::nullopt;
	const int veerg = static_cast<int>(x / RUUT);
	const int rida = static_cast<int>(y / RUUT);
	if (veerg >= seaded.laius || rida >= seaded.korgus) return std::nullopt;
	return Ruut{ rida, veerg };
}

sf::FloatRect ManguVaade::naoAla() const {
	const float kesk = std::floor(loogilineSuurus().x / 2.f);
	return sf::FloatRect({ kesk - 10.f, 2.f }, { 20.f, 20.f });
}

void ManguVaade::sundmus(const sf::Event& sundmus, sf::Vector2f hiir) {
	if (sundmus.is<sf::Event::MouseMoved>()) {
		hover = ruutKohal(hiir);
		naguHover = naoAla().contains(hiir);
	}
	else if (sundmus.is<sf::Event::MouseLeft>()) {
		hover.reset();
		naguHover = false;
	}
	else if (const auto* vajutus = sundmus.getIf<sf::Event::MouseButtonPressed>()) {
		const auto ruut = ruutKohal(hiir);
		switch (vajutus->button) {
		case sf::Mouse::Button::Left:
			if (naoAla().contains(hiir)) naguAll = true;
			else if (ruut && !mang.kasMangOnLabi()) {
				vasakAll = true;
				vajutatud = ruut;
			}
			break;
		case sf::Mouse::Button::Right:
			if (ruut) kaik(*ruut, true, false);
			break;
		case sf::Mouse::Button::Middle:
			if (ruut) kaik(*ruut, false, true);
			break;
		default: break;
		}
	}
	else if (const auto* lahti = sundmus.getIf<sf::Event::MouseButtonReleased>()) {
		if (lahti->button != sf::Mouse::Button::Left) return;
		if (naguAll && naoAla().contains(hiir)) alusta(seaded);
		else if (vasakAll) {
			// Ruut avatakse lahti laskmisel ja ainult siis, kui hiir on samal ruudul
			const auto ruut = ruutKohal(hiir);
			if (ruut && ruut == vajutatud) kaik(*ruut, false, false);
		}
		naguAll = false;
		vasakAll = false;
		vajutatud.reset();
	}
	else if (const auto* klahv = sundmus.getIf<sf::Event::KeyPressed>()) {
		if (klahv->code == sf::Keyboard::Key::Enter || klahv->code == sf::Keyboard::Key::F2) alusta(seaded);
		else if (klahv->code == sf::Keyboard::Key::Escape) tagasi = true;
	}
}

void ManguVaade::kaik(const Ruut& ruut, bool lipp, bool umber) {
	if (mang.kasMangOnLabi()) return;
	if (lipp) {
		mang.teeKaik(ruut.rida, ruut.veerg, true);
		return;
	}
	const char praegu = mang.ruut(ruut.rida, ruut.veerg);
	if (umber || (praegu >= '1' && praegu <= '8')) mang.avaUmber(ruut.rida, ruut.veerg);
	else mang.teeKaik(ruut.rida, ruut.veerg, false);

	if (mang.kasMangOnLabi()) algaLaine(ruut);
}

// Mängu lõpus levib klõpsatud ruudust laine (kaotusel punane, võidul valge)
void ManguVaade::algaLaine(const Ruut& algus) {
	std::uniform_real_distribution<float> juhuslik(0.f, LAINE_JUHUS);
	viivitused.assign(static_cast<size_t>(seaded.korgus) * seaded.laius, 0.f);
	laineKestus = 0.f;
	for (int r = 0; r < seaded.korgus; r++) {
		for (int v = 0; v < seaded.laius; v++) {
			const float kaugus = std::hypot(static_cast<float>(r - algus.rida),
			                                static_cast<float>(v - algus.veerg));
			const float viivitus = kaugus * LAINE_SAMM + juhuslik(juhus);
			viivitused[static_cast<size_t>(r) * seaded.laius + v] = viivitus;
			laineKestus = std::max(laineKestus, viivitus + LAINE_RUUT);
		}
	}
	lopuAeg = 0.f;
	lainetab = true;
}

void ManguVaade::uuenda(float sekundid) {
	if (mang.kasAlanud() && !mang.kasMangOnLabi()) aeg += sekundid;
	if (mang.kasMangOnLabi()) lopuAeg += sekundid;
}

ikoon::Nagu ManguVaade::naoOlek() const {
	if (mang.kasMangOnLabi()) return mang.kasOliKaotus() ? ikoon::Nagu::Kaotus : ikoon::Nagu::Voit;
	if (vasakAll) return ikoon::Nagu::Vajutab;
	return ikoon::Nagu::Tavaline;
}

void ManguVaade::joonista(Louend& louend) const {
	const sf::Vector2u suurus = loogilineSuurus();
	louend.ristkylik(0, 0, static_cast<float>(suurus.x), static_cast<float>(suurus.y), teema::TAUST);
	joonistaRiba(louend);
	for (int r = 0; r < seaded.korgus; r++) {
		for (int v = 0; v < seaded.laius; v++) joonistaRuut(louend, r, v);
	}
	if (lainetab) joonistaLaine(louend);
	if (mang.kasMangOnLabi() && lopuAeg >= laineKestus) joonistaTeade(louend);
}

void ManguVaade::joonistaRiba(Louend& louend) const {
	const float laius = static_cast<float>(loogilineSuurus().x);
	louend.ristkylik(0, 0, laius, teema::RIBA, teema::RIBA_TAUST);
	louend.ristkylik(0, teema::RIBA - 1, laius, 1, teema::TAUST);

	// Vasakul: mitu pommi on veel märkimata (lippude järgi, võib minna miinusesse)
	const long jaanud = static_cast<long>(mang.getPommideArv()) - static_cast<long>(mang.getLippe());
	ikoon::pomm(louend, 4, 4);
	kiri::kirjutaVarjuga(louend, std::to_string(jaanud), 22, 9, teema::TEKST, teema::VARI);

	// Keskel: nägu, mis on ka "uus mäng" nupp
	const sf::FloatRect ala = naoAla();
	Nupp naoNupp{ ala, "" };
	joonistaNupp(louend, naoNupp, naguHover, naguAll && naguHover);
	const float nihe = (naguAll && naguHover) ? 1.f : 0.f;
	ikoon::nagu(louend, naoOlek(), ala.position.x + 2, ala.position.y + 2 + nihe);

	// Paremal: aeg sekundites (kuni 999)
	const std::string aegTekst = std::to_string(std::min(999, static_cast<int>(aeg)));
	kiri::kirjutaVarjuga(louend, aegTekst, laius - 6 - kiri::laius(aegTekst), 9, teema::TEKST,
	                     teema::VARI);
}

void ManguVaade::joonistaRuut(Louend& louend, int rida, int veerg) const {
	const float x = laudX + veerg * RUUT;
	const float y = laudY + rida * RUUT;
	const float s = RUUT - 1.f; // 1-piksline vahe ruutude vahel jääb tausta värvi
	const char c = mang.ruut(rida, veerg);
	const bool suletud = c == Minesweeper::AVAMATA || c == Minesweeper::LIPP;

	if (suletud) {
		const bool hiirPeal = hover && hover->rida == rida && hover->veerg == veerg;
		const bool all = hiirPeal && vasakAll && c == Minesweeper::AVAMATA;
		sf::Color varv = teema::SULETUD;
		if (!mang.kasMangOnLabi()) {
			if (all) varv = teema::VAJUTATUD;
			else if (hiirPeal) varv = teema::HOVER;
		}
		louend.ristkylik(x, y, s, s, varv);
		if (!all) {
			// Kerge reljeef: hele ülemine ja vasak serv, tume alumine ja parem serv
			louend.ristkylik(x, y, s, 1, teema::SULETUD_HELE);
			louend.ristkylik(x, y, 1, s, teema::SULETUD_HELE);
			louend.ristkylik(x, y + s - 1, s, 1, teema::SULETUD_TUME);
			louend.ristkylik(x + s - 1, y, 1, s, teema::SULETUD_TUME);
		}
		if (c == Minesweeper::LIPP) ikoon::lipp(louend, x, y);
		return;
	}

	louend.ristkylik(x, y, s, s, c == Minesweeper::PLAHVATANUD ? teema::PLAHVATUS : teema::AVATUD);
	if (c >= '1' && c <= '8') {
		const std::string number(1, c);
		// Number kahekordse suurusega (10x14) ruudu keskel
		kiri::kirjuta(louend, number, x + 3, y + 1, teema::NUMBRID[c - '0'], 2.f);
	}
	else if (c == Minesweeper::POMM || c == Minesweeper::PLAHVATANUD) {
		ikoon::pomm(louend, x, y);
	}
	else if (c == Minesweeper::VALE_LIPP) {
		ikoon::pomm(louend, x, y);
		ikoon::rist(louend, x, y);
	}
}

void ManguVaade::joonistaLaine(Louend& louend) const {
	sf::Color varv = mang.kasOliKaotus() ? teema::LAINE_KAOTUS : teema::LAINE_VOIT;
	for (int r = 0; r < seaded.korgus; r++) {
		for (int v = 0; v < seaded.laius; v++) {
			const float t = lopuAeg - viivitused[static_cast<size_t>(r) * seaded.laius + v];
			if (t < 0.f || t >= LAINE_RUUT) continue;
			const float k = t / LAINE_RUUT;
			// Ruudu keskelt kasvav ruut, mis samal ajal hääbub
			const float suurus = 2.f * std::round(k * (RUUT - 1.f) / 2.f);
			varv.a = static_cast<std::uint8_t>(230.f * (1.f - k));
			const float x = laudX + v * RUUT + std::floor((RUUT - 1.f - suurus) / 2.f);
			const float y = laudY + r * RUUT + std::floor((RUUT - 1.f - suurus) / 2.f);
			louend.ristkylik(x, y, suurus, suurus, varv);
		}
	}
}

void ManguVaade::joonistaTeade(Louend& louend) const {
	const sf::Vector2u suurus = loogilineSuurus();
	const float kesk = std::floor(suurus.x / 2.f);
	const float y = std::floor(laudY + (seaded.korgus * RUUT - 56.f) / 2.f);
	louend.ristkylik(0, y, static_cast<float>(suurus.x), 56, teema::TEADE_TAUST);
	const bool kaotus = mang.kasOliKaotus();
	const sf::Color varv = kaotus ? teema::LAINE_KAOTUS : teema::TEKST;
	kiri::kirjutaKeskele(louend, kaotus ? "MÄNG LÄBI" : "VÕIT!", kesk, y + 8, varv, 2.f);
	kiri::kirjutaKeskele(louend, "ENTER - UUESTI", kesk, y + 31, teema::TEKST);
	kiri::kirjutaKeskele(louend, "ESC - MENÜÜ", kesk, y + 43, teema::TEKST_TUHM);
}
