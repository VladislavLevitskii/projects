#include "Preferans.hpp"

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