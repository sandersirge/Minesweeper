#include "Ikoonid.hpp"
#include "Teema.hpp"

#include <array>
#include <string>

namespace ikoon {
namespace {

using Kaart = std::array<std::string, SUURUS>;

// Värvid kaartide tähtede jaoks ('.' on läbipaistev)
sf::Color varv(char c) {
	switch (c) {
	case 'K': return { 20, 20, 28 };     // pommi keha, näo piirjoon
	case 'H': return { 236, 236, 246 };  // pommi läige
	case 'R': return { 232, 36, 36 };    // lipp
	case 'r': return { 156, 16, 22 };    // lipu vari
	case 'Y': return { 255, 214, 48 };   // nägu
	case 'y': return { 220, 162, 20 };   // näo vari
	case 'W': return { 255, 246, 196 };  // näo läige
	case 'B': return { 24, 18, 18 };     // silmad, suu
	case 'G': return { 40, 40, 64 };     // päikeseprillid
	default: return sf::Color::Transparent;
	}
}

const Kaart LIPP = {
	"................",
	"................",
	"......RRR#......",
	"....RRRRR#......",
	"..RRRRRRR#......",
	"...rrRRRR#......",
	"......rrr#......",
	".........#......",
	".........#......",
	".........#......",
	".........#......",
	"......#####.....",
	".....#######....",
	"................",
	"................",
	"................",
};

const Kaart POMM = {
	"................",
	".......##.......",
	".......##.......",
	"...#..####..#...",
	"....########....",
	"...##HH######...",
	"...##HH######...",
	".##############.",
	".##############.",
	"...##########...",
	"...##########...",
	"....########....",
	"...#..####..#...",
	".......##.......",
	".......##.......",
	"................",
};

// Näo põhi; silmad ja suu lisatakse oleku järgi
const Kaart NAGU = {
	".....KKKKKK.....",
	"...KKYYYYYYKK...",
	"..KYYWWYYYYYYK..",
	".KYYWWYYYYYYYyK.",
	".KYYYYYYYYYYYyK.",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	"KYYYYYYYYYYYYYyK",
	".KYYYYYYYYYYYyyK",
	".KyYYYYYYYYYyyK.",
	"..KyyYYYYYYyyK..",
	"...KKyyyyyyKK...",
	".....KKKKKK.....",
};

// Kaardi '#' märgid saavad antud värvi, teised tähed värvid tabelist
void joonistaKaart(Louend& louend, const Kaart& kaart, float x, float y, float s, char trellid) {
	for (int r = 0; r < SUURUS; r++) {
		for (int v = 0; v < SUURUS; v++) {
			const char c = kaart[r][v] == '#' ? trellid : kaart[r][v];
			const sf::Color cv = varv(c);
			if (cv.a != 0) louend.piksel(x + v * s, y + r * s, cv, s);
		}
	}
}

Kaart naoKaart(Nagu nagu) {
	Kaart k = NAGU;
	const auto paneB = [&](int v, int r, char c = 'B') { k[r][v] = c; };
	switch (nagu) {
	case Nagu::Tavaline: // naeratus
		paneB(5, 5); paneB(5, 6); paneB(10, 5); paneB(10, 6);
		paneB(4, 10); paneB(11, 10);
		for (int v = 5; v <= 10; v++) paneB(v, 11);
		break;
	case Nagu::Vajutab: // üllatunud "O"
		paneB(5, 5); paneB(5, 6); paneB(10, 5); paneB(10, 6);
		paneB(7, 9); paneB(8, 9); paneB(6, 10); paneB(9, 10);
		paneB(6, 11); paneB(9, 11); paneB(7, 12); paneB(8, 12);
		break;
	case Nagu::Kaotus: // X-silmad ja kurb suu
		for (int d = 0; d < 3; d++) {
			paneB(4 + d, 4 + d); paneB(6 - d, 4 + d);
			paneB(9 + d, 4 + d); paneB(11 - d, 4 + d);
		}
		paneB(4, 12); paneB(11, 12);
		for (int v = 5; v <= 10; v++) paneB(v, 11);
		break;
	case Nagu::Voit: // päikeseprillid ja lai naeratus
		for (int v = 3; v <= 12; v++) paneB(v, 5, 'G');
		for (int v = 3; v <= 6; v++) paneB(v, 6, 'G');
		for (int v = 9; v <= 12; v++) paneB(v, 6, 'G');
		paneB(4, 7, 'G'); paneB(5, 7, 'G'); paneB(10, 7, 'G'); paneB(11, 7, 'G');
		paneB(4, 6, 'H'); paneB(9, 6, 'H');
		paneB(3, 10); paneB(12, 10);
		for (int v = 4; v <= 11; v++) paneB(v, 11);
		for (int v = 6; v <= 9; v++) paneB(v, 12);
		break;
	}
	return k;
}

} // namespace

void lipp(Louend& louend, float x, float y, float s) { joonistaKaart(louend, LIPP, x, y, s, 'K'); }

void pomm(Louend& louend, float x, float y, float s) { joonistaKaart(louend, POMM, x, y, s, 'K'); }

void rist(Louend& louend, float x, float y, float s) {
	for (int i = 2; i <= 13; i++) {
		louend.ristkylik(x + i * s, y + i * s, 2 * s, s, teema::RIST);
		louend.ristkylik(x + (14 - i) * s, y + i * s, 2 * s, s, teema::RIST);
	}
}

void nagu(Louend& louend, Nagu n, float x, float y, float s) {
	joonistaKaart(louend, naoKaart(n), x, y, s, 'K');
}

sf::Image naguPilt(Nagu n, unsigned s) {
	const Kaart kaart = naoKaart(n);
	sf::Image pilt({ SUURUS * s, SUURUS * s }, sf::Color::Transparent);
	for (unsigned r = 0; r < SUURUS * s; r++) {
		for (unsigned v = 0; v < SUURUS * s; v++) {
			pilt.setPixel({ v, r }, varv(kaart[r / s][v / s]));
		}
	}
	return pilt;
}

} // namespace ikoon
