# Uživatelská dokumentace

## Člověče, nezlob se! – Pravidla a ovládání

Tato dokumentace popisuje pravidla hry **Člověče, nezlob se!**, ovládání a interakce ve hře.

---

## Pravidla hry

1. **Cíl hry:**
   - Přesunout všechny své čtyři figurky z výchozího pole do cílové zóny na základě hodu kostkou.

2. **Začátek hry:**
   - Každý hráč začíná se čtyřmi figurkami ve své "lobby".
   - Pro vstup na herní desku je nutné hodit číslo **6**.

3. **Průběh tahu:**
   - Hráč hodí kostkou a může pohnout libovolnou vlastní figurkou na základě výsledku hodu kliknutím na figurku.
   - Hra je po směru hodinových ručiček

4. **Vytlačování figurek:**
   - Pokud hráč vstoupí na pole obsazené jinou figurkou (i stejné barvy), figurka se vrací do "lobby".

5. **Cílová zóna:**
   - Figurky musí přesně odpovídat výsledku hodu, aby mohly vstoupit do cílové zóny.
   - Cílová zóna je bezpečná pro figurky, které tam vstoupí. (ale vytlačit může i figurka stejné barvy)

6. **Konec hry:**
   - Hráč, který jako první dostane všechny své figurky do cílové zóny, vyhrává.

---

## Ovládání

1. **Spuštění hry:**
   - Spusťte hru příkazem `python main.py`.
   - Zadejte počet hráčů (2–4) v dialogovém okně, které se zobrazí.

2. **Práce s herním oknem:**
   - **Hod kostkou:** Klikněte na tlačítko "Hodit kostkou" v dolní části okna.
   - **Výběr figurky:** Klikněte na figurku, kterou chcete přesunout, a ta se automaticky přesune podle hodu kostkou.

3. **Indikace na tahu:**
   - Ve spodní části herního okna je zobrazena informace o hráči, který je na tahu, včetně hodnoty hodu.

---

## Herní tipy

- Vždy se snažte vstoupit na herní desku co nejdříve.
- Při hodu čísla **6** si promyslete, kterou figurku posunete, abyste maximalizovali svůj postup.
- Využívejte možnost vytlačování soupeřových figurek pro zpomalení jejich postupu.
- Dávejte pozor na cílovou zónu – správné načasování hodu je klíčové!

---

Pokud narazíte na problémy při spuštění nebo během hraní, zkontrolujte:

1. **Kompatibilitu Pythonu:**
   - Hra vyžaduje Python 3.7 nebo novější.

2. **Dostupnost souborů:**
   - Ujistěte se, že všechny herní obrázky jsou ve složce `project/img/`.

3. **Instalaci balíčků:**
   - Zajistěte, že máte nainstalovány knihovny `tkinter` a `Pillow`.

## Poznámky

1. Pro správné spuštění hry se ujistěte, že všechny obrázky (`desk.png`, `red_figure.png`, `yellow_figure.png`, `green_figure.png`, `blue_figure.png`) jsou dostupné ve složce `project/img/`.
2. Hru spouštějte z hlavního adresáře projektu příkazem uvedeným výše.