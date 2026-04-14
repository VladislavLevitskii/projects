#pragma once
#include "Player.hpp"
#include "Button.hpp"
#include "GameConfig.hpp"
#include "BiddingGrid.hpp"
#include <SFML/Graphics.hpp>
#include <string>

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
    Button restartButton;

    Button changeBidToLeft;
    Button changeBidToRight;
    Button selectThisBid;

    Button biddingSelectGame;
    Button biddingPass;
    Button biddingMisere;

    sf::Text difficultyText;

    struct DifficultyOption {
        Button* button;
        GameDifficulty value;
    };

    float windowCenterX;
    DifficultyOption options[NUMBER_OF_DIFFICULTIES] = {};
    Bid lockedGrid;
    bool changedDifficulty = false;
    bool loweredBid = false;
    bool isLockedGrid = false;

    bool loadHandWave(const std::string& filename) { return handTexture.loadFromFile(filename); }
    void drawMenu(sf::RenderWindow& window, GameDifficulty& difficulty, GameState& gameState, sf::Clock& aiClock);
    void drawTable(sf::RenderWindow& window, PlayerList& listOfPlayer, Talon& talon, DiscardPile& discardPile);
    bool drawSelectingRegime(sf::RenderWindow& window, PlayerList& listOfPlayer, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid);
    bool drawSelectingRegimeBid(PlayerList& listOfPlayers, sf::RenderWindow& window, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid, int thrownCards);
    void drawTalon(Talon& talon, const float& shift, sf::RenderWindow& window);
    void drawDiscardPile(DiscardPile& discardPile, sf::RenderWindow& window);
    void drawFinalScore(sf::RenderWindow& window, PlayerList& listOfPlayers, GameState& gameState, GameType& gameType);
    void drawScore(PlayerList& listOfPlayers, sf::RenderWindow& window);
    void drawCards(PlayerList& listOfPlayers, const float& shift, sf::RenderWindow& window);
    void drawRegimes(PlayerList& listOfPlayers, sf::RenderWindow& window);
};