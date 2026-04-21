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

/**
 * @class Game
 * @brief The core controller and state manager of the application.
 * Handles the main game loop, enforces game rules, manages turns, distributes 
 * cards, and controls transitions between different game phases (e.g., menu, 
 * bidding, active round, end screen).
 */
class Game {
    friend class AI;
public:
    /**
     * Constructs the Game object.
     * @param win Reference to the SFML render window used for drawing.
     */
    Game(sf::RenderWindow& win);

    /**
     * Starts the main game loop.
     */
    void run();

private:
    std::mt19937 gen{ static_cast<unsigned int>(std::time(nullptr)) };
    UI ui;
    DiscardPile discardPile         = DISCARD_PILE_FOR_CARDS;
    Talon talon                     = TALON_POSITION_FOR_CARDS;
    sf::RenderWindow& window;
    BiddingGrid biddingGrid;
    sf::Font font;
    std::vector<Card> deck;
    PlayerList listOfPlayers;
    size_t currentPlayerIdx         = 0;
    size_t round                    = 0;
    sf::Clock aiClock;
    Suit trumpAllPass               = Suit::None;
    Suit trumpContract              = Suit::None;
    GameState gameState             = GameState::Menu;
    GameDifficulty difficulty       = GameDifficulty::Easy;
    GameType gameType               = GameType::None;
    int trickWinnerIdx              = 0;
    int numberOfTrumpCardsAllPass   = 0;
    int numberOfTrumpCardsContract  = 0;
    int playingIdxContract          = -1;
    int thrownCards                 = 0;
    bool turnedTalonCard            = false;
    bool trumpSetted                = false;
    bool dealt                      = false;
    bool timeRestarted              = false;
    bool calculatedTrump            = false;
    bool tookActionBidding          = false;
    bool finilizedBidding           = false;
    bool finilizedMisere            = false;
    bool talonDistributed           = false;
    bool hasEveryoneChoosedRegime   = false;
    bool reseted                    = false;
    std::random_device rd;

    /**
     * Updates the game logic based on the current state.
     */
    void update();

    /**
     * Draws the current game state to the window.
     */
    void render();

    /**
     * Resets the game and prepares parameters for a new game.
     */
    void resetGame(); 

    /**
     * Loads all necessary textures and fonts required for the game.
     */
    void initResources();

    /**
     * Checks if valid cards are being hovered by the mouse in the All Pass game mode.
     */
    void checkHoveringAllPass();

    /**
     * Checks if valid cards are being hovered by the mouse in the Contract game mode.
     */
    void checkHoveringContract();

    /**
     * Handles mouse clicks on the player's cards.
     */
    void checkCardClicked();

    /**
     * Passes the turn to the next player.
     */
    void passMove();

    /**
     * Evaluates the trick winner in the All Pass game mode.
     */
    void calculateScoreAllPass();

    /**
     * Evaluates the trick winner in the Contract game mode.
     */
    void calculateScoreContract();

    /**
     * Handles the core logic flow for the All Pass game mode.
     */
    void processAllPass();

    /**
     * Recalculates the final score for all players.
     */
    void recalculateTotal();

    /**
     * Wrapper for recalculateTotal, specifically used after a Contract game.
     */
    void recalculateTotalContract();

    /**
     * Wrapper for recalculateTotal, specifically used after a Misere game.
     */
    void recalculateTotalMisere();

    /**
     * Handles the bidding phase logic of the game.
     */
    void processBidding();

    /**
     * Handles the core logic flow for the Contract game mode.
     */
    void processContract();

    /**
     * Handles the core logic flow for the Misere game mode.
     */
    void processMisere();

    /**
     * Moves the talon cards into the human player's hand.
     */
    void playerTakeTalon();

    /**
     * Handles the discarding of chosen cards from the player's hand.
     */
    void iterateCardsAndThrow();

    /**
     * Handles playing a selected card from the hand to the discard pile.
     */
    void iterateCardsAndPlay();

    /**
     * Cleans up after a trick and prepares the next one.
     * @param playerTookIdx The index of the player who won the trick.
     */
    void endRound(size_t playerTookIdx);

    /**
     * Shuffles and distributes cards to the players and the talon.
     * @return True if the cards were successfully dealt, false otherwise.
     */
    bool dealCards();

    /**
     * Checks if all players have selected their preferred game regime.
     * @return True if everyone has chosen, false otherwise.
     */
    bool everyoneChoosedRegime();

    /**
     * Handles the card discarding phase before a contract begins.
     * @return True if the choosing phase is completed, false otherwise.
     */
    bool processChoosingCards();

    /**
     * Counts the number of cards currently played in the active trick.
     * @return The number of played cards.
     */
    int playedCards();

    /**
     * Exports the current game state for the AI minimax algorithm.
     * @return A MiniGameState object representing the current status.
     */
    MiniGameState exportForMinimax() const;
};
