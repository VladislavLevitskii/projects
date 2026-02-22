#include "Game.h"

Game::Game(sf::RenderWindow& win) :
    window(win),
    ui(win),
    gameState(GameState::Menu),
    difficulty(GameDifficulty::Easy),
    dealt(false),
    currentPlayerIdx(0),
    talon(TALON_POSITION_FOR_CARDS),
    discardPile(DISCARD_PILE_FOR_CARDS),
    timeRestarted(false),
    gameType(GameType::None)
    {
    if (!ui.backTexture.loadFromFile("images/back.png")) {
        throw std::runtime_error("Failed to load backface texture: images/back.png");
    }

    if (!ui.deckTexture.loadFromFile("images/deck.png")) {
        throw std::runtime_error("Failed to load deck texture: images/deck.png");
    }

    if (!ui.loadHandWave("images/handwave.png")) {
        throw std::runtime_error("Failed to load handwave texture: images/handwave.png");
    }

    if (!font.loadFromFile("fonts/CascadiaMono-Regular.ttf")) {
        throw std::runtime_error("Failed to load font: fonts/CascadiaMono-Regular.ttf");
    }

    for (int s = static_cast<int>(Suit::Spades); s <= static_cast<int>(Suit::Hearts); ++s) {
        for (int v = 7; v <= 14; ++v) {
            deck.emplace_back(v, static_cast<Suit>(s), &ui.deckTexture, &ui.backTexture);
        }
    }

    listOfPlayers.push_back(std::make_unique<Player>(PLAYER_POSITION_FOR_CARDS, PLAYER_POSITION_FOR_HANDS, PLAYER_POSITION_FOR_SCORE));
    listOfPlayers.push_back(std::make_unique<AI>(AI1_POSITION_FOR_CARDS, AI1_POSITION_FOR_HANDS, 1, AI1_POSITION_FOR_SCORE));
    listOfPlayers.push_back(std::make_unique<AI>(AI2_POSITION_FOR_CARDS, AI2_POSITION_FOR_HANDS, 2, AI2_POSITION_FOR_SCORE));

    for (auto& player : listOfPlayers) {
        player->initScoreDisplay(font);
        player->initIndicator(ui.handTexture);
    }
}

void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        update(ui);
        render(ui);
    }
}

int Game::playedCards() {
    return static_cast<int>(std::count_if(discardPile.hand.begin(), discardPile.hand.end(),
        [](const Card& c) { return c.getVisible(); }));
}

void Game::checkHovering(const int numberOfTrumps) {
    bool isAnyCardHovered = false;
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {        
        if (it->getSprite().getGlobalBounds().contains(mousePos) && !isAnyCardHovered && (trump == it->getSuit() || trump == Suit::None || numberOfTrumpCards == 0)) {
            it->setHovered(true);
            isAnyCardHovered = true;
        } else {
            it->setHovered(false);
        }
    }
}

void Game::checkClicked() {
    static bool mousePressedLastFrame = false;
    bool mousePressedNow = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool clicked = mousePressedNow && !mousePressedLastFrame;
    mousePressedLastFrame = mousePressedNow;
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {
        if (it->getSprite().getGlobalBounds().contains(mousePos) && it->getHovered() && clicked) {
            it->setClicked(true);
        } else {
            it->setClicked(false);
        }
    }
}

bool Game::dealCards() {
    for (auto&& player : listOfPlayers) {
        player->hand.clear();
    }
    talon.hand.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, NUMBER_OF_CARDS - 1);
    std::shuffle(deck.begin(), deck.end(), gen);
    for (size_t i = 0; i < NUMBER_OF_PLAYERS; i++) {
        for (size_t j = 0; j < MAX_HAND_SIZE; j++) {
            listOfPlayers[i]->hand.push_back(deck[i * MAX_HAND_SIZE + j]);
        }
    }

    for (auto&& player : listOfPlayers)
        std::sort(player->hand.begin(), player->hand.end());

    for (int i = MAX_HAND_SIZE * NUMBER_OF_PLAYERS; i < (MAX_HAND_SIZE * NUMBER_OF_PLAYERS) + NUMBER_OF_PLAYERS - 1; i++)
        talon.hand.push_back(deck[i]);

    for (auto&& card : talon.hand) {
        card.setTurned(false);
        card.setBack();
    }

    std::uniform_int_distribution<> distPlayer(0, NUMBER_OF_PLAYERS - 1);
    int firstPlayer = distPlayer(gen);
    listOfPlayers[firstPlayer]->hasMove = true;
    currentPlayerIdx = firstPlayer;
    round = 0;
    return true;
}

static std::string suitToString(Suit s) {
    switch (s) {
    case Suit::Spades:   return "Spades";
    case Suit::Clubs:    return "Clubs";
    case Suit::Diamonds: return "Diamonds";
    case Suit::Hearts:   return "Hearts";
    default:             return "None";
    }
}

