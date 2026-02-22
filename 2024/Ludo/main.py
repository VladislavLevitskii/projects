import tkinter as tk
from tkinter import simpledialog
from project.logic import start_game

"""
Základní nastavení hry Člověče, nezlob se !
"""

def ask_player_count():
    question = tk.Tk()
    question.withdraw()

    player_count = simpledialog.askinteger(
        "Počet hráčů",
        "Zadejte počet hráčů (2-4):",
        minvalue=2,
        maxvalue=4
    )
    question.destroy()
    start_game(player_count)

if __name__ == "__main__":
    ask_player_count()