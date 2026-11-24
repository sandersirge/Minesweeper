#pragma once
#include "Louend.hpp"

// 16x16 piksel-ikoonid (lipp, pomm, nägu), joonistatud selle projekti jaoks.
namespace ikoon {

inline constexpr int SUURUS = 16;

enum class Nagu { Tavaline, Vajutab, Kaotus, Voit };

void lipp(Louend& louend, float x, float y, float suurendus = 1.f);
void pomm(Louend& louend, float x, float y, float suurendus = 1.f);
void rist(Louend& louend, float x, float y, float suurendus = 1.f); // vale lipu märk
void nagu(Louend& louend, Nagu nagu, float x, float y, float suurendus = 1.f);
// Akna ikoon (nägu) täisarvulise suurendusega
sf::Image naguPilt(Nagu nagu, unsigned suurendus);

} // namespace ikoon
