#pragma once
#include <SFML/Graphics.hpp>
#include <string>

static const sf::Color GREY { 120, 120, 120 };
static const sf::Color LIGHT_GREY { 150, 150, 150 };

class Button {
public:
    Button() = default;

    void setSize(const sf::Vector2f& size);
    sf::Vector2f getSize();
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition();
    void setText(const std::string& str, const sf::Font& font, unsigned int size);
    void setFillColor(const sf::Color& color);
    void setHoverColor(const sf::Color& color);

    void draw(sf::RenderWindow& window) const;
    void update(const sf::RenderWindow& window);
    bool isHovered(const sf::RenderWindow& window) const;
    bool isClicked(const sf::RenderWindow& window) const;

private:
    sf::RectangleShape shape;
    sf::Text text;

    sf::Color fillColor = GREY;
    sf::Color hoverColor = LIGHT_GREY;
};