class CardComparator {
public:
    Suit trump;
    CardComparator(Suit t) : trump(t) {}

    bool operator()(const Card& a, const Card& b) const {
        if (a.getSuit() != trump && b.getSuit() == trump) return true;
        if (a.getSuit() == trump && b.getSuit() != trump) return false;
        return a < b;
    }
};

void Game::calculateScore() {
    auto it = std::max_element(discardPile.hand.begin(), discardPile.hand.end(), CardComparator(trump));
    size_t indexMax = std::distance(discardPile.hand.begin(), it);
    discardPile.setCardsInvisible();
    ++listOfPlayers[indexMax].get()->tricks;
    listOfPlayers[indexMax].get()->updateScoreText();
    listOfPlayers[indexMax].get()->hasMove = true;
    listOfPlayers[currentPlayerIdx].get()->hasMove = false;
    currentPlayerIdx = indexMax;
    ++round;
    if (talon.hand.size() != 0) {
        talon.hand.pop_back();
    }
    turnedTalonCard = false;
    trump = Suit::None;
    calculatedTrump = false;
    aiClock.restart();
}

void Game::passMove() {
    listOfPlayers[currentPlayerIdx]->hasMove = false;
    currentPlayerIdx = (currentPlayerIdx + 1) % NUMBER_OF_PLAYERS;
    if (playedCards() != NUMBER_OF_PLAYERS) {
        listOfPlayers[currentPlayerIdx].get()->hasMove = true;
    }
    aiClock.restart();
}

void Game::processAllPass() {
    if (round == MAX_HAND_SIZE) {
        round = 0;
        gameState = GameState::Menu;
        dealt = false;
        return;
    }

    if (round < 2 && !turnedTalonCard) {
        talon.hand.back().setTurned(true);
        trump = talon.hand.back().getSuit();
        calculatedTrump = false;
        turnedTalonCard = true;
    }

    if (playedCards() == NUMBER_OF_PLAYERS) {
        calculateScore();
        return;
    }

    if (currentPlayerIdx != 0) {
        Suit tmpTrump = trump;
        listOfPlayers[currentPlayerIdx].get()->makeMoveAllPass(difficulty, discardPile, trump);
        if (trump != tmpTrump) {
            calculatedTrump = false;
        }
        passMove();
    } else {
        if (!timeRestarted) {
            clockForStartingGame.restart();
            timeRestarted = true;
        }

        if (!calculatedTrump) {
            numberOfTrumpCards = listOfPlayers[0].get()->getNumberOfTrumpCards(trump);
            calculatedTrump = true;
        }

        if (timeRestarted && clockForStartingGame.getElapsedTime().asSeconds() > TIME_BETWEEN_MOVE) {
            checkHovering(numberOfTrumpCards);
            checkClicked();

            for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {
                if (it->getClicked() && it->getHovered()) {
                    if (trump == Suit::None) {
                        calculatedTrump = false;
                        trump = it->getSuit();
                    }
                    discardPile.hand[0] = std::move(*it);
                    discardPile.hand[0].setVisible(true);
                    discardPile.hand[0].setFront();
                    listOfPlayers[0].get()->hand.erase(std::next(it).base());
                    passMove();
                    break;
                }
            }
        }

    }

    if (playedCards() == 1 && round > 2 && trump == Suit::None) {
        auto it = std::find_if(discardPile.hand.begin(), discardPile.hand.end(),
            [](const Card& c) {
                return c.getVisible();
            });
        if (it != discardPile.hand.end()) {
            trump = it->getSuit();
            calculatedTrump = false;
        }
    }
}

void Game::update(UI& ui) {
    switch (gameState) {
    case GameState::Menu:
        break;

    case GameState::Game:
        if (!dealt) { dealt = dealCards(); }
        if (aiClock.getElapsedTime().asSeconds() < TIME_BETWEEN_MOVE) { return; }
        //if (gameType == GameType::None) { gameType = GameType::Bidding; }
        switch (gameType) {
            case GameType::None:
                gameType = GameType::AllPass; // for now
                break;
            case GameType::Bidding:
                break;
            case GameType::AllPass:
                processAllPass();
                break;
            case GameType::Contract:
                break;
            case GameType::Misere:
                break;
            default:
                break;
        }
        break;

    case GameState::End:
        break;
    }
}

void Game::render(UI& ui) {
    switch (gameState) {
    case GameState::Menu:
        ui.drawMenu(window, difficulty, gameState);
        break;

    case GameState::Game:
        ui.drawTable(window, listOfPlayers, talon, discardPile);
        break;

    case GameState::End:
        break;
    }

    window.display();
}
