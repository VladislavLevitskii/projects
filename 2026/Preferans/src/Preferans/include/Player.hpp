#pragma once
#include "Card.hpp"
#include "GameConfig.hpp"
#include "BiddingGrid.hpp"
#include "Minimax.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <assert.h>
#include <string>
#include <iostream>

/**
 * Converts and displays the player's wanted game mode.
 * @param gameType The gameType the user wants.
 * @param bid Used to display what bid the player wants to play (used in contract).
 * @return String representation to display.
 */
std::string gameTypeToString(const GameType gameType, const Bid bid);

// forward declarations
class DiscardPile;
class Talon;
class Game;

/**
 * @class CardContainer
 * @brief Abstract base class for anything that holds cards.
 * Provides a standard std::vector holding Card objects. Used as a foundation 
 * for players' hands, the discard pile, and the talon.
 */
class CardContainer {
public:
    std::vector<Card> hand;
    virtual ~CardContainer() = default;
};

/**
 * @class TablePile
 * @brief Represents a generic physical pile of cards on the table.
 * Inherits from CardContainer and adds 2D spatial coordinates (SFML Vector2f) 
 * to position the pile correctly on the game screen.
 */
class TablePile : public CardContainer {
public:
    sf::Vector2f position;
    TablePile(sf::Vector2f pos) : position(pos) {}
    virtual ~TablePile() = default;
};

/**
 * @class Talon
 * @brief Represents the talon (or 'window') in the game.
 * Inherits from TablePile. Holds the cards set aside during the initial deal 
 * that the winning bidder can draw and exchange before a contract begins.
 */
class Talon : public TablePile {
public:
    Talon(sf::Vector2f posCards)
        : TablePile(posCards) {}
};

/**
 * @class DiscardPile
 * @brief Represents the central play area during a trick.
 * Inherits from TablePile. Manages the specific slot positions on the table 
 * where players throw their cards during active gameplay and handles the 
 * visibility of the current trick.
 */
class DiscardPile : public TablePile {
public:
    std::vector<sf::Vector2f> slotPositions;

    DiscardPile(sf::Vector2f posCards)
        : TablePile(posCards) {
        hand.resize(NUMBER_OF_PLAYERS);

        slotPositions.emplace_back(posCards.x + PADDING_BOTTOM, posCards.y + (PADDING_BOTTOM * 2));
        slotPositions.emplace_back(posCards.x, posCards.y);
        slotPositions.emplace_back(posCards.x + (PADDING_BOTTOM * 2), posCards.y);
    }

    /**
     * Hides all cards currently in the discard pile.
     */
    void setCardsInvisible();
};

/**
 * @class Player
 * @brief Abstract base class representing a generic participant in the game.
 * Holds essential data such as the player's hand, current score, selected game 
 * regime, and provides pure virtual functions that must be implemented to 
 * execute game moves.
 */
class Player : public CardContainer {
public:
    Player(const sf::Vector2f posCards, const sf::Vector2f posHand, const sf::Vector2f posScore, const sf::Vector2f posRegime)
        : positionForCards(posCards), positionForHand(posHand), positionForScore(posScore), positionForRegime(posRegime) {}

    virtual ~Player() = default;

    sf::Vector2f positionForCards;
    sf::Vector2f positionForHand;
    sf::Vector2f positionForScore;
    sf::Vector2f positionForRegime;
    sf::Text scoreDisplay;
    sf::Text gameModeDisplay;
    sf::Sprite turnIndicator;
    Bid bidForContract;
    GameType playerWantedRegime = GameType::None;
    int tricks = 0;
    int whists = 0;
    int penalty = 0;
    int bullet = 0;
    int total = 0;
    bool hasMove = false;
    bool hasTrump = false;

