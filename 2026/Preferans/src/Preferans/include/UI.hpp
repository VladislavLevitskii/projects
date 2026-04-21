#pragma once
#include "Player.hpp"
#include "Button.hpp"
#include "GameConfig.hpp"
#include "BiddingGrid.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @class UI
 * @brief Manages the User Interface and rendering.
 * This class is responsible for drawing all visual elements on the screen 
 * (menus, game table, cards, scores, and indicators) and handling the graphical 
 * representation of the game state.
 * 
 * @note Some methods return bool, that is done for communication between UI and 
 * Game classes
 */
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
    
    /**
     * Draws the main menu on the screen.
     * @param window The SFML render window where the menu will be drawn.
     * @param difficulty Reference to the current game difficulty setting.
     * @param gameState Reference to the current state of the game.
     * @param aiClock Reference to the clock used for AI timing.
     */
    void drawMenu(sf::RenderWindow& window, GameDifficulty& difficulty, GameState& gameState, sf::Clock& aiClock);
    
    /**
     * Draws the basic game table, including the players, talon, and discard pile.
     * @param window The SFML render window where the table will be drawn.
     * @param listOfPlayer Reference to the list of players in the game.
     * @param talon Reference to the talon object.
     * @param discardPile Reference to the discard pile object.
     */
    void drawTable(sf::RenderWindow& window, PlayerList& listOfPlayer, Talon& talon, DiscardPile& discardPile);
    
    /**
     * Draws the talon on the table.
     * @param talon Reference to the talon object to be drawn.
     * @param shift Float value representing the visual shift/offset for the cards.
     * @param window The SFML render window where the talon will be drawn.
     */
    void drawTalon(Talon& talon, const float& shift, sf::RenderWindow& window);

    /**
     * Draws the discard pile on the table.
     * @param discardPile Reference to the discard pile object to be drawn.
     * @param window The SFML render window where the discard pile will be drawn.
     */
    void drawDiscardPile(DiscardPile& discardPile, sf::RenderWindow& window);

    /**
     * Draws the end game screen and displays the final score.
     * @param window The SFML render window where the final score will be drawn.
     * @param listOfPlayers Reference to the list of players to extract scores.
     * @param gameState Reference to the current state of the game.
     * @param gameType Reference to the finished game type.
     */
    void drawFinalScore(sf::RenderWindow& window, PlayerList& listOfPlayers, GameState& gameState, GameType& gameType);
    
    /**
     * Draws the current scores of all players during the game.
     * @param listOfPlayers Reference to the list of players.
     * @param window The SFML render window where the scores will be drawn.
     */
    void drawScore(PlayerList& listOfPlayers, sf::RenderWindow& window);

    /**
     * Draws all the players' cards on the screen.
     * @param listOfPlayers Reference to the list of players.
     * @param shift Float value representing the visual shift/offset between cards.
     * @param window The SFML render window where the cards will be drawn.
     */
    void drawCards(PlayerList& listOfPlayers, const float& shift, sf::RenderWindow& window);

    /**
     * Draws the selected regimes (game modes) above each player's avatar/area.
     * @param listOfPlayers Reference to the list of players.
     * @param window The SFML render window where the regimes will be drawn.
     */
    void drawRegimes(PlayerList& listOfPlayers, sf::RenderWindow& window);

    /**
     * Draws the regime selection interface for the player.
     * @param window The SFML render window where the interface will be drawn.
     * @param listOfPlayer Reference to the list of players.
     * @param talon Reference to the talon object.
     * @param discardPile Reference to the discard pile object.
     * @param biddingGrid Reference to the bidding grid object.
     * @return True if a regime was successfully selected, false otherwise.
     */
    bool drawSelectingRegime(sf::RenderWindow& window, PlayerList& listOfPlayer, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid);
    
    /**
     * Draws the final bid selection interface for the player.
     * @param listOfPlayers Reference to the list of players.
     * @param window The SFML render window where the interface will be drawn.
     * @param talon Reference to the talon object.
     * @param discardPile Reference to the discard pile object.
     * @param biddingGrid Reference to the bidding grid object.
     * @param thrownCards The number of cards that have been discarded.
     * @return True if the final bid was confirmed, false otherwise.
     */
    bool drawSelectingRegimeBid(PlayerList& listOfPlayers, sf::RenderWindow& window, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid, int thrownCards);

    /**
     * Loads the hand wave texture from a specified file.
     * @param filename The path to the texture file.
     * @return True if the texture loading was successful, false otherwise.
     */
    bool loadHandWave(const std::string& filename) { return handTexture.loadFromFile(filename); }
};
