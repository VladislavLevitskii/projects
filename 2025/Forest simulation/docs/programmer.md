# Programátorská dokumentace

Tento projekt simuluje šíření požáru v lese a práci hasičů na jeho hašení. Hlavní logika je rozdělena do několika tříd a souborů: `UI.cs` (uživatelské rozhraní a řízení stavu hry), `Forest.cs` (datový model lesa a pravidla šíření), `ForestRenderer.cs` (vykreslování stavu lesa do bitmapy), `FireFighter.cs` (model hasiče a jeho pohyb/naplňování vody) a `Cell.cs` (reprezentace jednotlivé buňky mřížky s informacemi o stavu a hasičích). 
Níže je stručný popis každého souboru a jeho klíčových tříd.

---

## UI.cs

- **Účel**: Obsahuje hlavní formulář (`UI`), který řídí herní smyčku, vykreslování, přepínání mezi stavy (menu, hra, fáze ohně, konec), zpracovává uživatelské vstupy (klávesy, kliky myší) a vykreslení statistik (graf počtu stromů).  
- **Důležité části**:
    - **Enum `State`** – definice stavů aplikace: `MENU`, `GAME`, `GAME_FIRE`, `END`.
    - **Metoda `ChooseState(State newState)`** – přepíná viditelnost ovládacích prvků a spouští/přerušuje timery podle stavu.
    - **Metoda `DrawChart()`** – vytvoří nové okno s grafem vývoje počtu stromů a zobrazí skóre.
    - **Metoda `GeneratePanel()`** – inicializuje a umístí panel pro vykreslení bitmapy lesu.
    - **Obsluha timerů** (`timer_forest_Tick`, `timer_firefighter_Tick`) – aktualizuje stav lesa a pohyb hasičů v závislosti na aktuálním stavu (`GAME` nebo `GAME_FIRE_`).
    - **Metoda `EmptyFF()`** – resetuje frontu a pozice všech hasičů.
    - **Obsluha událostí tlačítek** (`callGame_Click`, `BEnd_Click`, `BBack_Click`, `BFill_Click`) – přepínání stavů hry a plnění hasičů vodou.
    - **Obsluha `Panel_MouseClick`** – při kliknutí na mapu v režimu požáru přidá cíl pro hasiče.

---

## ForestRenderer.cs

- **Účel**: Obsahuje statickou třídu, která převádí aktuální stav mřížky (`Cell[,]`) na bitmapu pro vykreslení v UI.  
- **Důležité části**:
    - **Statické pole `Bitmap bmp` a `Graphics g`** – bitmapa a objekt pro kreslení, znovu využívané pro výkon.
    - **Metoda `Render(Cell[,] grid)`** – v cyklech sbírá obdélníky podle typu buňky a pět barev (červená pro oheň, světle šedá pro prázdné, odstíny zelené pro stromy podle hodnoty `ColorValue`), pak vykreslí obdélníky, mřížku a ikony hasičských jednotek (modré elipsy uvnitř buněk s hasičem).

---

## Forest.cs

- **Účel**: Definuje třídu modelu lesa, která udržuje mřížku buněk, parametry simulace a obsahuje logiku pro generování lesa, vzniku požáru a aktualizaci stavu buněk.  
- **Důležité části**:
    - **Metoda `Empty(Cell[,] Grid)`** – inicializuje všechny buňky na stav `Empty`.
    - **Metoda `GenerateForest()`** – v náhodném výběru 10 buněk nastaví stromy s náhodnou intenzitou zelené a vytvoří jednu instanci hasiče (přidá do statické kolekce `FireFighters`).
    - **Metoda `GenerateFire()`** – náhodně spouští hořící buňky na místech, kde je strom (cca 0,5 % šance na spálení každé stromové buňky).
    - **Metoda `CalculatePath(...)`** – jednoduchá logika pro výpočet trasy hasiče k určité buňce (posun po přímce).
    - **Metoda `UpdateState()`** – podle stavu UI (`GAME` nebo `GAME_FIRE_`) provede jednu iteraci simulace:
      - **Reprodukce stromů** – buňky s `Tree` snižují `ColorValue` na základě sousedních stromů, a prázdné buňky mohou vyrůst ve strom podle sousedů a nastavené šance.
      - **Šíření požáru** – každá buňka `Burning` má šanci zhasnout (`chance_die_fire`) nebo rozšířit oheň na sousední stromy (`chance_grow_fire_every_turn`).
      - **Přepnutí do stavu `GAME_FIRE`** – pokud náhodná hodnota spadne pod `chance_fire_every_turn`.

---

## FireFighter.cs

- **Účel**: Modeluje jednotlivého hasiče, frontu úkolů (cesty), jeho pozici, kapacitu vody a logiku návratu k okraji (naplnění vodou).  
- **Důležité části**:
    - **Vlastnosti**: `Name`, `Capacity`, `Queue<int[]> Queue_FF`, `int[] Position`, `bool Disappear`.
    - **Statické pole `FireFighters`** – sdílený seznam všech hasičů v simulaci.
    - **Konstruktor** – inicializuje jméno, kapacitu, frontu a příznak pro zmizení.
    - **Metoda `FillWater(FireFighter fireFighter)`** – vyprázdní frontu, poté určí nejbližší okraj mřížky pro danou pozici a naplní frontu postupem po přímce od pozice k okraji. Nastaví příznak `MamZmizet`, aby hasič po dokončení cesty zmizel (vrátí se pro novou vodu).

---

## Cell.cs

- **Účel**: Definuje třídu buňky v mřížce lesa, kterou může obsahovat strom, prázdné místo nebo plamen, a seznam hasičů nacházejících se na této pozici.  
- **Důležité části**:
    - **Enum `CellState { Empty, Tree, Burning }`** – stav buňky.
    - **Vlastnosti**: `CellState State`, `int ColorValue` (volená intenzita zelené barvy pro stromy), `List<FireFighter> FireFighters` (seznam hasičů v buňce).
    - **Konstruktor** – inicializuje buňku jako prázdnou (`Empty`), `ColorValue = 0` a prázdný seznam hasičů.