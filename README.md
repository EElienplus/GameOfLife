# Hra zivota - Portfolio Verze

Strucny popis
-------------
Interaktivni implementace Conwayovy Hry zivota s vizualnimi efekty (bloom, gloom), trasovymi stopy a mnoha preddefinovanymi vzory. Navrzeno pro prehlizeni a experimentovani s mrizkou.

Ovladani
--------
- Space: spustit / pozastavit
- S: provest jeden krok
- R: nahodne naplneni
- C: vymazat
- G: prepnout mrizku
- T: zmenit tema
- `[` / `]`: snizit / zvysit cilovou rychlost
- Ctrl/Cmd + scroll: zmena rychlosti
- F: prepnout stopy (trails)
- B: prepnout bloom efekt
- V: prepnout gloom (tmavy prekrivac)
- K / L: snizit / zvysit intenzitu bloom
- - / = nebo +: zmena velikosti bunek
- P: screenshot (ulozi PNG)
- E: export aktualniho vzoru do `pattern_export.txt`
- I: import vzoru z `pattern_export.txt` (vlozi do stredu obrazovky)
- F1: zobrazit zkratky (panel)
- F2: zobrazit statistiky (panel)
- Mys: levy tlacitko kresleni/zapnuti bunek, pravy tlacitko mazani, stredni tlacitko posun kamery
- Scroll bez modifieru: zoom okolo kurzoru

Vzory (klavesy 1..0)
--------------------
Cisla 1 az 0 vlozi preddefinovane vzory do stredu aktualniho pohledu. Mezi dostupnymi vzory jsou:
1. Glider
2. Diehard
3. Pulsar
4. Battleship
5. R-Pentomino
6. LWSS
7. Toad
8. Beacon
9. Beehive
0. Gosper Gun

Funkce
------
- Trails: zhasinajici stopy po zivech bunkach
- Bloom: separabilni blur pro svetelny efekt
- Gloom: jemny tmavy prekrivac a vignette
- Export / import jednoduchych vzoru do textoveho souboru
- Nastavitelna velikost bunek, zoom a posun kamery

Kompilace a spusteni (macOS / CLion)
-----------------------------------
1. Nainstalujte zavislosti:
   - Homebrew: `brew install raylib`
2. Otevrete projekt v CLion nebo pouzijte CMake:
   - `mkdir build && cd build`
   - `cmake ..`
   - `cmake --build .`
3. Spusteni:
   - spustte vygenerovany spustitelny soubor nebo v CLion stisknete Run.
4. Poznamka: pri linkovani muze byt treba pridat systemove frameworky (OpenGL, Cocoa) podle nastaveni raylib na macOS.

Soubory
-------
- `main.cpp` - hlavni zdrojovy soubor s logikou a vykreslovanim
- `pattern_export.txt` - vyexportovany vzor (vytvori se pri stisku E)

Licence
-------
Projekt je licencovan pod MIT licencí.

Autor
-----
Portfolio projekt.
