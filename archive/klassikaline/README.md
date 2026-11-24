# Klassikaline versioon (arhiiv)

Mängu esimene graafiline versioon: mängulaua andmed küsitakse terminalis ning mängitakse kas
terminalis või lihtsas aknas `Pildid/` kausta piltidega.

<img width="498" height="417" alt="Klassikaline versioon" src="https://github.com/user-attachments/assets/81f23ec5-67b7-48a9-ab8d-bc5fdde7edcf" />

Uus graafiline liides (menüü, raskusastmed, piksel-graafika) on projekti juurkaustas (`gui/`).
Mõlemad kasutavad sama mänguloogikat (`Minesweeper.h`, `Minesweeper.cpp`).

Ehitamine (projekti juurkaustast):
```shell
cmake -S . -B build -G Ninja
cmake --build build --target rungame_klassikaline
cd build && ./rungame_klassikaline
```
`Pildid` kopeeritakse käivitatava faili kõrvale; käivita see `build` kaustast.
