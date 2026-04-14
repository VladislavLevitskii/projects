#include "Preferans.hpp"
#include <SFML/Graphics.hpp>

constexpr int WIDTH_WINDOW = 1200;
constexpr int HEIGHT_WINDOW = 800;
constexpr int FRAME_RATE = 60;

int main() {
    sf::RenderWindow window(
        sf::VideoMode(WIDTH_WINDOW, HEIGHT_WINDOW),
        "Preferans",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(FRAME_RATE);
    Game game(window);
    game.run();

    return 0;
}