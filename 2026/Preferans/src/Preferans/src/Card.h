#pragma once
#include <SFML/Graphics.hpp>
#include "GameConfig.h"

class Card {
public:

    Card(const int& value, const Suit& suit, const sf::Texture* deckTex, const sf::Texture* backTex);
    Card() : Card(0, Suit::None, nullptr, nullptr) { isVisible = false; }

    void setBack();
    void setFront();
    void setPosition(const float x, const float y);
    void draw(sf::RenderWindow& window) const;
    void setScale(const float factor);
    void centerOrigin();
    int getValue() const { return (value == 1) ? CARDS_ONE_SUIT - 1 : value; };
    Suit getSuit() const { return suit; };
    sf::Sprite getSprite() const { return sprite; };
    bool operator<(const Card& other) const { return realValue(*this) < realValue(other); };
    bool getHovered() const;
    bool getClicked() const { return isClicked; };
    bool getVisible() const { return isVisible; };
    bool getTurned() const { return isTurned; };
    void setHovered(bool value) { isHovered = value; };
    void setClicked(bool value) { isClicked = value; };
    void setVisible(bool value) { isVisible = value; };
    void setTurned(bool value) { isTurned = value; }

private:

    // this function is used only to sort hands by suit, otherwise it's completely useless, use getValue instead
    int realValue(const Card& card) const;

    sf::Sprite sprite;
    int value;
    Suit suit;
    const sf::Texture* deckTexturePtr;
    const sf::Texture* backTexturePtr;

    bool isHovered = false;
    bool isClicked = false;
    bool isVisible = true;
    bool isTurned = false;
};