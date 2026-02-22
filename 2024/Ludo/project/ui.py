import tkinter as tk
from PIL import Image, ImageTk
from project.paths import *  # Pokud používáš cesty z paths.py

def initialize_ui(current_player, circle_radius, players, start_game_button_clicked, select_tile):

    def initialize_figures(players):
        figures = {}
        for player in players:
            figures[player] = []
            for (x, y) in positions[player]:
                figure = canvas.create_image(x, y, image=figure_images[player])
                figures[player].append(figure)
        return figures
    
    def initialize_paths(unique_paths, shared_path):
        circles = []
        for x, y in shared_path:
            circle = canvas.create_oval(
                x - circle_radius, y - circle_radius,
                x + circle_radius, y + circle_radius,
                width=0
            )
            circles.append({"circle": circle, "x": x, "y": y, "figure": None})

        for player, path in unique_paths.items():
            for x, y in path:
                circle = canvas.create_oval(
                    x - circle_radius, y - circle_radius,
                    x + circle_radius, y + circle_radius,
                    width = 0
                )
                circles.append({"circle": circle, "x": x, "y": y, "figure": None})
        return circles

    root = tk.Tk()
    width = 600
    height = 640
    screen_width = root.winfo_screenwidth()
    screen_height = root.winfo_screenheight()

    x = (screen_width // 2) - (width // 2)
    y = (screen_height // 2) - (height // 2)

    image_path_background = "project/img/desk.png"
    image = Image.open(image_path_background)
    background_image = ImageTk.PhotoImage(image)

    canvas = tk.Canvas(root, width=600, height=600)
    canvas.pack(fill="both", expand=False)
    canvas.create_image(0, 0, image=background_image, anchor="nw")

    size = (16, 31)

    figure_images = {
        "Červený": ImageTk.PhotoImage(Image.open("project/img/red_figure.png").resize(size)),
        "Žlutý": ImageTk.PhotoImage(Image.open("project/img/yellow_figure.png").resize(size)),
        "Zelený": ImageTk.PhotoImage(Image.open("project/img/green_figure.png").resize(size)),
        "Modrý": ImageTk.PhotoImage(Image.open("project/img/blue_figure.png").resize(size))
    }

    positions = {
        "Červený": [(48, 48), (99, 48), (100, 98), (49, 98)],
        "Žlutý": [(48, 500), (98, 500), (98, 550), (48, 550)],
        "Zelený": [(500, 500), (550, 500), (550, 550), (500, 550)],
        "Modrý": [(500, 48), (550, 48), (550, 98), (500, 98)]
    }

    player_paths = {
        "Červený": path_red,
        "Žlutý": path_yellow,
        "Zelený": path_green,
        "Modrý": path_blue
    }

    figures = initialize_figures(players)

    circles = initialize_paths(unique_paths, shared_path)

    root.title("Člověče, nezlob se!")
    root.geometry(f"{width}x{height}+{x}+{y}")
    root.resizable(False, False)
    bottom_frame = tk.Frame(root, bg="lightgray")
    bottom_frame.pack(side="bottom", fill="x")

    player_turn_label = tk.Label(bottom_frame, text="", font=("Arial", 16), bg="lightgray")
    player_turn_label.pack(side="left", padx=10)

    roll_button = tk.Button(bottom_frame, text="Start the game", font=("Arial", 14), command = lambda: start_game_button_clicked(roll_button, player_turn_label))
    roll_button.pack(side="right", padx = 10)

    player_turn_label["text"] = f"Na tahu: {current_player}"
    canvas.bind("<Button-1>", lambda event: select_tile(event, figures, canvas, player_paths, positions, circles, root))
    root.mainloop()
