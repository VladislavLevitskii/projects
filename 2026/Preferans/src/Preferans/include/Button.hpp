#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "GameConfig.hpp"

static const sf::Color GREY { 120, 120, 120 };
static const sf::Color LIGHT_GREY { 150, 150, 150 };

/**
 * @class Button
 * @brief A graphical user interface component.
 * Wraps SFML shapes and text to create a clickable button. Handles its own 
 * rendering, text alignment, and mouse interaction states (hovering, clicking).
 */
class Button {
public:
    Button() = default;

    /**
     * Sets the size of the button.
     * @param size The target size of the button as a 2D vector.
     */
    void setSize(const sf::Vector2f& size);

    /**
     * Retrieves the current size of the button.
     * @return The size of the button as a 2D vector.
     */
    sf::Vector2f getSize();

    /**
     * Sets the position of the button on the screen.
     * @param pos The target position coordinates.
     */
    void setPosition(const sf::Vector2f& pos);

    /**
     * Retrieves the current position of the button.
     * @return The position coordinates of the button.
     */
    sf::Vector2f getPosition();

    /**
     * Sets the text displayed inside the button.
     * @param str The string of text to display.
     * @param font The SFML font to be used for the text.
     * @param size The character size of the text.
     */
    void setText(const std::string& str, const sf::Font& font, unsigned int size);

    /**
     * Sets the default fill color of the button.
     * @param color The target fill color.
     */
    void setFillColor(const sf::Color& color);

    /**
     * Sets the color of the button when the mouse hovers over it.
     * @param color The target hover color.
     */
    void setHoverColor(const sf::Color& color);

    /**
     * Draws the button and its text to the specified window.
     * @param window The SFML render window where the button will be drawn.
     */
    void draw(sf::RenderWindow& window) const;

    /**
     * Updates the button state, checking if it is hovered and changing its color accordingly.
     * @param window The SFML render window used to check the mouse position.
     */
    void update(const sf::RenderWindow& window);

    /**
     * Checks if the mouse cursor is currently hovering over the button.
     * @param window The SFML render window used to check the mouse position.
     * @return True if the button is hovered by the mouse, false otherwise.
     */
    bool isHovered(const sf::RenderWindow& window) const;

    /**
     * Checks if the button was clicked.
     * @note This function ensures a single trigger per click. If the user holds the 
     * mouse button down, subsequent calls will return false until the button is released and clicked again.
     * @param window The SFML render window used to check the mouse state.
     * @return True if the button was just clicked, false otherwise.
     */
    bool isClicked(const sf::RenderWindow& window);

private:
    bool lastTimePressed = false;
    sf::RectangleShape shape;
    sf::Text text;

    sf::Color fillColor = GREY;
    sf::Color hoverColor = LIGHT_GREY;
};
