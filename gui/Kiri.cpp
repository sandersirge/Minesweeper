#include "Kiri.hpp"

#include <array>
#include <cmath>
#include <map>

namespace kiri {
namespace {

using Kuju = std::array<const char*, TAHE_KORGUS>;

// Tähtede kujud: '#' on täidetud piksel. Kõik joonistatud selle projekti jaoks.
const std::map<char32_t, Kuju>& tahed() {
	static const std::map<char32_t, Kuju> tabel = {
		{ U'A', { ".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#" } },
		{ U'B', { "####.", "#...#", "#...#", "####.", "#...#", "#...#", "####." } },
		{ U'C', { ".###.", "#...#", "#....", "#....", "#....", "#...#", ".###." } },
		{ U'D', { "####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####." } },
		{ U'E', { "#####", "#....", "#....", "####.", "#....", "#....", "#####" } },
		{ U'F', { "#####", "#....", "#....", "####.", "#....", "#....", "#...." } },
		{ U'G', { ".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".###." } },
		{ U'H', { "#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#" } },
		{ U'I', { ".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###." } },
		{ U'J', { "..###", "...#.", "...#.", "...#.", "#..#.", "#..#.", ".##.." } },
		{ U'K', { "#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#" } },
		{ U'L', { "#....", "#....", "#....", "#....", "#....", "#....", "#####" } },
		{ U'M', { "#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#" } },
		{ U'N', { "#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#" } },
		{ U'O', { ".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###." } },
		{ U'P', { "####.", "#...#", "#...#", "####.", "#....", "#....", "#...." } },
		{ U'Q', { ".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#" } },
		{ U'R', { "####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#" } },
		{ U'S', { ".####", "#....", "#....", ".###.", "....#", "....#", "####." } },
		{ U'T', { "#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.." } },
		{ U'U', { "#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###." } },
		{ U'V', { "#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.." } },
		{ U'W', { "#...#", "#...#", "#...#", "#.#.#", "#.#.#", "#.#.#", ".#.#." } },
		{ U'X', { "#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#" } },
		{ U'Y', { "#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.." } },
		{ U'Z', { "#####", "....#", "...#.", "..#..", ".#...", "#....", "#####" } },
		{ U'0', { ".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###." } },
		{ U'1', { "..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###." } },
		{ U'2', { ".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####" } },
		{ U'3', { "####.", "....#", "....#", ".###.", "....#", "....#", "####." } },
		{ U'4', { "...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#." } },
		{ U'5', { "#####", "#....", "####.", "....#", "....#", "#...#", ".###." } },
		{ U'6', { ".###.", "#....", "#....", "####.", "#...#", "#...#", ".###." } },
		{ U'7', { "#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..." } },
		{ U'8', { ".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###." } },
		{ U'9', { ".###.", "#...#", "#...#", ".####", "....#", "....#", ".###." } },
		{ U' ', { ".....", ".....", ".....", ".....", ".....", ".....", "....." } },
		{ U':', { ".....", "..#..", "..#..", ".....", "..#..", "..#..", "....." } },
		{ U'!', { "..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.." } },
		{ U'?', { ".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.." } },
		{ U'-', { ".....", ".....", ".....", "#####", ".....", ".....", "....." } },
		{ U'+', { ".....", "..#..", "..#..", "#####", "..#..", "..#..", "....." } },
		{ U'.', { ".....", ".....", ".....", ".....", ".....", ".....", "..#.." } },
		{ U',', { ".....", ".....", ".....", ".....", ".....", "..#..", ".#..." } },
		{ U'/', { "....#", "....#", "...#.", "..#..", ".#...", "#....", "#...." } },
		{ U'(', { "...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#." } },
		{ U')', { ".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..." } },
		{ U'<', { "...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#." } },
		{ U'>', { ".#...", "..#..", "...#.", "....#", "...#.", "..#..", ".#..." } },
		{ U'×', { ".....", ".....", "#...#", ".#.#.", "..#..", ".#.#.", "#...#" } }, // ×
		{ U'·', { ".....", ".....", ".....", "..#..", ".....", ".....", "....." } }, // ·
	};
	return tabel;
}

enum class Mark { Puudub, Taepid, Tilde };

struct Taht {
	const Kuju* kuju = nullptr;
	Mark mark = Mark::Puudub;
};

Taht leiaTaht(char32_t m) {
	Mark mark = Mark::Puudub;
	switch (m) {
	case U'x': m = U'×'; break; // "9x9" -> 9×9
	case U'ä': case U'Ä': m = U'A'; mark = Mark::Taepid; break; // ä Ä
	case U'ö': case U'Ö': m = U'O'; mark = Mark::Taepid; break; // ö Ö
	case U'ü': case U'Ü': m = U'U'; mark = Mark::Taepid; break; // ü Ü
	case U'õ': case U'Õ': m = U'O'; mark = Mark::Tilde; break;  // õ Õ
	default:
		if (m >= U'a' && m <= U'z') m = m - U'a' + U'A';
	}
	const auto& tabel = tahed();
	auto leitud = tabel.find(m);
	if (leitud == tabel.end()) leitud = tabel.find(U'?');
	return { &leitud->second, mark };
}

// Loeb UTF-8 tekstist järgmise märgi ja liigub edasi
char32_t jargmine(std::string_view tekst, size_t& i) {
	const auto b = static_cast<unsigned char>(tekst[i]);
	int pikkus = 1;
	char32_t m = b;
	if (b >= 0xF0) { pikkus = 4; m = b & 0x07; }
	else if (b >= 0xE0) { pikkus = 3; m = b & 0x0F; }
	else if (b >= 0xC0) { pikkus = 2; m = b & 0x1F; }
	for (int k = 1; k < pikkus && i + k < tekst.size(); k++) {
		m = (m << 6) | (static_cast<unsigned char>(tekst[i + k]) & 0x3F);
	}
	i += pikkus;
	return m;
}

size_t markideArv(std::string_view tekst) {
	size_t arv = 0;
	for (size_t i = 0; i < tekst.size();) {
		jargmine(tekst, i);
		arv++;
	}
	return arv;
}

} // namespace

float laius(std::string_view tekst, float suurendus) {
	const size_t arv = markideArv(tekst);
	if (arv == 0) return 0.f;
	return (static_cast<float>(arv) * (TAHE_LAIUS + VAHE) - VAHE) * suurendus;
}

void kirjuta(Louend& louend, std::string_view tekst, float x, float y, sf::Color varv, float s) {
	for (size_t i = 0; i < tekst.size();) {
		const Taht taht = leiaTaht(jargmine(tekst, i));
		for (int r = 0; r < TAHE_KORGUS; r++) {
			for (int v = 0; v < TAHE_LAIUS; v++) {
				if ((*taht.kuju)[r][v] == '#') louend.piksel(x + v * s, y + r * s, varv, s);
			}
		}
		if (taht.mark == Mark::Taepid) {
			louend.piksel(x + 1 * s, y - 2 * s, varv, s);
			louend.piksel(x + 3 * s, y - 2 * s, varv, s);
		}
		else if (taht.mark == Mark::Tilde) {
			louend.piksel(x + 1 * s, y - 3 * s, varv, s);
			louend.piksel(x + 4 * s, y - 3 * s, varv, s);
			louend.piksel(x + 0 * s, y - 2 * s, varv, s);
			louend.piksel(x + 2 * s, y - 2 * s, varv, s);
			louend.piksel(x + 3 * s, y - 2 * s, varv, s);
		}
		x += (TAHE_LAIUS + VAHE) * s;
	}
}

void kirjutaKeskele(Louend& louend, std::string_view tekst, float keskX, float y, sf::Color varv,
                    float s) {
	// Täisarvuline asukoht, et pikslid ei jääks poole piksli peale (udune tekst)
	kirjuta(louend, tekst, std::floor(keskX - laius(tekst, s) / 2.f), y, varv, s);
}

void kirjutaVarjuga(Louend& louend, std::string_view tekst, float x, float y, sf::Color varv,
                    sf::Color vari, float s) {
	kirjuta(louend, tekst, x + s, y + s, vari, s);
	kirjuta(louend, tekst, x, y, varv, s);
}

} // namespace kiri