    /**
     * Executes the All Pass game mode move.
     * @param difficulty Current game difficulty.
     * @param discardPile Reference to the discard pile.
     * @param trump Current trump suit.
     * @param gen Random number generator.
     * @param gameType Current game type.
     * @param game Reference to the main Game object.
     */
    virtual void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType, const Game& game) = 0;
    
    /**
     * Handles the bidding phase move.
     * @param difficulty Current game difficulty.
     * @param discardPile Reference to the discard pile.
     * @param gen Random number generator.
     * @param biddingGrid Reference to the bidding grid.
     * @param game Reference to the main Game object.
     */
    virtual void makeMoveBid(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) = 0;
    
    /**
     * Makes the final bid selection after the bidding phase racing.
     * @param difficulty Current game difficulty.
     * @param talon Reference to the talon.
     * @param gen Random number generator.
     * @param biddingGrid Reference to the bidding grid.
     * @param game Reference to the main Game object.
     */
    virtual void makeFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) = 0;
    
    /**
     * Executes a move during a contract game mode.
     * @param difficulty Current game difficulty.
     * @param discardPile Reference to the discard pile.
     * @param gen Random number generator.
     * @param biddingGrid Reference to the bidding grid.
     * @param trumpAllPass Trump suit from the discard pile logic.
     * @param trumpContract Actual trump suit for the contract.
     * @param gameType Current game type.
     * @param game Reference to the main Game object.
     */
    virtual void makeMoveContract(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, Suit& trumpAllPass, Suit& trumpContract, const GameType gameType, const Game& game) = 0;
    
    /**
     * Discards cards before a misere game begins.
     * @param difficulty Current game difficulty.
     * @param talon Reference to the talon.
     * @param gen Random number generator.
     * @param game Reference to the main Game object.
     */
    virtual void makeMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, const Game& game) = 0;

    /**
     * Evaluates all playable cards in a contract.
     * @param trumpAllPass Trump suit in case there is no trump contract suit on the discard pile.
     * @param trumpContract Trump suit for the contract.
     * @return Vector of legal card indices.
     */
    virtual std::vector<int> getLegalMovesContract(const Suit trumpAllPass, const Suit trumpContract);
    
    /**
     * Evaluates all playable cards in All Pass game mode.
     * @param trump Current trump suit.
     * @return Vector of legal card indices.
     */
    virtual std::vector<int> getLegalMovesAllPass(Suit trump);
    
    /**
     * Initializes the turn indicator sprite.
     * @param texture Texture to apply to the indicator.
     */
    virtual void initIndicator(sf::Texture& texture);

    /**
     * Counts the number of trump cards in the player's hand.
     * @param trump Suit to count.
     * @return Number of trump cards.
     */
    int getNumberOfTrumpCards(const Suit trump);

    /**
     * Updates all UI information for the player.
     */
    void updateFullInfo();

    /**
     * Refreshes the displayed score text.
     */
    void updateScoreText();

    /**
     * Sets up the score text display.
     * @param font SFML font used for the text.
     */
    void initScoreDisplay(const sf::Font& font);

    /**
     * Sets up the game mode text display.
     * @param font SFML font used for the text.
     */
    void initGameModeDisplay(const sf::Font& font);
};

/**
 * @class AI
 * @brief Represents a computer-controlled opponent.
 * Inherits from the base Player class. Uses algorithms (such as Minimax or 
 * specific heuristics depending on the difficulty) to automatically evaluate 
 * the game state and determine the optimal moves and bids.
 */
class AI : public Player {
public:
    AI(const sf::Vector2f posCards, const sf::Vector2f posHand, int index, const sf::Vector2f posScore, const sf::Vector2f posRegime)
        : Player(posCards, posHand, posScore, posRegime), indexAI(index) {}

