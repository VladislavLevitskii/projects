#include "../include/UI.hpp"

UI::UI(sf::Window& window)
    : options{ {&easyButton, GameDifficulty::Easy}, {&mediumButton, GameDifficulty::Medium}, {&hardButton, GameDifficulty::Hard} } {

    windowCenterX = window.getSize().x / 2.f;
    font.loadFromFile("fonts/CascadiaMono-Regular.ttf");

    auto setupBtn = [&](auto& btn, sf::Vector2f pos, sf::Vector2f size, const std::string& text, sf::Color col, sf::Color hov, unsigned int fontSize = CHARACTER_SIZE_NORMAL) {
        btn.setSize(size);
        btn.setPosition(pos);
        btn.setText(text, font, fontSize);
        btn.setFillColor(col);
        btn.setHoverColor(hov);
        };

    const float wHeight = (float)window.getSize().y;
    const float startY = wHeight / 2.f;
    const float spacing = 10.f;

    const float midStartX = windowCenterX - START_BUTTON_SIZE.x / 2.f;
    const float midDiffX = windowCenterX - BUTTON_DIFFICULTY_SIZE.x / 2.f;

    setupBtn(startButton, { midStartX, wHeight - PADDING_BOTTOM * 2 }, START_BUTTON_SIZE, "Start", BUTTON_START_DIFFICULTY_COLOR, BUTTON_START_DIFFICULTY_COLOR_HOVER, CHARACTER_SIZE_BIG);
    setupBtn(easyButton, { midDiffX,  wHeight - PADDING_BOTTOM * 3 }, BUTTON_DIFFICULTY_SIZE, "Easy", BUTTON_EASY_DIFFICULTY_COLOR, BUTTON_EASY_DIFFICULTY_COLOR_HOVER);
    setupBtn(mediumButton, { midDiffX,  wHeight - PADDING_BOTTOM * 4 }, BUTTON_DIFFICULTY_SIZE, "Medium", BUTTON_MEDIUM_DIFFICULTY_COLOR, BUTTON_MEDIUM_DIFFICULTY_COLOR_HOVER);
    setupBtn(hardButton, { midDiffX,  wHeight - PADDING_BOTTOM * 5 }, BUTTON_DIFFICULTY_SIZE, "Hard", BUTTON_HARD_DIFFICULTY_COLOR, BUTTON_HARD_DIFFICULTY_COLOR_HOVER);
    setupBtn(restartButton, { midStartX, startY - START_BUTTON_SIZE.y / 2.f }, START_BUTTON_SIZE, "New Game", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);

    difficultyText.setFont(font);
    difficultyText.setCharacterSize(CHARACTER_SIZE_BIG);
    difficultyText.setFillColor(DIFFICULTY_TEXT_COLOR);
    difficultyText.setString(DIFFICULTY_EASY_STRING);
    auto bounds = difficultyText.getLocalBounds();
    difficultyText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    difficultyText.setPosition(windowCenterX, DIFFICULTY_TEXT_POSITION_Y);

    setupBtn(biddingSelectGame, { midStartX, startY - 100.f }, START_BUTTON_SIZE, "6 spades", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);
    setupBtn(biddingPass, { midStartX, startY - 25.f }, START_BUTTON_SIZE, "Pass", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);
    setupBtn(biddingMisere, { midStartX, startY + 50.f }, START_BUTTON_SIZE, "Misere", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);

    setupBtn(changeBidToLeft, { windowCenterX - START_BUTTON_SIZE.x * 1.5f - spacing, startY }, START_BUTTON_SIZE, "<", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);
    setupBtn(changeBidToRight, { windowCenterX + START_BUTTON_SIZE.x * 0.5f + spacing, startY }, START_BUTTON_SIZE, ">", BUTTON_RESTART_COLOR, BUTTON_RESTART_COLOR_HOVER);

    setupBtn(selectThisBid, { midStartX, startY }, START_BUTTON_SIZE, "Bid", BUTTON_EASY_DIFFICULTY_COLOR, BUTTON_EASY_DIFFICULTY_COLOR_HOVER);
}

void UI::drawMenu(sf::RenderWindow& window, GameDifficulty& difficulty, GameState& gameState, sf::Clock& aiClock) {
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
        aiClock.restart();
        gameState = GameState::Game;
        changedDifficulty = true;
    }
}

