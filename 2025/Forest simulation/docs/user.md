# Uživatelská dokumentace

Tato hra simuluje šíření požáru v náhodně generovaném lese a práci hasičů na jeho hašení. Níže najdete návod, jak program spustit a jak jej ovládat.

---

## Spuštění programu

1. Stáhnětě archiv repozitáře a rozbalte.
2. Ve složce published spusťte `FIRE_SIMULATION.exe`.

---

## Hlavní okno (Menu)

Po spuštění se objeví jednoduché menu, kde můžete:

- **Vybrat obtížnost simulace**  
  Každá volba nastaví odlišné parametry (kapacita hasiče, pravděpodobnost požáru, rychlost růstu stromů apod.).
  Ve spodní části je rozbalovací seznam (ComboBox) s položkami:
  - **Lehká** (výchozí)
  - **Střední**
  - **Těžká**    

- **Odkaz na pravidla**  
  Kliknutím na text „Pravidla“ (link) se otevře výchozí webový prohlížeč a načte se stránka s popisem herních principů.

- **Tlačítko „Start“**  
  Po výběru obtížnosti klikněte na tlačítko **Start**. Tím se přepnete do režimu simulace.

---

## Režim simulace

Po kliknutí na **Start** se objeví herní plátno (DoubleBufferPanel) uprostřed okna, kde se vykresluje les. Simulace probíhá v několika fázích:

1. **Fáze růstu stromů**  
   - Timer každým tiknutím aktualizuje stav každé buňky.  
   - Stromové buňky postupně „stárnou“ a prázdné buňky vedle již existujících stromů mohou náhodně vyrůst v nový strom.

2. **Zahájení požáru**  
   - Náhodně, s předem nastavenou pravděpodobností, se simulace přepne do stavu požáru.  
   - V této fázi se v lese náhodně zapálí několik stromů.

3. **Fáze požáru a zásahy hasičů**  
   - Hořící buňky šíří plameny na sousední stromy.  
   - **Tlačítko Naplnit vodu**:  
     - Stisknutím mezerníku (klávesa **Space**) se tlačítko „Naplnit“ automaticky aktivuje.  
     - Hasiči po kliknutí na tlačítko vyrazí k nejbližšímu okraji mapy, aby doplnili vodu.  
   - **Kliknutí myší do plátna (panelu)**:  
     - V režimu požáru můžete kliknout na libovolnou buňku. Hasič pak dojde k této buňce a hasí strom (samozřejmě pokud má dostatek vody).
   - **Fáze požáru skončí, když na mapě nezbyde žádný oheň, nebo když všechny stromy shoří**

4. **Ukončení simulace (konec hry)**  
   - Simulace se zastaví, když v celém lese není žádný strom nebo pokud uživatel stiskne tlačítko `Konec`.  
   - Po ukončení se zobrazí graf vývoje počtu stromů v novém okně, spolu s průměrným skóre (průměrným počtem stromů za celou simulaci).  
   - Zobrazí se tlačítko **Zpět**, kterým se vrátíte do menu a můžete spustit novou hru.

---

## Tipy a poznámky

- **Kapacita hasiče**:  
  Hodnota se liší podle obtížnosti. Ve „Lehké“ obtížnosti má hasič vyšší kapacitu (více vodních náplní) než ve „Těžké“.

- **Náhodné chování**:  
  Les se generuje náhodně a požár může začít v libovolné stromové buňce, takže každá hra je unikátní.

- **Mezerník**:  
  Nebojte se používat mezerník, hodně to šetří čas.

- **Strategie**:  
  Nejlepší strategie je izolování požáru od většího seskupení stromů.