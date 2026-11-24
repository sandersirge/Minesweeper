#pragma once
#include "Louend.hpp"

#include <string_view>

// Väike 5x7 piksel-kiri (suurtähed, numbrid, Õ Ä Ö Ü ja mõned märgid). Tekst on UTF-8.
// Täppide ja tilde jaoks on tähe kohal 3 pikslit ruumi.
namespace kiri {

inline constexpr int TAHE_LAIUS = 5;
inline constexpr int TAHE_KORGUS = 7;
inline constexpr int VAHE = 1;

// Teksti laius pikslites antud suurendusega
float laius(std::string_view tekst, float suurendus = 1.f);
// (x, y) on esimese tähe ülemine vasak nurk
void kirjuta(Louend& louend, std::string_view tekst, float x, float y, sf::Color varv,
             float suurendus = 1.f);
void kirjutaKeskele(Louend& louend, std::string_view tekst, float keskX, float y, sf::Color varv,
                    float suurendus = 1.f);
// Tekst ühe piksli suuruse varjuga (paremal all), et see paistaks igal taustal
void kirjutaVarjuga(Louend& louend, std::string_view tekst, float x, float y, sf::Color varv,
                    sf::Color vari, float suurendus = 1.f);

} // namespace kiri
