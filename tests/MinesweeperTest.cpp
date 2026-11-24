// Mänguloogika testid. Akent ega pilte pole vaja: pommid pannakse seaPommid() abil kindlatele
// kohtadele. Käivita: ctest --test-dir build (või otse build/minesweeper_test).
#include "../Minesweeper.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int ebaonnestunud = 0;
int kokku = 0;

void kontrolli(bool tingimus, const std::string& kirjeldus) {
	kokku++;
	if (!tingimus) {
		ebaonnestunud++;
		std::cerr << "  EBAÕNNESTUS: " << kirjeldus << '\n';
	}
}

// Laud, kus pommid on antud (rida, veerg) kohtadel
Minesweeper laud(int korgus, int laius, const std::vector<std::pair<int, int>>& pommid) {
	Minesweeper ms;
	ms.uusMang(korgus, laius, 10, false);
	std::vector<bool> kohad(static_cast<size_t>(korgus) * laius, false);
	for (auto [r, v] : pommid) kohad.at(static_cast<size_t>(r) * laius + v) = true;
	ms.seaPommid(kohad);
	return ms;
}

// Vaigistab mängu teated (std::cout) testi ajaks
struct Vaikne {
	std::ostringstream puhver;
	std::streambuf* vana = std::cout.rdbuf(puhver.rdbuf());
	~Vaikne() { std::cout.rdbuf(vana); }
};

void test(const std::string& nimi, const std::function<void()>& keha) {
	const int enne = ebaonnestunud;
	{
		Vaikne vaikne;
		keha();
	}
	std::cout << (ebaonnestunud == enne ? "[ OK ] " : "[FAIL] ") << nimi << '\n';
}
} // namespace

