import tkinter as tk
import random
from project.paths import *
from project.ui import initialize_ui

"""
Základní soubor s logikou pro hru
"""

def start_game(player_count):

    def start_game_button_clicked(roll_button, player_turn_label):
        nonlocal game_started, first_roll, current_player_index, move_made
        move_made = False
        if not game_started:
            game_started = True
            first_roll = True
            roll_button["text"] = "Hodit kostkou"
            return
        elif first_roll:
            roll_dice()
            current_player_index = (current_player_index) % len(players)
            first_roll = False
        else:
            roll_dice()
            current_player_index = (current_player_index + 1) % len(players)
        player_turn_label["text"] = f"Na tahu: {players[current_player_index]} - Hodil: {roll_value}"

    def roll_dice():
        nonlocal roll_value
        roll_value = random.randint(1, 6)

    def return_figure_to_lobby(figure, player, figures, canvas, positions, circles):
        figure_idx = figures[player].index(figure)
        lobby_x, lobby_y = positions[player][figure_idx]
        canvas.coords(figure, lobby_x, lobby_y)
        for circle in circles:
            if circle["figure"] == figure:
                circle["figure"] = None
                break

    def check_winner(player, canvas, player_paths, figures):
        path = player_paths[player]
        last_positions = path[-4:]

        for (x, y) in last_positions:
            position_occupied = False
            for figure in figures[player]:
                figure_coords = canvas.coords(figure)
                if abs(figure_coords[0] - x) < 20 and abs(figure_coords[1] - y) < 20:
                    position_occupied = True
                    break
            if not position_occupied:
                return False
        return True
    
    def select_figure(clicked_x, clicked_y, current_player, figures, canvas):
        for player, figures_list in figures.items():
            if player == current_player:
                for idx, figure in enumerate(figures_list):
                    coords = canvas.coords(figure)
                    figure_x, figure_y = coords[0], coords[1]
                    if (clicked_x - figure_x)**2 + (clicked_y - figure_y)**2 <= circle_radius**2:
                        return figure

    def select_tile(event, figures, canvas, player_paths, positions, circles, root):
        nonlocal selected_figure, current_player_index, game_started, roll_value, move_made

        if not game_started or move_made:
            return

        current_player = players[current_player_index]
        clicked_x, clicked_y = event.x, event.y

        selected_figure = select_figure(clicked_x, clicked_y, current_player, figures, canvas)

        if not selected_figure or selected_figure == [None]:
            return

        handle_selected_figure(selected_figure, current_player, canvas, player_paths, positions, circles, figures, root)

    def handle_selected_figure(selected_figure, current_player, canvas, player_paths, positions, circles, figures, root):
        nonlocal move_made

        current_position = canvas.coords(selected_figure)
        current_index = path_index_for_position(current_player, current_position, player_paths)

        if current_index == -1:
            handle_figure_in_lobby(selected_figure, current_player, canvas, player_paths, positions, circles, figures)
            return

        move_figure(selected_figure, current_player, current_index, canvas, player_paths, positions, circles, figures, root)

    def handle_figure_in_lobby(selected_figure, current_player, canvas, player_paths, positions, circles, figures):
        nonlocal move_made

        if roll_value == 6:
            start_tile = player_paths[current_player][0]

            for player, figures_list in figures.items():
                for figure in figures_list:
                    figure_coords = canvas.coords(figure)
                    if figure_coords[0] == start_tile[0] and figure_coords[1] == start_tile[1]:
                        return_figure_to_lobby(figure, player, figures, canvas, positions, circles)

            canvas.coords(selected_figure, start_tile[0], start_tile[1])
            move_made = True

    def move_figure(selected_figure, current_player, current_index, canvas, player_paths, positions, circles, figures, root):
        nonlocal move_made

        next_index = current_index + roll_value
        if next_index >= len(player_paths[current_player]):
            return

        next_position = player_paths[current_player][next_index]
        handle_collisions(next_position, selected_figure, figures, canvas, positions, circles)
        canvas.coords(selected_figure, next_position[0], next_position[1])
        move_made = True

        if check_winner(current_player, canvas, player_paths, figures):
            tk.messagebox.showinfo("Vítězství!", f"Hráč {current_player} vyhrál!")
            root.destroy()

    def handle_collisions(next_position, selected_figure, figures, canvas, positions, circles):
        tolerance = 10
        for player, figures_list in figures.items():
            for figure in figures_list:
                figure_coords = canvas.coords(figure)
                # Porovnáváme pozice s tolerancí
                if (abs(next_position[0] - figure_coords[0]) <= tolerance and 
                    abs(next_position[1] - figure_coords[1]) <= tolerance and 
                    figure != selected_figure):
                    return_figure_to_lobby(figure, player, figures, canvas, positions, circles)

    def path_index_for_position(player, position, player_paths):
        threshold = 5
        path = player_paths[player]
        for idx, (x, y) in enumerate(path):
            if abs(position[0] - x) < threshold and abs(position[1] - y) < threshold:
                return idx
        return -1

    first_roll = False
    game_started = False
    current_player_index = 0
    roll_value = 0
    selected_figure = [None]
    move_made = False
    circle_radius = 19

    players = ["Červený", "Modrý", "Zelený", "Žlutý"][:player_count]
    current_player = players[0]

    initialize_ui(current_player, circle_radius, players, start_game_button_clicked, select_tile)