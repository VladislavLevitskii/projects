#pragma once
#include <SFML/Graphics.hpp>
#include "Card.hpp"
#include "Button.hpp"
#include "Player.hpp"
#include "GameConfig.hpp"
#include "UI.hpp"
#include "BiddingGrid.hpp"
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <memory>
#include <array>
#include <stdexcept>
#include <ctime>

class Game {
public:
    Game(sf::RenderWindow& win);
    void run();
private:
    std::mt19937 gen{ static_cast<unsigned int>(std::time(nullptr)) };
    UI ui;
    DiscardPile discardPile;
    Talon talon;
    sf::RenderWindow& window;
    BiddingGrid biddingGrid;
    sf::Font font;
    std::vector<Card> deck;
    PlayerList listOfPlayers;
    size_t currentPlayerIdx;
    size_t round = 0;
    sf::Clock aiClock;
    Suit trumpAllPass = Suit::None;
    Suit trumpContract = Suit::None;
    GameState gameState;
    GameDifficulty difficulty;
    GameType gameType;
    int trickWinnerIdx = 0;
    int numberOfTrumpCardsAllPass;
    int numberOfTrumpCardsContract;
    int playingIdxContract;
    int thrownCards;
    bool turnedTalonCard = false;
    bool trumpSetted = false;
    bool dealt;
    bool timeRestarted;
    bool calculatedTrump;
    bool tookActionBidding;
    bool finilizedBidding;
    bool finilizedMisere;
    bool talonDistributed;
    bool hasEveryoneChoosedRegime;
    bool reseted;
    std::random_device rd;

    void render();
    void update();
    void checkHoveringAllPass();
    void checkHoveringContract();
    void checkCardClicked();
    bool dealCards();
    void passMove();
    void calculateScoreAllPass();
    void calculateScoreContract();
    int playedCards();
    void processAllPass();
    void recalculateTotal();
    void recalculateTotalContract();
    void processBidding();
    bool everyoneChoosedRegime();
    void processContract();
    bool processChoosingCards();
    void processMisere();
    void playerTakeTalon();
    void iterateCardsAndThrow();
    void iterateCardsAndPlay();
    void endRound(size_t playerTookIdx);
    void recalculateTotalMisere();
};
