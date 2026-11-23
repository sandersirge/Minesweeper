#include "Minesweeper.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>

namespace {
std::mt19937& juhuslikkus() {
	static std::mt19937 generaator{ std::random_device{}() };
	return generaator;
}
} // namespace

int loeTaisarv(const std::string& kusimus, int min, int max) {
	while (true) {
		std::cout << kusimus;
		int vaartus{};
		if (std::cin >> vaartus) {
			if (vaartus >= min && vaartus <= max) return vaartus;
			std::cout << "Sisesta arv vahemikus " << min << "-" << max << ".\n";
			continue;
		}
		if (std::cin.eof()) throw SisendLopetatud();
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "Sisesta täisarv.\n";
	}
}

struct Minesweeper::Tekstuurid {
	sf::Texture pomm, avamata, tuhi, lipp;
	sf::Texture numbrid[8]; // 1...8
	std::map<char, const sf::Texture*> charTekstuuriks;
};

void Minesweeper::laeTekstuurid(const std::string& kaust) {
	auto uued = std::make_shared<Tekstuurid>();
	const std::filesystem::path pildid{ kaust };
	const auto lae = [&](sf::Texture& tekstuur, const char* nimi) {
		const std::string fail = (pildid / nimi).string();
		if (!tekstuur.loadFromFile(fail)) throw fail;
	};
	lae(uued->lipp, "lipp.png");
	lae(uued->pomm, "pomm.png");
	lae(uued->tuhi, "tuhi.png");
	lae(uued->avamata, "avamata.png");
	const char* numbriPildid[8] = { "yks.png", "kaks.png", "kolm.png", "neli.png",
		"viis.png", "kuus.png", "seitse.png", "kaheksa.png" };
	for (int i = 0; i < 8; i++) lae(uued->numbrid[i], numbriPildid[i]);

	// Viidad, mitte koopiad - iga pilt on mälus ainult üks kord
	uued->charTekstuuriks[AVAMATA] = &uued->avamata;
	uued->charTekstuuriks[TUHI] = &uued->tuhi;
	uued->charTekstuuriks[LIPP] = &uued->lipp;
	uued->charTekstuuriks[POMM] = &uued->pomm;
	for (int i = 0; i < 8; i++) uued->charTekstuuriks[static_cast<char>('1' + i)] = &uued->numbrid[i];
	tekstuurid = std::move(uued);
}

void Minesweeper::kusiManguLauaAndmed() {
	int k = 0;
	int l = 0;
	size_t ruute = 0;
	while (true) {
		k = loeTaisarv("Vali mängulaua kõrgus (1-" + std::to_string(MAX_KORGUS) + "): ", 1, MAX_KORGUS);
		l = loeTaisarv("Vali mängulaua laius (1-" + std::to_string(MAX_LAIUS) + "): ", 1, MAX_LAIUS);
		ruute = static_cast<size_t>(k) * static_cast<size_t>(l);
		if (ruute >= 2) break;
		std::cout << "Laual peab olema vähemalt 2 ruutu (üks pomm ja üks vaba ruut).\n";
	}
	const bool graafiline = loeTaisarv("Kas kuvada graafiline liides? (1/0): ", 0, 1) == 1;

	while (true) {
		const int p = loeTaisarv("Mitu protsenti ruutudest võiksid olla pommid (1-99): ", 1, 99);
		// Vähemalt üks pomm ja vähemalt üks vaba ruut, muidu pole mida mängida
		const size_t mituPommi = ruute * static_cast<size_t>(p) / 100;
		if (mituPommi == 0) {
			std::cout << "Sellise protsendiga ei tuleks ühtegi pommi, vali suurem protsent.\n";
			continue;
		}
		if (mituPommi >= ruute) {
			std::cout << "Sellise protsendiga oleksid kõik ruudud pommid, vali väiksem protsent.\n";
			continue;
		}
		uusMang(k, l, p, graafiline);
		return;
	}
}

void Minesweeper::uusMang(int uusKorgus, int uusLaius, int uusProtsent, bool graafiline) {
	korgus = uusKorgus;
	laius = uusLaius;
	protsent = uusProtsent;
	kuvaGraafiliselt = graafiline;
	vektoritePikkus = static_cast<size_t>(korgus) * static_cast<size_t>(laius);
	pommideArv = std::min(vektoritePikkus - 1, vektoritePikkus * static_cast<size_t>(protsent) / 100);

	mangOnLabi = false;
	oliKaotus = false;
	pommidPaigutatud = false;
	avatudRuute = 0;
	pommid.assign(vektoritePikkus, false);
	ruudud.assign(vektoritePikkus, 0);
	mangijaLaud.assign(vektoritePikkus, AVAMATA);
}

