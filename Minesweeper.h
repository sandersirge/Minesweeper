#pragma once
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

// Mängu loogika. Ei sõltu SFML-ist: graafiline liides (gui/) ja arhiveeritud klassikaline
// versioon (archive/klassikaline/) joonistavad laua ruut() abil.

// Visatakse, kui sisend lõpeb (Ctrl+D / Ctrl+Z või suunatud sisendi lõpp), et mäng saaks
// korrektselt sulguda, selle asemel et küsida sama küsimust lõputult.
struct SisendLopetatud : std::runtime_error {
	SisendLopetatud() : std::runtime_error("Sisend lõppes") {}
};

// Küsib täisarvu vahemikus [min, max], kuni saab sobiva. Vigane sisend (nt tähed) visatakse ära
// ja küsitakse uuesti; sisendi lõpus visatakse SisendLopetatud.
int loeTaisarv(const std::string& kusimus, int min, int max);

class Minesweeper {
public:
	// Mängulaua ruudud, nagu mängija neid näeb
	static constexpr char AVAMATA = '?';     // mängija pole veel avanud
	static constexpr char TUHI = ' ';        // avatud, ühtegi pommi kõrval pole
	static constexpr char LIPP = '!';        // mängija pandud lipp
	static constexpr char POMM = 'X';        // pomm (näidatakse kaotuse järel)
	static constexpr char PLAHVATANUD = '*'; // pomm, mille mängija avas
	static constexpr char VALE_LIPP = 'V';   // lipp kohas, kus pommi polnud (kaotuse järel)
	// '1'...'8' - avatud ruut, number näitab, mitu pommi on ümberringi

	// Terminali versiooni piirid: aken on 640x480 pikslit ja iga ruut vähemalt 1 piksel
	static constexpr int MAX_KORGUS = 480;
	static constexpr int MAX_LAIUS = 640;

	// Küsib terminalis mängulaua andmed ja alustab uut mängu.
	void kusiManguLauaAndmed();
	// Alustab uut mängu, pommide arv protsendina. Pommid paigutatakse esimese avamise ajal, nii
	// et esimene käik on alati ohutu.
	void uusMang(int korgus, int laius, int protsent, bool graafiline);
	// Sama, kuid pommide täpse arvuga (graafilise liidese raskusastmed). Arv piiratakse nii, et
	// vähemalt üks ruut jääb vabaks.
	void uusMangPommidega(int korgus, int laius, size_t pommid);
	// Paneb pommid kohe kindlatele kohtadele (testimiseks); esimese käigu kaitset siis pole.
	void seaPommid(const std::vector<bool>& pommiKohad);
	// Kas mängu teated (nt "See ruut on juba avatud.") prinditakse terminali. Graafiline
	// liides lülitab need välja.
	void naitaTeateid(bool naita) { teated = naita; }

	// Avab ruudu või paneb/võtab lipu. Tagastab false, kui käik polnud lubatud.
	bool teeKaik(int rida, int veerg, bool kasLipp);
	// "Akord": kui avatud numbri ümber on sama palju lippe kui number näitab, avatakse kõik
	// ülejäänud naabrid (valesti pandud lipu korral võib see pommi avada). Tagastab, kas midagi
	// avati.
	bool avaUmber(int rida, int veerg);
	void kaikTerminalis();
	void prindiMangulaud() const;

	char ruut(int rida, int veerg) const;
	int getKorgus() const { return korgus; }
	int getLaius() const { return laius; }
	size_t getPommideArv() const { return pommideArv; }
	size_t getLippe() const;
	bool kasAlanud() const { return pommidPaigutatud; } // esimene ruut on avatud
	bool kasMangOnLabi() const { return mangOnLabi; }
	bool kasOliKaotus() const { return oliKaotus; }
	bool getKuvaGraafika() const { return kuvaGraafiliselt; }

private:
	bool mangOnLabi{ false }, oliKaotus{ false }, kuvaGraafiliselt{ false };
	bool pommidPaigutatud{ false };
	bool teated{ true };
	int korgus{ 0 };
	int laius{ 0 };
	size_t vektoritePikkus{ 0 };
	size_t pommideArv{ 0 };
	size_t avatudRuute{ 0 };

	// Maatriksid on ühe pika jadana: ruut (rida, veerg) on indeksil rida * laius + veerg
	std::vector<bool> pommid;      // pommide asetused
	std::vector<int> ruudud;       // mitu pommi on iga ruudu ümber (pommi enda kohal -1)
	std::vector<char> mangijaLaud; // mängulaud, mida mängija näeb (vt konstante ülal)

	// Kuni 8 naabri indeksid ilma mälueralduseta (suurel laual kutsutakse seda sadu tuhandeid kordi)
	struct Naabrid {
		std::array<size_t, 8> indeksid{};
		size_t arv{ 0 };
		const size_t* begin() const { return indeksid.data(); }
		const size_t* end() const { return indeksid.data() + arv; }
		size_t size() const { return arv; }
	};
	Naabrid naabrid(size_t indeks) const;
	void teade(const char* tekst) const;
	void paigutaPommid(size_t ohutuIndeks);
	void arvutaRuudud();
	// Avab ruudu (ja tühja ala). Tagastab false, kui avati pomm.
	bool avaRuut(size_t indeks);
	void avaRuudud(size_t algusIndeks);
	void kontrolliVoitu();
	void kaotus(size_t plahvatanudIndeks);
};
