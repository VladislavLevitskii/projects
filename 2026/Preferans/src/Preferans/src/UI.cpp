#include "UI.h"

UI::UI(sf::Window& window)
    : options{ {&easyButton, GameDifficulty::Easy}, {&mediumButton, GameDifficulty::Medium}, {&hardButton, GameDifficulty::Hard} } {
    windowCenter = window.getSize().x / 2.f;
    font.loadFromFile("fonts/CascadiaMono-Regular.ttf");

    auto setupBtn = [&](auto& btn, std::string text, float yOffset, sf::Color col, sf::Color hov, sf::Vector2f size) {
        btn.setSize(size);
        btn.setPosition({ windowCenter - size.x / 2.f, window.getSize().y - yOffset });
        btn.setText(text, font, (text == "Start" ? CHARACTER_SIZE_BIG : CHARACTER_SIZE_NORMAL));
        btn.setFillColor(col);
        btn.setHoverColor(hov);
        };

    setupBtn(startButton,   "Start",    PADDING_BOTTOM * 2, BUTTON_START_DIFFICULTY_COLOR,  BUTTON_START_DIFFICULTY_COLOR_HOVER,    START_BUTTON_SIZE);
    setupBtn(easyButton,    "Easy",     PADDING_BOTTOM * 3, BUTTON_EASY_DIFFICULTY_COLOR,   BUTTON_EASY_DIFFICULTY_COLOR_HOVER,     BUTTON_DIFFICULTY_SIZE);
    setupBtn(mediumButton,  "Medium",   PADDING_BOTTOM * 4, BUTTON_MEDIUM_DIFFICULTY_COLOR, BUTTON_MEDIUM_DIFFICULTY_COLOR_HOVER,   BUTTON_DIFFICULTY_SIZE);
    setupBtn(hardButton,    "Hard",     PADDING_BOTTOM * 5, BUTTON_HARD_DIFFICULTY_COLOR,   BUTTON_HARD_DIFFICULTY_COLOR_HOVER,     BUTTON_DIFFICULTY_SIZE);

    difficultyText.setFont(font);
    difficultyText.setCharacterSize(CHARACTER_SIZE_BIG);
    difficultyText.setFillColor(DIFFICULTY_TEXT_COLOR);
    difficultyText.setString(DIFFICULTY_EASY_STRING);
    auto bounds = difficultyText.getLocalBounds();
    difficultyText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    difficultyText.setPosition(windowCenter, DIFFICULTY_TEXT_POSITION_Y);
}

void UI::drawMenu(sf::RenderWindow& window, GameDifficulty& difficulty, GameState& gameState) {
    window.clear(sf::Color::Black);

    if (changedDifficulty) {
        switch (difficulty) {
            case GameDifficulty::Easy: difficultyText.setString(DIFFICULTY_EASY_STRING); break;
            case GameDifficulty::Medium: difficultyText.setString(DIFFICULTY_MEDIUM_STRING); break;
            case GameDifficulty::Hard: difficultyText.setString(DIFFICULTY_HARD_STRING); break;
        }
        changedDifficulty = false;
    }

    window.draw(difficultyText);

    easyButton.update(window);      easyButton.draw(window);
    mediumButton.update(window);    mediumButton.draw(window);
    hardButton.update(window);      hardButton.draw(window);
    startButton.update(window);     startButton.draw(window);

    for (auto&& opt : options) {
        if (opt.button->isClicked(window)) {
            difficulty = opt.value;
            changedDifficulty = true;
            break;
        }
    }

    if (startButton.isClicked(window)) {
        gameState = GameState::Game;
        changedDifficulty = true;
    }
}

void UI::drawTalon(Talon& talon, const float& shift, sf::RenderWindow& window) {
    float x = talon.positionForCards.x;
    float y = talon.positionForCards.y;
    for (auto&& card : talon.hand) {
        card.setPosition(x, y);
        card.setScale(0.25f);
        if (card.getTurned())
            card.setFront();
        card.draw(window);
        x += shift;
    }
}

void UI::drawDiscardPile(DiscardPile& discardPile, sf::RenderWindow& window) {
    for (int i = (int)discardPile.hand.size() - 1; i >= 0; --i) {
        auto& card = discardPile.hand[i];

        if (!card.getVisible()) {
            continue;
        }

        if (i < NUMBER_OF_PLAYERS) {
            card.setPosition(discardPile.slotPositions[i].x, discardPile.slotPositions[i].y);
        }

        card.setScale(0.25f);
        card.setFront();
        card.draw(window);
    }
}

void UI::drawScore(std::vector<std::unique_ptr<Player>>& listOfPlayers, sf::RenderWindow& window) {
    for (auto&& player : listOfPlayers) {
        window.draw(player->scoreDisplay);
    }
}

void UI::drawCards(std::vector<std::unique_ptr<Player>>& listOfPlayers, const float& shift, sf::RenderWindow& window) {
    for (size_t indexPlayer = 0; indexPlayer < NUMBER_OF_PLAYERS; ++indexPlayer) {
        auto&& player = listOfPlayers[indexPlayer];
        float x = player->positionForCards.x;
        float y = player->positionForCards.y;

        Card tmpCard = Card(4, Suit::Hearts, nullptr, nullptr);
        for (auto it = player->hand.begin(); it != player->hand.end(); ) {
            Card& card = *it;

            if (card.getSuit() != tmpCard.getSuit() && indexPlayer != 0) {
                y += 10;
            } else if (card.getSuit() != tmpCard.getSuit()) {
                x += 50;
            }

            if (card.getHovered()) {
                card.setPosition(x, y - 20);
            } else {
                card.setPosition(x, y);
            }

            card.setScale(0.25f);
            if (indexPlayer != 0) card.setBack();
            card.draw(window);
            tmpCard = card;

            if (indexPlayer == 0) x += shift;
            else y += shift;

            ++it;
        }

        if (player->hasMove) {
            window.draw(player->turnIndicator);
        }
    }
}

void UI::drawTable(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>>& listOfPlayers, Talon& talon, DiscardPile& discardPile) {
    window.clear(sf::Color::Green);
    static float shift = 50.f;

    drawTalon(talon, shift, window);
    drawDiscardPile(discardPile, window);
    drawScore(listOfPlayers, window);
    drawCards(listOfPlayers, shift, window);
}