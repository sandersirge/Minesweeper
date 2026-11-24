#pragma once
#include <SFML/Graphics/Color.hpp>

// Graafilise liidese mõõdud ja värvid. Kõik on "loogilistes pikslites": aken näitab neid
// täisarvulise suurendusega, nii et piksel-graafika jääb teravaks.
namespace teema {

inline constexpr int RUUT = 16;        // ühe ruudu suurus
inline constexpr int RIBA = 24;        // mänguvaate ülemise riba kõrgus
inline constexpr int MIN_LAIUS = 160;  // mänguvaate minimaalne laius (riba mahub ära)
inline constexpr int MAX_SUURENDUS = 4;

inline constexpr sf::Color TAUST{ 8, 16, 48 };          // ruutudevahelised jooned, akna taust
inline constexpr sf::Color RIBA_TAUST{ 14, 28, 82 };
inline constexpr sf::Color SULETUD{ 16, 80, 240 };
inline constexpr sf::Color SULETUD_HELE{ 70, 132, 255 };  // suletud ruudu ülemine/vasak serv
inline constexpr sf::Color SULETUD_TUME{ 8, 46, 172 };    // suletud ruudu alumine/parem serv
inline constexpr sf::Color HOVER{ 48, 114, 255 };
inline constexpr sf::Color VAJUTATUD{ 10, 54, 200 };
inline constexpr sf::Color AVATUD{ 150, 186, 255 };
inline constexpr sf::Color PLAHVATUS{ 228, 46, 32 };
inline constexpr sf::Color RIST{ 228, 30, 30 };           // vale lipu rist

inline constexpr sf::Color TEKST{ 236, 242, 255 };
inline constexpr sf::Color TEKST_TUHM{ 132, 160, 222 };
inline constexpr sf::Color VARI{ 4, 10, 32 };

inline constexpr sf::Color NUPP{ 26, 62, 172 };
inline constexpr sf::Color NUPP_HOVER{ 44, 94, 222 };
inline constexpr sf::Color NUPP_VAJUTATUD{ 12, 40, 132 };
inline constexpr sf::Color NUPP_VALITUD{ 72, 132, 255 };
inline constexpr sf::Color NUPP_SERV{ 4, 14, 56 };

inline constexpr sf::Color LAINE_VOIT{ 255, 255, 255 };
inline constexpr sf::Color LAINE_KAOTUS{ 255, 52, 24 };
inline constexpr sf::Color TEADE_TAUST{ 0, 0, 0, 176 };

// Numbrite värvid 1...8 (indeks 0 jääb kasutamata)
inline constexpr sf::Color NUMBRID[9] = {
	{ 0, 0, 0 },     { 18, 44, 168 }, { 14, 118, 40 }, { 196, 24, 24 }, { 70, 10, 140 },
	{ 132, 36, 0 },  { 0, 112, 124 }, { 24, 24, 24 },  { 92, 92, 104 },
};

} // namespace teema
