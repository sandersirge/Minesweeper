# Kirjeldus
See grupitöö on kloon teadatuntud mängust *Minesweeper*, kasutades loogikaks C++ programmeerimiskeelt ning SFML teeki graafika jaoks.<br /><br />
<img width="498" height="417" alt="20260509_16h07m59s_grim" src="https://github.com/user-attachments/assets/81f23ec5-67b7-48a9-ab8d-bc5fdde7edcf" /><br />


# Funktsionaalsused
 - Saab valida mängulaua suuruse (kõrguse ja laiuse eraldi)
 - Saab valida, mitu protsenti ruutudest on pommid
 - Saab valida terminali ja graafilise liidese vahel

# Projekti ülesehitus
 .<br />
 ├─ Pildid<br />
 │    └─ *.png - Kõik pildifailid mängulaua ruutude jaoks.<br />
 ├─ main.cpp - Loob mängu objekti ja alustab mängutsüklit.<br />
 ├─ Minesweeper.cpp<br />
 ├─ Minesweeper.h - Mängu loogika.<br />
 ├─ Makefile - Kompileerib mängu ja teeb jooksutatava faili *rungame* (muutmise ja kasutamise juhend allpool).<br />
 └─ README.md

# Mängu kasutamise juhend
## Linux
Üldise info SFML projekti kasutamise kohta leiab ametlikust vikist (https://www.sfml-dev.org/tutorials/3.0/getting-started/linux/#compiling-an-sfml-program) <br />
Mängule on loodud ka *Makefile*, et automaatselt kompileerida ja linkida.<br />
```shell
make
./rungame
```
#### Mängu jooksutamiseks on vaja järgmisi pakette:<br />
 - SFML, versioon 3 (lihtsaim on kasutada enda süsteemi paketihaldurit)<br />
 - C++ keele kompilaator (Vaikimisi kasutatakse g++, endale sobiva kompilaatori kasutamiseks muuta failis "Makefile" muutujat "COMPILER")<br />
 - make (käsk ei pruugi kõikide süsteemidega kaasa tulla)<br />
#### Näidis programmi kasutamisest:<br />
```shell
git clone https://github.com/MartinMaenniste/cppRyhmatoo.git
cd cppRyhmatoo

make
./rungame
```
Ubuntu/Debian (ka WSL) paketid:
```shell
sudo apt install build-essential cmake ninja-build pkg-config libsfml-dev
```

## CMake (Linux, WSL, Windows)
```shell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && ./rungame
```
CMake kasutab paigaldatud SFML 3-e; kui seda pole, laeb ja kompileerib ta selle ise (`-DMINESWEEPER_FETCH_SFML=OFF` keelab). Pildid kopeeritakse käivitatava faili kõrvale, seega käivita mäng `build` kaustast (CLionis: *Working directory* = build-kaust).

## Windows (MSYS2)
Testitud MSYS2 UCRT64 keskkonnas (GCC, SFML 3.1).
1. Paigalda MSYS2: `winget install MSYS2.MSYS2`
2. Ava **MSYS2 UCRT64** terminal ja paigalda tööriistad:
   ```shell
   pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,gdb,cmake,ninja,pkgconf,sfml} make
   ```
3. Kompileeri samas terminalis kas `make` või ülaltoodud CMake käskudega.
4. Väljaspool MSYS2 terminali käivitamiseks lisa `C:\msys64\ucrt64\bin` PATH-i (seal on vajalikud DLL-id).

CLionis vali *Settings → Build, Execution, Deployment → Toolchains* alt MinGW tööriistakomplekt kaustaga `C:\msys64\ucrt64`.<br />
Visual Studio puhul aitab ametlik vikileht - https://www.sfml-dev.org/tutorials/3.0/getting-started/visual-studio/
