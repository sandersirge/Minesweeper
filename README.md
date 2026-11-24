# Kirjeldus
See grupitöö on kloon teadatuntud mängust *Minesweeper*, kasutades loogikaks C++ programmeerimiskeelt ning SFML teeki graafika jaoks.<br /><br />
<img width="700" alt="Menüü, mäng ja kaotus" src="docs/ekraanipilt.png" /><br />

Graafiline stiil (sinine piksel-ruudustik, reageeriv nägu, laine mängu lõpus) on inspireeritud projektist [Kofybrek/Minesweeper](https://github.com/Kofybrek/Minesweeper); kood ja piksel-graafika on selle projekti omad. Esimene versioon (terminal ja pildifailidega aken) on kaustas [archive/classic](archive/classic).

# Funktsionaalsused
 - Algusmenüü: raskusaste (algaja 9×9 · 10, keskmine 16×16 · 40, ekspert 16×30 · 99) või kohandatud kõrgus, laius ja pommide arv
 - Vasak klõps avab ruudu, parem klõps paneb või võtab lipu
 - Avatud numbril vasak (või keskmine) klõps avab ümbruse, kui ümber on sama palju lippe kui number näitab
 - Esimene käik ei ole kunagi pomm (kui ruumi on, on ka selle ümbrus vaba)
 - Tühja ruudu avamine avab kogu tühja ala koos ümbritsevate numbritega
 - Võidad, kui kõik pommita ruudud on avatud (lippe ei pea panema)
 - Ülemisel ribal: märkimata pommid, nägu (klõps = uus mäng) ja aeg
 - Mängu lõpus levib laine (kaotusel punane, võidul valge); kaotusel näidatakse pomme ja valesid lippe
 - Klahvid: Enter / F2 uus mäng, Esc menüüsse (menüüs: 1-3 raskusaste, Enter alusta, Esc välju)
 - Aken suurendab piksel-graafikat täisarvu korda, nii et see on ka suurel ekraanil terav

# Projekti ülesehitus
 .<br />
 ├─ gui<br />
 │    ├─ App.* - Aken, menüü ja mängu vahetamine.<br />
 │    ├─ MenuScreen.* - Algusmenüü (raskusaste, kohandatud laud).<br />
 │    ├─ GameScreen.* - Mängulaud, ülemine riba, laine ja lõputeade.<br />
 │    ├─ PixelFont.* - 5x7 piksel-kiri (ka Õ Ä Ö Ü).<br />
 │    ├─ Icons.* - Lipp, pomm ja nägu piksel-graafikana.<br />
 │    └─ Canvas.*, Screen.*, Theme.hpp - Joonistamine, nupud, värvid.<br />
 ├─ archive/classic - Esimene versioon (terminal ja Pildid/ kausta piltidega aken).<br />
 ├─ tests<br />
 │    └─ MinesweeperTest.cpp - Mänguloogika testid (aknata).<br />
 ├─ main.cpp - Käivitab graafilise rakenduse.<br />
 ├─ Minesweeper.cpp<br />
 ├─ Minesweeper.h - Mängu loogika (ei sõltu SFML-ist).<br />
 ├─ .clang-format - Koodi vormindus (CLion: Code → Reformat Code).<br />
 ├─ CMakeLists.txt - CMake'i ehitus (mäng, arhiveeritud versioon, testid).<br />
 ├─ Makefile - Kompileerib mängu *rungame* ja testid (muutmise ja kasutamise juhend allpool).<br />
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
Testid: `ctest --test-dir build` (või `make test`).<br />
CMake kasutab paigaldatud SFML 3-e; kui seda pole, laeb ja kompileerib ta selle ise (`-DMINESWEEPER_FETCH_SFML=OFF` keelab). Mäng ei vaja pildifaile, kogu graafika joonistatakse koodis. Arhiveeritud versioon ehitatakse sihtmärgina `rungame_classic` (`-DMINESWEEPER_BUILD_ARCHIVE=OFF` jätab selle välja).

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
