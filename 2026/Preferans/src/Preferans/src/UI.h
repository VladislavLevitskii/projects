#pragma once
#include "Player.h"
#include "Button.h"
#include "GameConfig.h"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <ranges>
#include <iostream>

class UI {
public:
    UI(sf::Window& window);

    sf::Texture deckTexture;
    sf::Texture backTexture;
    sf::Font font;
    sf::Texture handTexture;
    Button startButton;
    Button easyButton;
    Button mediumButton;
    Button hardButton;
    sf::Text difficultyText;

    struct DifficultyOption {
        Button* button;
        GameDifficulty value;
    };

    float windowCenter;
    DifficultyOption options[NUMBER_OF_DIFFICULTIES] = {};
    bool changedDifficulty = false;

    bool loadHandWave(const std::string& filename) { return handTexture.loadFromFile(filename); }
    void drawMenu(sf::RenderWindow& window, GameDifficulty& difficulty, GameState& gameState);
    void drawTable(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>>& listOfPlayer, Talon& talon, DiscardPile& discardPile);
    void drawTalon(Talon& talon, const float& shift, sf::RenderWindow& window);
    void drawDiscardPile(DiscardPile& discardPile, sf::RenderWindow& window);
    void drawScore(std::vector<std::unique_ptr<Player>>& listOfPlayers, sf::RenderWindow& window);
    void drawCards(std::vector<std::unique_ptr<Player>>& listOfPlayers, const float& shift, sf::RenderWindow& window);
};