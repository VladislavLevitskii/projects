#pragma once
#include <SFML/Graphics.hpp>
#include "GameConfig.hpp"

/**
 * Converts a suit enum to its string representation.
 * @param suit The suit to convert.
 * @return The string representation of the suit.
 */
std::string suitToString(Suit suit);

/**
 * @class CardBase
 * @brief A lightweight base class for logical card data.
 * Contains only the fundamental properties of a playing card (suit and value) 
 * without any graphical or physical overhead. Useful for AI calculations 
 * and data structures.
 */
class CardBase {
protected:
    int value;
    Suit suit;
public:
    /**
     * Retrieves the suit of the card.
     * @return The suit of the card.
     */
    Suit getSuit() const { return suit; }

    /**
     * Retrieves the value of the card.
     * @return The value of the card (adjusted if necessary, e.g., Ace high).
     */
    int getValue() const { return (value == 1) ? CARDS_ONE_SUIT - 1 : value; }

    /**
     * Constructs a base card with a specific value and suit.
     * @param v The value of the card.
     * @param s The suit of the card.
     */
    CardBase(int v, Suit s) : value(v), suit(s) {}
};

/**
 * @class Card
 * @brief A graphical representation of a playing card.
 * Inherits from CardBase and adds SFML-specific properties such as sprites, 
 * textures (front and back), positioning, and interaction states (hovered, 
 * clicked, visible, turned).
 */
class Card : public CardBase {
public:

    /**
     * Constructs a Card with specified value, suit, and texture pointers.
     * @param value The value of the card.
     * @param suit The suit of the card.
     * @param deckTex Pointer to the texture containing the card fronts.
     * @param backTex Pointer to the texture for the card back.
     */
    Card(const int& value, const Suit& suit, const sf::Texture* deckTex, const sf::Texture* backTex);
    
    /**
     * Default constructor for an invisible, empty card.
     */
    Card() : Card(0, Suit::None, nullptr, nullptr) { isVisible = false; }

    /**
     * Sets the card to display its back texture.
     */
    void setBack();

    /**
     * Sets the card to display its front texture.
     */
    void setFront();

    /**
     * Sets the position of the card on the screen.
     * @param x The X coordinate.
     * @param y The Y coordinate.
     */
    void setPosition(const float x, const float y);

    /**
     * Renders the card onto the specified window.
     * @param window The SFML render window where the card will be drawn.
     */
    void draw(sf::RenderWindow& window) const;

    /**
     * Sets the scale factor for the card's sprite.
     * @param factor The scaling multiplier.
     */
    void setScale(const float factor);

    /**
     * Sets the origin of the card's sprite to its center.
     */
    void setCenterOrigin();

    /**
     * Retrieves the SFML sprite of the card.
     * @return The card's sprite object.
     */
    sf::Sprite getSprite() const { return sprite; };

    /**
     * Compares this card with another for sorting purposes in the player's hand.
     * @note This is ONLY used for visual sorting by suit and value.
     * @param other The other card to compare against.
     * @return True if this card should be placed before the other card.
     */
    bool operator<(const Card& other) const { return fakeValue(*this) < fakeValue(other); };

    /**
     * Checks if the mouse is currently hovering over the card.
     * @return True if hovered, false otherwise.
     */
    bool getHovered() const;

    /**
     * Checks if the card is currently clicked.
     * @return True if clicked, false otherwise.
     */
    bool getClicked() const { return isClicked; };

    /**
     * Checks if the card is visible on the screen.
     * @return True if visible, false otherwise.
     */
    bool getVisible() const { return isVisible; };

    /**
     * Checks if the card is turned face up.
     * @return True if turned face up, false otherwise.
     */
    bool getTurned() const { return isTurned; };

    /**
     * Sets the hovered state of the card.
     * @param hovered The target hover state.
     */
    void setHovered(bool hovered) { isHovered = hovered; };

    /**
     * Sets the clicked state of the card.
     * @param clicked The target clicked state.
     */
    void setClicked(bool clicked) { isClicked = clicked; };

    /**
     * Changes the visibility of the card.
     * @param visible The target visibility state.
     */
    void setVisible(bool visible) { isVisible = visible; };

    /**
     * Sets the turned (face up/down) state of the card.
     * @param turned The target turned state.
     */
    void setTurned(bool turned) { isTurned = turned; }

private:

    /**
     * Calculates a fake value used exclusively by operator< to sort hands.
     * @param card The card to evaluate.
     * @return A calculated integer representing the sorting weight.
     */
    int fakeValue(const Card& card) const;

    sf::Sprite sprite;
    const sf::Texture* deckTexturePtr;
    const sf::Texture* backTexturePtr;

    bool isHovered = false;
    bool isClicked = false;
    bool isVisible = true;
    bool isTurned = false;
};

/**
 * Comparator used for actual game logic comparison of cards, taking the trump suit into account.
 */
class CardComparator {
public:
    Suit trump;
    
    /**
     * Constructs a CardComparator with a specific trump suit.
     * @param t The current trump suit.
     */
    CardComparator(Suit t) : trump(t) {}

    /**
     * Compares two cards based on value and trump suit rules.
     * @param a The first card.
     * @param b The second card.
     * @return True if card 'a' is strictly weaker than card 'b' in the context of the trump.
     */
    bool operator()(const Card& a, const Card& b) const {
        if (a.getSuit() != trump && b.getSuit() == trump) return true;
        if (a.getSuit() == trump && b.getSuit() != trump) return false;
        return a < b;
    }
};
