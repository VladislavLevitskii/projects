#include "Button.h"

void Button::setSize(const sf::Vector2f& size) {
    shape.setSize(size);
}

void Button::setPosition(const sf::Vector2f& pos) {
    shape.setPosition(pos);
}

void Button::setText(const std::string& str, const sf::Font& font, unsigned int size) {
    text.setString(str);
    text.setFont(font);
    text.setCharacterSize(size);
    sf::FloatRect rect = text.getLocalBounds();
    text.setOrigin(rect.left + rect.width / 2.f, rect.top + rect.height / 2.f);
    text.setPosition(shape.getPosition().x + shape.getSize().x / 2.f,
        shape.getPosition().y + shape.getSize().y / 2.f);
}

void Button::setFillColor(const sf::Color& color) {
    fillColor = color;
    shape.setFillColor(fillColor);
}

void Button::setHoverColor(const sf::Color& color) {
    hoverColor = color;
}

bool Button::isHovered(const sf::RenderWindow& window) const {
    auto mp = sf::Mouse::getPosition(window);
    return shape.getGlobalBounds().contains((float)mp.x, (float)mp.y);
}

bool Button::isClicked(const sf::RenderWindow& window) const {
    return isHovered(window) && sf::Mouse::isButtonPressed(sf::Mouse::Left);
}

void Button::update(const sf::RenderWindow& window) {
    if (isHovered(window)) shape.setFillColor(hoverColor);
    else shape.setFillColor(fillColor);
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape);
    window.draw(text);
}

sf::Vector2f Button::getPosition() {
    return shape.getPosition();
}

sf::Vector2f Button::getSize() {
    return shape.getSize();
}