#pragma once
#include <SFML/Graphics.hpp>
#include "Card.h"
#include "Button.h"
#include "Player.h"
#include "GameConfig.h"
#include "UI.h"
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <memory>
#include <array>
#include <stdexcept>

class Game {
public:
    Game(sf::RenderWindow& win);

    UI ui;
    DiscardPile discardPile;
    Talon talon;
    sf::RenderWindow& window;
    sf::Font font;
    std::vector<Card> deck;
    std::vector<std::unique_ptr<Player>> listOfPlayers;
    size_t currentPlayerIdx;
    size_t round = 0;
    sf::Clock clockForStartingGame;
    sf::Clock aiClock;
    int trickWinnerIdx = 0;
    Suit trump = Suit::None;
    GameState gameState;
    GameDifficulty difficulty;
    GameType gameType;
    int numberOfTrumpCards;
    bool turnedTalonCard = false;
    bool trumpSetted = false;
    bool dealt;
    bool timeRestarted;
    bool calculatedTrump;

    void run();
    void render(UI& ui);
    void update(UI& ui);
    void checkHovering(const int numberOfTrumps);
    void checkClicked();
    bool dealCards();
    void passMove();
    void calculateScore();
    int playedCards();
    void processAllPass();
    int getNumberOfTrumpCards;
};