    /** @name Virtual Overrides
     * Overrides of the virtual movement functions from the Player class.
     */
    ///@{
    void makeMoveContract(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, Suit& trumpAllPass, Suit& trumpContract, const GameType gameType, const Game& game) override;
    void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType, const Game& game) override;
    void makeMoveBid(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) override;
    void makeMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, const Game& game) override;
    void makeFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) override;
    ///@}

    /**
     * Executes a move in Contract mode when difficulty is set to easy.
     * @param discardPile Reference to the discard pile.
     * @param trump Actual trump suit for the contract.
     * @param trumpAllPass Trump suit from the discard pile logic.
     * @param gen Random number generator.
     * @param gameType Current game type.
     */
    void easyMoveContract(DiscardPile& discardPile, const Suit& trump, Suit& trumpAllPass, std::mt19937& gen, const GameType gameType);
    
    /**
     * Executes a move in All Pass mode when difficulty is set to easy.
     * @param discardPile Reference to the discard pile.
     * @param trump Current trump suit.
     * @param gen Random number generator.
     * @param gameType Current game type.
     */
    void easyMoveAllPass(DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType);
    
    /**
     * Executes a bidding move when difficulty is set to easy.
     * @param biddingGrid Reference to the bidding grid.
     * @param gen Random number generator.
     */
    void easyMoveBid(BiddingGrid& biddingGrid, std::mt19937& gen);
    
    /**
     * Prepares for a misere game when difficulty is set to easy.
     * @param difficulty Current game difficulty.
     * @param talon Reference to the talon.
     * @param gen Random number generator.
     */
    void easyMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen);
    
    /**
     * Makes the final bid selection when difficulty is set to easy.
     * @param difficulty Current game difficulty.
     * @param talon Reference to the talon.
     * @param gen Random number generator.
     */
    void easyMoveFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen);

    /**
     * Determines the bid to play using heuristics (used in medium and hard difficulties).
     * @param biddingGrid Reference to the bidding grid.
     */
    void moveBidHeuristic(BiddingGrid& biddingGrid);

    /**
     * Places a selected card on the discard pile.
     * @param rndIdx Index of the card to play.
     * @param discardPile Reference to the discard pile.
     */
    void playCardAI(int rndIdx, DiscardPile& discardPile);

    /**
     * Picks a random legal card to play (used in easy difficulty).
     * @param trumpAllPass Trump suit from the discard pile logic.
     * @param gen Random number generator.
     * @param gameType Current game type.
     * @param trumpContract Optional actual trump suit for the contract.
     * @return Index of the selected card.
     */
    int getRandomCardIdx(const Suit trumpAllPass, std::mt19937& gen, const GameType gameType, const Suit trumpContract = Suit::None);

    /**
     * Intelligently discards cards to the talon (in contract game mode).
     * @param trump Current trump suit.
     */
    void performSmartDiscard(Suit trump);

    /**
     * Predicts the number of tricks the AI can win.
     * @param currentHand Vector representing the AI's current hand.
     * @param trump Current trump suit.
     * @return Estimated number of tricks.
     */
    int estimateTricks(const std::vector<Card>& currentHand, Suit trump);

private:
    int indexAI = 0;
    std::random_device rd;
};

/**
 * @class HumanPlayer
 * @brief Represents a human player in the game.
 * Inherits from the base Player class. Unlike the AI, this class relies on 
 * physical UI interactions (such as mouse clicks) to make decisions during 
 * the bidding and playing phases.
 */
class HumanPlayer : public Player {
public:
    using Player::Player;

    /** @name Unused Overrides
     * These functions are not used by human players but must be initialized due to C++ polymorphism.
     */
    ///@{
    void makeMoveContract(GameDifficulty, DiscardPile&, std::mt19937&, BiddingGrid&, Suit&, Suit&, const GameType, const Game&) override {}
    void makeMoveAllPass(GameDifficulty, DiscardPile&, Suit&, std::mt19937&, GameType, const Game&) override {}
    void makeMoveBid(GameDifficulty, DiscardPile&, std::mt19937&, BiddingGrid&, const Game&) override {}
    void makeMoveMiserePrepare(GameDifficulty, Talon&, std::mt19937&, const Game&) override {}
    void makeFinalBid(GameDifficulty, Talon&, std::mt19937&, BiddingGrid&, const Game&) override {}
    ///@}
};