int main() {
	test("numbrid ei keera rea äärest järgmisele reale", [] {
		// 3x4 laud, pomm paremal üleval (0,3). Vana kood luges (1,0) naabriks indeksi 3 ehk (0,3).
		Minesweeper ms = laud(3, 4, { { 0, 3 } });
		ms.teeKaik(2, 0, false);
		kontrolli(ms.ruut(1, 0) == Minesweeper::TUHI, "(1,0) on tühi, mitte 1");
		kontrolli(ms.ruut(0, 2) == '1', "(0,2) näitab 1");
		kontrolli(ms.ruut(1, 3) == '1', "(1,3) näitab 1");
	});

	test("tühja ala avamine viimase ruudu juures ei jookse kokku", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		kontrolli(ms.teeKaik(2, 2, false), "käik lubatud");
		kontrolli(ms.ruut(2, 2) == Minesweeper::TUHI, "(2,2) on tühi");
	});

	test("tühi ala avab ka diagonaalsed numbrid ja mäng võidetakse", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		ms.teeKaik(2, 2, false);
		kontrolli(ms.ruut(1, 1) == '1', "(1,1) avatud, 1");
		kontrolli(ms.ruut(0, 1) == '1' && ms.ruut(1, 0) == '1', "servanumbrid avatud");
		kontrolli(ms.kasMangOnLabi() && !ms.kasOliKaotus(), "kõik vabad ruudud avatud = võit");
		kontrolli(ms.ruut(0, 0) == Minesweeper::LIPP, "võidu järel on pommil lipp");
	});

	test("lipud jäävad tühja ala avamisel alles", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		ms.teeKaik(1, 1, true);
		ms.teeKaik(2, 2, false);
		kontrolli(ms.ruut(1, 1) == Minesweeper::LIPP, "lipp alles");
		kontrolli(!ms.kasMangOnLabi(), "lipu all on veel vaba ruut, mäng käib");
		ms.teeKaik(1, 1, true); // võtab lipu ära
		ms.teeKaik(1, 1, false);
		kontrolli(ms.kasMangOnLabi() && !ms.kasOliKaotus(), "viimase ruudu avamine võidab");
	});

	test("avatud ruudule ei saa lippu panna ega seda uuesti avada", [] {
		Minesweeper ms = laud(2, 3, { { 0, 0 } });
		ms.teeKaik(1, 1, false);
		kontrolli(ms.ruut(1, 1) == '1', "(1,1) avatud");
		kontrolli(!ms.teeKaik(1, 1, true), "lipukäik avatud ruudule keelatud");
		kontrolli(ms.ruut(1, 1) == '1', "ruut jäi avatuks");
		kontrolli(!ms.teeKaik(1, 1, false), "uuesti avamine keelatud");
	});

	test("lipuga ruutu ei saa avada", [] {
		Minesweeper ms = laud(2, 2, { { 0, 0 } });
		ms.teeKaik(1, 1, true);
		kontrolli(!ms.teeKaik(1, 1, false), "avamine keelatud");
		kontrolli(ms.ruut(1, 1) == Minesweeper::LIPP, "lipp alles");
	});

	test("pommi avamine kaotab ja näitab pommid ning valed lipud", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 }, { 2, 2 } });
		ms.teeKaik(0, 0, true);  // õige lipp
		ms.teeKaik(0, 2, true);  // vale lipp
		ms.teeKaik(2, 2, false); // pomm
		kontrolli(ms.kasMangOnLabi() && ms.kasOliKaotus(), "kaotus");
		kontrolli(ms.ruut(2, 2) == Minesweeper::PLAHVATANUD, "avatud pomm on märgitud");
		kontrolli(ms.ruut(0, 0) == Minesweeper::LIPP, "õige lipp jääb");
		kontrolli(ms.ruut(0, 2) == Minesweeper::VALE_LIPP, "vale lipp märgitakse");
		kontrolli(ms.ruut(1, 1) == Minesweeper::AVAMATA, "ülejäänud ruudud jäävad kinni");
		kontrolli(!ms.teeKaik(1, 0, false), "pärast mängu lõppu käike ei tehta");
	});

	test("kaotusel näidatakse märkimata pommid", [] {
		Minesweeper ms = laud(2, 3, { { 0, 0 }, { 1, 2 } });
		ms.teeKaik(0, 0, false);
		kontrolli(ms.ruut(0, 0) == Minesweeper::PLAHVATANUD, "plahvatanud pomm");
		kontrolli(ms.ruut(1, 2) == Minesweeper::POMM, "teine pomm näha");
	});

	test("number avab ümbruse, kui lippe on õige arv", [] {
		// Pomm (0,0); (1,1) näitab 1
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		ms.teeKaik(0, 1, false); // avab ainult "1"
		kontrolli(ms.ruut(0, 1) == '1', "(0,1) avatud");
		kontrolli(!ms.avaUmber(0, 1), "ilma liputa ei avata midagi");
		ms.teeKaik(0, 0, true);
		kontrolli(ms.avaUmber(0, 1), "lipuga avatakse ümbrus");
		kontrolli(ms.ruut(1, 1) == '1' && ms.ruut(1, 2) == Minesweeper::TUHI, "naabrid avatud");
		kontrolli(ms.kasMangOnLabi() && !ms.kasOliKaotus(), "kõik vabad ruudud avatud = võit");
	});

	test("valesti pandud lipuga ümbruse avamine kaotab", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		ms.teeKaik(0, 1, false);
		ms.teeKaik(0, 2, true); // vale lipp, pomm (0,0) jääb kinni
		kontrolli(ms.avaUmber(0, 1), "ümbrus avati");
		kontrolli(ms.kasOliKaotus(), "pomm avati - kaotus");
		kontrolli(ms.ruut(0, 0) == Minesweeper::PLAHVATANUD, "plahvatanud pomm");
		kontrolli(ms.ruut(0, 2) == Minesweeper::VALE_LIPP, "vale lipp märgitud");
	});

	test("ümbrust ei avata tühjal, suletud ega lipuga ruudul", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		kontrolli(!ms.avaUmber(1, 1), "suletud ruut");
		ms.teeKaik(1, 1, true);
		kontrolli(!ms.avaUmber(1, 1), "lipuga ruut");
		kontrolli(!ms.avaUmber(5, 5), "laua väline ruut");
	});

	test("pommide täpne arv ja piir", [] {
		Minesweeper ms;
		ms.uusMangPommidega(16, 30, 99);
		kontrolli(!ms.kasAlanud(), "pommid pannakse alles esimesel avamisel");
		ms.teeKaik(8, 15, false);
		kontrolli(ms.kasAlanud(), "mäng algas");
		kontrolli(ms.getPommideArv() == 99, "99 pommi");
		Minesweeper tais;
		tais.uusMangPommidega(3, 3, 50);
		kontrolli(tais.getPommideArv() == 8, "vähemalt üks ruut jääb vabaks");
	});

	test("lippude arv", [] {
		Minesweeper ms = laud(3, 3, { { 0, 0 } });
		ms.teeKaik(0, 0, true);
		ms.teeKaik(2, 2, true);
		kontrolli(ms.getLippe() == 2, "kaks lippu");
		ms.teeKaik(2, 2, true);
		kontrolli(ms.getLippe() == 1, "lipu eemaldamine");
	});

	test("laua välised käigud lükatakse tagasi", [] {
		Minesweeper ms = laud(2, 2, { { 0, 0 } });
		kontrolli(!ms.teeKaik(2, 0, false) && !ms.teeKaik(0, -1, true), "väljaspool lauda");
	});

	test("esimene käik pole kunagi pomm ja avab ala, kui ruumi on", [] {
		for (int i = 0; i < 200; i++) {
			Minesweeper ms;
			ms.uusMang(5, 5, 90, false); // 22 pommi 25 ruudust - naabreid ei saa vabaks jätta
			ms.teeKaik(2, 2, false);
			kontrolli(!ms.kasOliKaotus(), "tihe laud: esimene käik ohutu");
		}
		for (int i = 0; i < 200; i++) {
			Minesweeper ms;
			ms.uusMang(5, 5, 50, false); // 12 pommi, 16 kohta väljaspool naabreid
			ms.teeKaik(2, 2, false);
			kontrolli(ms.ruut(2, 2) == Minesweeper::TUHI, "esimene ruut on tühi");
		}
	});

	test("pomme on täpselt protsendi jagu", [] {
		Minesweeper ms;
		ms.uusMang(10, 10, 30, false);
		ms.teeKaik(0, 0, false);
		kontrolli(ms.getPommideArv() == 30, "30 pommi");
	});

	test("suur laud paljude pommidega valmib (vana rand() jäi siin lõputult tööle)", [] {
		Minesweeper ms;
		ms.uusMang(Minesweeper::MAX_KORGUS, Minesweeper::MAX_LAIUS, 60, false);
		ms.teeKaik(0, 0, false);
		kontrolli(ms.getPommideArv() == 307200u * 60 / 100, "184320 pommi");
	});

	test("loeTaisarv küsib uuesti ja lõpetab sisendi lõpus", [] {
		std::istringstream sisend("abc\n0\n42\n7\n");
		std::streambuf* vana = std::cin.rdbuf(sisend.rdbuf());
		const int arv = loeTaisarv("? ", 1, 9);
		kontrolli(arv == 7, "tähed ja vahemikust välja jäävad arvud jäetakse vahele");
		bool visati = false;
		try {
			loeTaisarv("? ", 1, 9);
		}
		catch (const SisendLopetatud&) {
			visati = true;
		}
		kontrolli(visati, "sisendi lõpus visatakse SisendLopetatud");
		std::cin.rdbuf(vana);
		std::cin.clear();
	});

	std::cout << '\n' << (kokku - ebaonnestunud) << "/" << kokku << " kontrolli õnnestus\n";
	return ebaonnestunud == 0 ? 0 : 1;
}
