#pragma once
#include <array>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

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
	// Mängulaua ruudud, nagu mängija neid näeb (mangijaLaud vektoris)
	static constexpr char AVAMATA = '?'; // mängija pole veel avanud
	static constexpr char TUHI = ' ';    // avatud, ühtegi pommi kõrval pole
	static constexpr char LIPP = '!';    // mängija pandud lipp
	static constexpr char POMM = 'X';    // pomm (näidatakse mängu lõpus)
	// '1'...'8' - avatud ruut, number näitab, mitu pommi on ümberringi

	// Mängulaua suurus: aken on 640x480 pikslit ja iga ruut peab olema vähemalt 1 piksel
	static constexpr int MAX_KORGUS = 480;
	static constexpr int MAX_LAIUS = 640;

	// Laeb ruutude pildid kaustast. Kui mõni pilt ei laadi, visatakse selle faili nimi (std::string).
	void laeTekstuurid(const std::string& kaust);

	// Küsib terminalis mängulaua andmed ja alustab uut mängu.
	void kusiManguLauaAndmed();
	// Alustab uut mängu. Pommid paigutatakse esimese avamise ajal, nii et esimene käik on alati ohutu.
	void uusMang(int korgus, int laius, int protsent, bool graafiline);
	// Paneb pommid kohe kindlatele kohtadele (testimiseks); esimese käigu kaitset siis pole.
	void seaPommid(const std::vector<bool>& pommiKohad);

	// Avab ruudu või paneb/võtab lipu. Tagastab false, kui käik polnud lubatud (põhjus prinditakse).
	bool teeKaik(int rida, int veerg, bool kasLipp);
	void kaikTerminalis();
	// Hiireklõps: vasak nupp avab, parem paneb lipu. Asukoht on akna pikslites.
	void handleEvent(const sf::RenderWindow& aken, sf::Mouse::Button nupp, sf::Vector2i asukoht);

	void prindiMangulaud() const;
	void kuvaMangulaud(sf::RenderWindow& aken) const;

	char ruut(int rida, int veerg) const;
	int getKorgus() const { return korgus; }
	int getLaius() const { return laius; }
	size_t getPommideArv() const { return pommideArv; }
	bool kasMangOnLabi() const { return mangOnLabi; }
	bool kasOliKaotus() const { return oliKaotus; }
	bool getKuvaGraafika() const { return kuvaGraafiliselt; }

private:
	// Pildid eraldi objektis, mis luuakse alles laeTekstuurid() ajal: sf::Texture vajab OpenGL-i
	// konteksti, mängu loogika (ja testid) ei vaja.
	struct Tekstuurid;
	std::shared_ptr<Tekstuurid> tekstuurid;

	bool mangOnLabi{ false }, oliKaotus{ false }, kuvaGraafiliselt{ false };
	bool pommidPaigutatud{ false };
	int korgus{ 0 };
	int laius{ 0 };
	int protsent{ 0 };
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
	void paigutaPommid(size_t ohutuIndeks);
	void arvutaRuudud();
	void avaRuudud(size_t algusIndeks);
	void reedaMangulaud();
};
