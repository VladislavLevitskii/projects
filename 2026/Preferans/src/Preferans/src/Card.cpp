#include "Card.h"

Card::Card(const int& val, const Suit& s, const sf::Texture* deckTex, const sf::Texture* backTex)
    : value(val), suit(s), deckTexturePtr(deckTex), backTexturePtr(backTex) {

    if (deckTexturePtr) {
        const_cast<sf::Texture*>(deckTexturePtr)->setSmooth(true);
    }
    setFront();
}

void Card::setPosition(const float x, const float y) {
    sprite.setPosition(x, y);
}

void Card::setScale(const float factor) {
    sprite.setScale(factor, factor);
}

void Card::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}

void Card::centerOrigin() {
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
}

void Card::setBack() {
    if (backTexturePtr) {
        sprite.setTexture(*backTexturePtr);
        sprite.setTextureRect(sf::IntRect(0, 0, CARD_WIDTH, CARD_HEIGHT));
    }
}

bool Card::getHovered() const {
    return isHovered;
}

int Card::realValue(const Card& card) const {
    int suitValue = 0;
    switch (card.getSuit()) {
    case Suit::Spades:
        suitValue = 1;
        break;
    case Suit::Clubs:
        suitValue = 10;
        break;
    case Suit::Diamonds:
        suitValue = 100;
        break;
    case Suit::Hearts:
        suitValue = 1000;
        break;
    }
    return (suitValue * card.getValue());
}

void Card::setFront() {
    if (deckTexturePtr) {
        sprite.setTexture(*deckTexturePtr);
        int spriteIndex = (value == CARDS_ONE_SUIT + 1) ? 0 : value - 1;
        int row = 0;
        switch (suit) {
        case Suit::Clubs: row = 0; break;
        case Suit::Diamonds: row = 1; break;
        case Suit::Hearts: row = 2; break;
        case Suit::Spades: row = 3; break;
        }
        sprite.setTextureRect(sf::IntRect(spriteIndex * CARD_WIDTH, row * CARD_HEIGHT, CARD_WIDTH, CARD_HEIGHT));
    }
}