void UI::drawTalon(Talon& talon, const float& shift, sf::RenderWindow& window) {
    float x = talon.position.x;
    float y = talon.position.y;
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

void UI::drawScore(PlayerList& listOfPlayers, sf::RenderWindow& window) {
    for (auto&& player : listOfPlayers) {
        window.draw(player->scoreDisplay);
    }
}

void UI::drawCards(PlayerList& listOfPlayers, const float& shift, sf::RenderWindow& window) {
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

bool UI::drawSelectingRegime(sf::RenderWindow& window, PlayerList& listOfPlayers, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid) {
    drawTable(window, listOfPlayers, talon, discardPile);

    static Bid lastBid = biddingGrid.getCurrentBid();
    if (biddingGrid.getCurrentBid() == FINAL_BID) {
        listOfPlayers[0]->playerWantedRegime = GameType::AllPass;
        return false;
    }

    if (lastBid != biddingGrid.getCurrentBid()) {
        std::string textForButton = std::to_string(biddingGrid.getCurrentBid().getValue()) + " ";
        textForButton += suitToString(biddingGrid.getCurrentBid().getSuit());
        biddingSelectGame.setText(textForButton, font, CHARACTER_SIZE_NORMAL);
        lastBid = biddingGrid.getCurrentBid();
    }

    biddingSelectGame.update(window);       biddingSelectGame.draw(window);
    biddingMisere.update(window);           biddingMisere.draw(window);
    biddingPass.update(window);             biddingPass.draw(window);

    if (biddingPass.isClicked(window)) {
        listOfPlayers[0]->playerWantedRegime = GameType::AllPass; 
        return true;
    } else if (biddingMisere.isClicked(window)) {
        listOfPlayers[0]->playerWantedRegime = GameType::Misere;
        return true;
    } else if (biddingSelectGame.isClicked(window)) {
        listOfPlayers[0]->playerWantedRegime = GameType::Contract;
        listOfPlayers[0]->bidForContract = biddingGrid.getCurrentBid();
        biddingGrid.increaseBid();
        return true;
    }

    return false;

}

bool UI::drawSelectingRegimeBid(PlayerList& listOfPlayers, sf::RenderWindow& window, Talon& talon, DiscardPile& discardPile, BiddingGrid& biddingGrid, int thrownCards) {
    drawTable(window, listOfPlayers, talon, discardPile);

    if (!isLockedGrid) {
        isLockedGrid = true;
        lockedGrid = biddingGrid.getCurrentBid();
    }
    
    if (!loweredBid) {
        biddingGrid.decreaseBid();
        lockedGrid = biddingGrid.getCurrentBid();
        loweredBid = true;
    }

    if (thrownCards == 2) {
        selectThisBid.setFillColor(BUTTON_HARD_DIFFICULTY_COLOR);
        selectThisBid.setHoverColor(BUTTON_HARD_DIFFICULTY_COLOR_HOVER);
    }

    std::string textForButton = std::to_string(biddingGrid.getCurrentBid().getValue()) + " " + suitToString(biddingGrid.getCurrentBid().getSuit());
    selectThisBid.setText(textForButton, font, CHARACTER_SIZE_NORMAL);

    for (auto* btn : { &changeBidToLeft, &changeBidToRight, &selectThisBid }) {
        btn->update(window);
        btn->draw(window);
    }

    if (changeBidToLeft.isClicked(window)) {
        if (biddingGrid.getCurrentBid() != lockedGrid) {
            biddingGrid.decreaseBid();
        }
        return false;
    }

    if (changeBidToRight.isClicked(window)) {
        biddingGrid.increaseBid();
        return false;
    }

    if (selectThisBid.isClicked(window) && listOfPlayers[0]->hand.size() == MAX_HAND_SIZE) {
        listOfPlayers[0]->bidForContract = biddingGrid.getCurrentBid();
        talon.hand.clear();
        return true;
    }
    
    return false;
}

void UI::drawRegimes(PlayerList& listOfPlayers, sf::RenderWindow& window) {

    for (auto&& player : listOfPlayers) {
        player->regimeDisplay.setString(gameTypeToString(player->playerWantedRegime, player->bidForContract));
    }

    for (auto&& player : listOfPlayers) {
        window.draw(player->regimeDisplay);
    }
}

void UI::drawFinalScore(sf::RenderWindow& window, PlayerList& listOfPlayers, GameState& gameState, GameType& gameType) {
    window.clear(sf::Color::Green);
    
    for (const auto& player : listOfPlayers) {
        window.draw(player->scoreDisplay);
    }

    restartButton.update(window);
    restartButton.draw(window);

    if (restartButton.isClicked(window)) {
        for (const auto& player : listOfPlayers) {
            player->updateScoreText();
        }
        gameState = GameState::Game;
        gameType = GameType::None;
        changedDifficulty = true;
    }
}

void UI::drawTable(sf::RenderWindow& window, PlayerList& listOfPlayers, Talon& talon, DiscardPile& discardPile) {
    window.clear(sf::Color::Green);
    static float shift = SHIFT_BETWEEN_CARDS;

    drawTalon(talon, shift, window);
    drawDiscardPile(discardPile, window);
    drawScore(listOfPlayers, window);
    drawCards(listOfPlayers, shift, window);
    drawRegimes(listOfPlayers, window);
}