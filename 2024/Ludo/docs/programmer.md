# Programátorská dokumentace k projektu "Člověče, nezlob se!"

Tento projekt implementuje hru **Člověče, nezlob se!** v grafickém prostředí pomocí knihovny `tkinter` pro Python. Hráči mohou hrát hru ve verzi pro 2 až 4 hráče.

## Struktura projektu

Projekt je rozdělen do několika souborů:

```
├── main.py              # Hlavní soubor pro spuštění hry
│
└── project/
    ├── logic.py         # Logika hry
    ├── paths.py         # Definice cest pro hráče
    ├── ui.py            # UI hry
    └── img/             # Složka s herními obrázky
        ├── desk.png
        ├── red_figure.png
        ├── yellow_figure.png
        ├── green_figure.png
        └── blue_figure.png
```

## `main.py`

Tento soubor se stará o spuštění hry a dotaz na počet hráčů. Používá knihovnu `tkinter` pro vytvoření grafického rozhraní.

### Hlavní funkce:

#### `ask_player_count()`
Funkce, která zobrazuje dialog pro zadání počtu hráčů (2-4). Na základě tohoto čísla se spustí hra.

```python
def ask_player_count():
```

## `logic.py`

Tento soubor obsahuje hlavní herní logiku a správu interakcí na desce.

### Hlavní funkce:

#### `start_game(player_count)`
Funkce, která inicializuje herní prostředí, nastaví hráče a spustí hru. Spravuje také změny hráčů a jejich tahy.

```python
def start_game(player_count):
    # Inicializace herní logiky a komponenty
```

#### `start_game_button_clicked()`
Obsluhuje kliknutí na tlačítko pro začátek hry a provádí první hod kostkou nebo pohyby figurek.

```python
def start_game_button_clicked():
    # Spuštění hry a zpracování prvního hodu
```

#### `roll_dice()`
Funkce pro hod kostkou, která vrací náhodné číslo mezi 1 a 6.

```python
def roll_dice():
    roll_value = random.randint(1, 6)
```

#### `select_tile()`
Funkce pro výběr figurky, která by se měla pohybovat. Zajišťuje také kontrolu na správnost pohybu. (nevrácí nic pokud je tah nemožný)

```python
def select_tile():
    # Výběr políčka pro pohyb figurek
```

#### `return_figure_to_lobby()`
Funkce pro výběr figurky, která by se měla pohybovat. Zajišťuje také kontrolu na správnost pohybu. (nevrácí nic pokud je tah nemožný)

```python
def return_figure_to_lobby():
    # Vrácení figurky
```

#### `move_figure()`
Funkce pro výběr figurky, která by se měla pohybovat. Zajišťuje také kontrolu na správnost pohybu. (nevrácí nic pokud je tah nemožný)

```python
def move_figure():
    # pohyb figurky
```

### Základní komponenty:

- **`players`** - Seznam hráčů, který závisí na počtu hráčů zadaném uživatelem.
- **`figures`** - Seznam figurek každého hráče.
- **`circles`** - Kolekce kruhů reprezentujících možné pozice pro pohyb figurek.
- **`player_paths`** - Cesty pro každého hráče.

## `paths.py`

Soubor obsahující cesty, které hráči následují při pohybu na herní desce.

### Cesty pro jednotlivé barvy:

- **`shared_path`** - Společná cesta pro všechny hráče.
- **`unique_paths`** - Unikátní cesty pro jednotlivé hráče podle barvy.

```python
shared_path = [
    # Sdílené cesty pro všechny hráče
]
```

### Specifické cesty pro jednotlivé barvy:

```python
path_red = [
    # Cesta pro červeného hráče
]
path_yellow = [
    # Cesta pro žlutého hráče
]
path_blue = [
    # Cesta pro modrého hráče
]
path_green = [
    # Cesta pro zeleného hráče
]
```