# Zápočtový program: FireSimulation

## Specifikace

**FireSimulation** je desktopová hra napsaná v jazyce **C#** s použitím **Windows Forms**.  
Program zobrazuje mřížku buněk představujících les. Stromy mohou náhodně růst a za určitých podmínek vzplanout – buď náhodně, nebo postupně.  
Uživatel spustí simulaci z hlavního menu. V určitých intervalech hra přepíná mezi růstem lesa a požárem.  
Hasiči pod vedením uživatele míří k ohniskům požáru, hasí je, a pokud jim dojde voda, jedou doplnit zásoby na okraj mapy.  
Simulace zaznamenává počet stromů v každém kroku a po jejím ukončení se zobrazí graf vývoje spolu s průměrným skóre.

## Instalace a spuštění

### Spuštění z EXE

1. Stáhnětě archiv repozitáře a rozbalte.
2. Ve složce published spusťte `FIRE_SIMULATION.exe`.

## Dokumentace

Dokumentace k programu je rozdělena na několik částí:

* [Uživatelská dokumentace](docs/user.md) – Návod pro hráče, jak hrát hru.
* [Programátorská dokumentace](docs/programmer.md) – Podrobný popis kódu a struktury projektu.