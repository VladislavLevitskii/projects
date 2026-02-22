#include "Preferans.h"
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(
        sf::VideoMode(1200, 800),
        "Preferans",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);
    Game game(window);
    game.run();

    return 0;
}