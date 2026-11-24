#include "Minesweeper.h"

#include <algorithm>
#include <iostream>
#include <limits>
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

void Minesweeper::uusMang(int uusKorgus, int uusLaius, int protsent, bool graafiline) {
	const size_t ruute = static_cast<size_t>(uusKorgus) * static_cast<size_t>(uusLaius);
	uusMangPommidega(uusKorgus, uusLaius, ruute * static_cast<size_t>(protsent) / 100);
	kuvaGraafiliselt = graafiline;
}

void Minesweeper::uusMangPommidega(int uusKorgus, int uusLaius, size_t pommid) {
	if (uusKorgus < 1 || uusLaius < 1) throw std::invalid_argument("laud peab olema vähemalt 1x1");
	korgus = uusKorgus;
	laius = uusLaius;
	vektoritePikkus = static_cast<size_t>(korgus) * static_cast<size_t>(laius);
	pommideArv = std::min(pommid, vektoritePikkus - 1);

	mangOnLabi = false;
	oliKaotus = false;
	pommidPaigutatud = false;
	avatudRuute = 0;
	this->pommid.assign(vektoritePikkus, false);
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

void Minesweeper::teade(const char* tekst) const {
	if (teated) std::cout << tekst << '\n';
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

bool Minesweeper::avaRuut(size_t indeks) {
	if (!pommidPaigutatud) paigutaPommid(indeks);
	if (pommid[indeks]) {
		kaotus(indeks);
		return false;
	}
	avaRuudud(indeks);
	return true;
}

// Kaotuse järel näidatakse pomme (õiged lipud jäävad, valed lipud märgitakse); ülejäänud
// avamata ruudud jäävad kinni nagu klassikalises Minesweeperis.
void Minesweeper::kaotus(size_t plahvatanudIndeks) {
	mangOnLabi = true;
	oliKaotus = true;
	for (size_t i = 0; i < vektoritePikkus; i++) {
		if (pommid[i] && mangijaLaud[i] == AVAMATA) mangijaLaud[i] = POMM;
		else if (!pommid[i] && mangijaLaud[i] == LIPP) mangijaLaud[i] = VALE_LIPP;
	}
	mangijaLaud[plahvatanudIndeks] = PLAHVATANUD;
	teade("Avasid pommi!");
}

// Võit: kõik ruudud peale pommide on avatud (lippe ei pea panema)
void Minesweeper::kontrolliVoitu() {
	if (mangOnLabi || avatudRuute != vektoritePikkus - pommideArv) return;
	mangOnLabi = true;
	oliKaotus = false;
	for (size_t i = 0; i < vektoritePikkus; i++) {
		if (pommid[i]) mangijaLaud[i] = LIPP;
	}
}

// Muudab mängija lauda; kuvamine toimub eraldi (prindiMangulaud või graafiline liides)
bool Minesweeper::teeKaik(int rida, int veerg, bool kasLipp) {
	if (mangOnLabi) return false;
	if (rida < 0 || rida >= korgus || veerg < 0 || veerg >= laius) {
		teade("Valitud käik ei ole mängulaua sees!");
		return false;
	}
	const size_t indeks = static_cast<size_t>(rida) * laius + veerg;
	const char praegu = mangijaLaud[indeks];

	if (kasLipp) {
		if (praegu == AVAMATA) mangijaLaud[indeks] = LIPP;
		else if (praegu == LIPP) mangijaLaud[indeks] = AVAMATA; // uus lipukäik võtab lipu ära
		else {
			teade("Avatud ruudule ei saa lippu panna!");
			return false;
		}
		return true;
	}

	if (praegu == LIPP) {
		teade("Lipuga ruutu ei saa avada! (võta enne lipp ära)");
		return false;
	}
	if (praegu != AVAMATA) {
		teade("See ruut on juba avatud.");
		return false;
	}

	if (avaRuut(indeks)) kontrolliVoitu();
	return true;
}

bool Minesweeper::avaUmber(int rida, int veerg) {
	if (mangOnLabi || rida < 0 || rida >= korgus || veerg < 0 || veerg >= laius) return false;
	const size_t indeks = static_cast<size_t>(rida) * laius + veerg;
	const char praegu = mangijaLaud[indeks];
	if (praegu < '1' || praegu > '8') return false;

	const Naabrid ymber = naabrid(indeks);
	int lippe = 0;
	for (size_t n : ymber) {
		if (mangijaLaud[n] == LIPP) lippe++;
	}
	if (lippe != praegu - '0') return false;

	bool avati = false;
	for (size_t n : ymber) {
		if (mangijaLaud[n] != AVAMATA) continue;
		avati = true;
		if (!avaRuut(n)) return true; // valesti pandud lipp - pomm avati, mäng läbi
	}
	kontrolliVoitu();
	return avati;
}

size_t Minesweeper::getLippe() const {
	return static_cast<size_t>(std::count(mangijaLaud.begin(), mangijaLaud.end(), LIPP));
}

void Minesweeper::kaikTerminalis() {
	const int l = loeTaisarv("Kas panna lipp? (1/0) ", 0, 1);
	const int y = loeTaisarv("Vali rida (0-" + std::to_string(korgus - 1) + "): ", 0, korgus - 1);
	const int x = loeTaisarv("Vali veerg (0-" + std::to_string(laius - 1) + "): ", 0, laius - 1);
	teeKaik(y, x, l == 1);
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