void Minesweeper::seaPommid(const std::vector<bool>& pommiKohad) {
	if (pommiKohad.size() != vektoritePikkus) throw std::invalid_argument("vale pommide arv");
	pommid = pommiKohad;
	pommideArv = static_cast<size_t>(std::count(pommid.begin(), pommid.end(), true));
	pommidPaigutatud = true;
	arvutaRuudud();
}

// Kuni 8 naabrit; ridade ääres ei "keerata" järgmisele/eelmisele reale
Minesweeper::Naabrid Minesweeper::naabrid(size_t indeks) const {
	Naabrid tulemus;
	const int rida = static_cast<int>(indeks / laius);
	const int veerg = static_cast<int>(indeks % laius);
	for (int dr = -1; dr <= 1; dr++) {
		for (int dv = -1; dv <= 1; dv++) {
			if (dr == 0 && dv == 0) continue;
			const int r = rida + dr;
			const int v = veerg + dv;
			if (r < 0 || r >= korgus || v < 0 || v >= laius) continue;
			tulemus.indeksid[tulemus.arv++] = static_cast<size_t>(r) * laius + v;
		}
	}
	return tulemus;
}

// Pommid pannakse alles esimese avamise ajal, et esimene käik ei oleks kunagi pomm. Kui ruumi
// on, jäetakse vabaks ka avatud ruudu naabrid, nii et esimene käik avab ala.
void Minesweeper::paigutaPommid(size_t ohutuIndeks) {
	std::vector<bool> keelatud(vektoritePikkus, false);
	keelatud.at(ohutuIndeks) = true;
	const Naabrid ymber = naabrid(ohutuIndeks);
	if (vektoritePikkus - 1 - ymber.size() >= pommideArv) {
		for (size_t n : ymber) keelatud.at(n) = true;
	}

	std::vector<size_t> kohad;
	kohad.reserve(vektoritePikkus);
	for (size_t i = 0; i < vektoritePikkus; i++) {
		if (!keelatud[i]) kohad.push_back(i);
	}
	std::shuffle(kohad.begin(), kohad.end(), juhuslikkus());

	pommid.assign(vektoritePikkus, false);
	for (size_t i = 0; i < pommideArv; i++) pommid.at(kohad.at(i)) = true;
	pommidPaigutatud = true;
	arvutaRuudud();
}

void Minesweeper::arvutaRuudud() {
	ruudud.assign(vektoritePikkus, 0);
	for (size_t i = 0; i < vektoritePikkus; i++) {
		if (pommid[i]) {
			ruudud[i] = -1;
			continue;
		}
		int loendur = 0;
		for (size_t n : naabrid(i)) {
			if (pommid[n]) loendur++;
		}
		ruudud[i] = loendur;
	}
}

// Avab ruudu; kui see on tühi, avab ka kõik ümbritsevad (8 suunda), kuni jõuab numbriteni.
// Järjekorraga, mitte rekursiooniga, et suur tühi ala ei ajaks pinu üle.
void Minesweeper::avaRuudud(size_t algusIndeks) {
	std::vector<size_t> jarjekord{ algusIndeks };
	while (!jarjekord.empty()) {
		const size_t indeks = jarjekord.back();
		jarjekord.pop_back();
		if (mangijaLaud[indeks] != AVAMATA || pommid[indeks]) continue; // lipud jäävad alles

		const int arv = ruudud[indeks];
		mangijaLaud[indeks] = arv == 0 ? TUHI : static_cast<char>('0' + arv);
		avatudRuute++;
		if (arv == 0) {
			for (size_t n : naabrid(indeks)) {
				if (mangijaLaud[n] == AVAMATA) jarjekord.push_back(n);
			}
		}
	}
}

void Minesweeper::reedaMangulaud() {
	for (size_t i = 0; i < vektoritePikkus; i++) {
		if (pommid[i]) mangijaLaud[i] = POMM;
		else if (ruudud[i] == 0) mangijaLaud[i] = TUHI;
		else mangijaLaud[i] = static_cast<char>('0' + ruudud[i]);
	}
}

// Muudab mängija lauda; kuvamine toimub eraldi (prindiMangulaud / kuvaMangulaud)
bool Minesweeper::teeKaik(int rida, int veerg, bool kasLipp) {
	if (mangOnLabi) return false;
	if (rida < 0 || rida >= korgus || veerg < 0 || veerg >= laius) {
		std::cout << "Valitud käik ei ole mängulaua sees!\n";
		return false;
	}
	const size_t indeks = static_cast<size_t>(rida) * laius + veerg;
	const char praegu = mangijaLaud[indeks];

	if (kasLipp) {
		if (praegu == AVAMATA) mangijaLaud[indeks] = LIPP;
		else if (praegu == LIPP) mangijaLaud[indeks] = AVAMATA; // uus lipukäik võtab lipu ära
		else {
			std::cout << "Avatud ruudule ei saa lippu panna!\n";
			return false;
		}
		return true;
	}

	if (praegu == LIPP) {
		std::cout << "Lipuga ruutu ei saa avada! (võta enne lipp ära)\n";
		return false;
	}
	if (praegu != AVAMATA) {
		std::cout << "See ruut on juba avatud.\n";
		return false;
	}

	if (!pommidPaigutatud) paigutaPommid(indeks);

	if (pommid[indeks]) {
		mangOnLabi = true;
		oliKaotus = true;
		reedaMangulaud();
		std::cout << "Avasid pommi!\n";
		return true;
	}

	avaRuudud(indeks);
	// Võit: kõik ruudud peale pommide on avatud (lippe ei pea panema)
	if (avatudRuute == vektoritePikkus - pommideArv) {
		mangOnLabi = true;
		oliKaotus = false;
		for (size_t i = 0; i < vektoritePikkus; i++) {
			if (pommid[i]) mangijaLaud[i] = LIPP;
		}
	}
	return true;
}

void Minesweeper::kaikTerminalis() {
	const int l = loeTaisarv("Kas panna lipp? (1/0) ", 0, 1);
	const int y = loeTaisarv("Vali rida (0-" + std::to_string(korgus - 1) + "): ", 0, korgus - 1);
	const int x = loeTaisarv("Vali veerg (0-" + std::to_string(laius - 1) + "): ", 0, laius - 1);
	teeKaik(y, x, l == 1);
}

void Minesweeper::handleEvent(const sf::RenderWindow& aken, sf::Mouse::Button nupp, sf::Vector2i asukoht) {
	if (nupp != sf::Mouse::Button::Left && nupp != sf::Mouse::Button::Right) return;
	// Pikslid vaate koordinaatideks, siis ruudu reaks/veeruks (ujukomaga, et ääred ei kaoks)
	const sf::Vector2f punkt = aken.mapPixelToCoords(asukoht);
	const sf::Vector2f suurus = aken.getView().getSize();
	if (punkt.x < 0 || punkt.y < 0 || suurus.x <= 0 || suurus.y <= 0) return;
	const int veerg = static_cast<int>(punkt.x * laius / suurus.x);
	const int rida = static_cast<int>(punkt.y * korgus / suurus.y);
	if (rida >= korgus || veerg >= laius) return;
	teeKaik(rida, veerg, nupp == sf::Mouse::Button::Right);
}

char Minesweeper::ruut(int rida, int veerg) const {
	return mangijaLaud.at(static_cast<size_t>(rida) * laius + veerg);
}

void Minesweeper::prindiMangulaud() const {
	// Veerunumbrid üleval ja reanumbrid vasakul, et käiku oleks lihtne sisestada
	std::cout << "\n    ";
	for (int v = 0; v < laius; v++) std::cout << (v % 10) << ' ';
	std::cout << '\n';
	for (int r = 0; r < korgus; r++) {
		std::cout << (r < 10 ? "  " : r < 100 ? " " : "") << r << ' ';
		for (int v = 0; v < laius; v++) std::cout << ruut(r, v) << ' ';
		std::cout << '\n';
	}
	std::cout << '\n';
}

// Akna suurust saab muuta, seega arvutatakse ruudu suurus vaate järgi, et kogu laud oleks näha
void Minesweeper::kuvaMangulaud(sf::RenderWindow& aken) const {
	if (!tekstuurid) return;
	const auto& charTekstuuriks = tekstuurid->charTekstuuriks;
	const sf::Vector2f suurus = aken.getView().getSize();
	const sf::Vector2f ruuduSuurus{ suurus.x / laius, suurus.y / korgus };
	sf::RectangleShape kujund(ruuduSuurus);
	for (int r = 0; r < korgus; r++) {
		for (int v = 0; v < laius; v++) {
			const auto tekstuur = charTekstuuriks.find(ruut(r, v));
			if (tekstuur == charTekstuuriks.end()) continue;
			kujund.setTexture(tekstuur->second, true);
			kujund.setPosition({ v * ruuduSuurus.x, r * ruuduSuurus.y });
			aken.draw(kujund);
		}
	}
}
