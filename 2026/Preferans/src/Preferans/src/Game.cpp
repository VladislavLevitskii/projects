#include "../include/Game.hpp"

Game::Game(sf::RenderWindow& win) : window(win), ui(win) {
    initResources();

    for (int s = static_cast<int>(Suit::Spades); s <= static_cast<int>(Suit::Hearts); ++s) {
        for (int v = 7; v <= 14; ++v) {
            deck.emplace_back(v, static_cast<Suit>(s), &ui.deckTexture, &ui.backTexture);
        }
    }

    listOfPlayers.push_back(std::make_unique<HumanPlayer>(PLAYER_POSITION_FOR_CARDS, PLAYER_POSITION_FOR_HANDS, PLAYER_POSITION_FOR_SCORE, PLAYER_POSITION_FOR_REGIME));
    listOfPlayers.push_back(std::make_unique<AI>(AI1_POSITION_FOR_CARDS, AI1_POSITION_FOR_HANDS, 1, AI1_POSITION_FOR_SCORE, AI1_POSITION_FOR_REGIME));
    listOfPlayers.push_back(std::make_unique<AI>(AI2_POSITION_FOR_CARDS, AI2_POSITION_FOR_HANDS, 2, AI2_POSITION_FOR_SCORE, AI2_POSITION_FOR_REGIME));

    for (auto& player : listOfPlayers) {
        player->initScoreDisplay(font);
        player->initGameModeDisplay(font);
        player->initIndicator(ui.handTexture);
    }
}

void Game::initResources() {
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
}

void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        update();
        render();
    }
}

int Game::playedCards() {
    return static_cast<int>(std::count_if(discardPile.hand.begin(), discardPile.hand.end(),
        [](const Card& c) { return c.getVisible(); }));
}

void Game::checkHoveringAllPass() {
    bool isAnyCardHovered = false;
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {        
        if (it->getSprite().getGlobalBounds().contains(mousePos) && !isAnyCardHovered && 
            (trumpAllPass == it->getSuit() || trumpAllPass == Suit::None || numberOfTrumpCardsAllPass == 0)) {
            it->setHovered(true);
            isAnyCardHovered = true;
        } else {
            it->setHovered(false);
        }
    }
}

void Game::checkHoveringContract() {
    bool isAnyCardHovered = false;
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {
        if (isAnyCardHovered || !it->getSprite().getGlobalBounds().contains(mousePos)) {
            it->setHovered(false);
            continue;
        }

        bool isSuitPlayable = (trumpAllPass == it->getSuit() || trumpAllPass == Suit::None) ||
            (numberOfTrumpCardsAllPass == 0 && numberOfTrumpCardsContract == 0) ||
            (trumpContract == it->getSuit() && numberOfTrumpCardsAllPass == 0);

        if (isSuitPlayable) {
            it->setHovered(true);
            isAnyCardHovered = true;
        } else {
            it->setHovered(false);
        }
    }
}

void Game::checkCardClicked() {
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

void Game::endRound(size_t indexMax) {
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
    trumpAllPass = Suit::None;
    calculatedTrump = false;
    aiClock.restart();
}

void Game::calculateScoreAllPass() {
    auto it = std::max_element(discardPile.hand.begin(), discardPile.hand.end(), CardComparator(trumpAllPass));
    size_t indexMax = std::distance(discardPile.hand.begin(), it);
    endRound(indexMax);
}

void Game::calculateScoreContract() {
    auto it = std::max_element(discardPile.hand.begin(), discardPile.hand.end(), CardComparator(trumpContract));
    if ((*it).getSuit() != trumpContract) {
        it = std::max_element(discardPile.hand.begin(), discardPile.hand.end(), CardComparator(trumpAllPass));
    }
    size_t indexMax = std::distance(discardPile.hand.begin(), it);
    endRound(indexMax);
}

void Game::passMove() {
    listOfPlayers[currentPlayerIdx]->hasMove = false;
    currentPlayerIdx = (currentPlayerIdx + 1) % NUMBER_OF_PLAYERS;
    if (playedCards() != NUMBER_OF_PLAYERS) {
        listOfPlayers[currentPlayerIdx].get()->hasMove = true;
    }
    aiClock.restart();
}

void Game::recalculateTotal() {
    auto it = std::min_element(listOfPlayers.begin(), listOfPlayers.end(),
        [](const std::unique_ptr<Player>& A, const std::unique_ptr<Player>& B) {
            return A->penalty < B->penalty;
        });

    assert((it != listOfPlayers.end()));

    int minPenalty = (*it)->penalty;
    int sumAdjustedPenalty = 0;
    for (const auto& p : listOfPlayers) {
        sumAdjustedPenalty += (p->penalty - minPenalty);
    }
    int averageMountainCompensation = (sumAdjustedPenalty * MULTIPLICATION_FACTOR_MOUNTAIN) / NUMBER_OF_PLAYERS;

    for (auto&& p : listOfPlayers) {
        int scoreFromBullet = p->bullet * MULTIPLICATION_FACTOR_MOUNTAIN;

        int myAdjustedPenalty = p->penalty - minPenalty;
        int scoreFromMountain = averageMountainCompensation - (myAdjustedPenalty * MULTIPLICATION_FACTOR_MOUNTAIN);

        p->total = scoreFromBullet + scoreFromMountain + p->whists;
        p->updateFullInfo();
    }
}

void Game::processAllPass() {
    if (round == MAX_HAND_SIZE) {
        round = 0;
        gameState = GameState::End;
        for (auto&& player : listOfPlayers) {
            player->penalty += player->tricks;
            player->tricks = 0;
        }
        recalculateTotal();
        dealt = false;
        return;
    }

    if (round < 2 && !turnedTalonCard) {
        talon.hand.back().setTurned(true);
        trumpAllPass = talon.hand.back().getSuit();
        calculatedTrump = false;
        turnedTalonCard = true;
    }

    if (playedCards() == NUMBER_OF_PLAYERS) {
        calculateScoreAllPass();
        return;
    }

    if (currentPlayerIdx != 0) {
        Suit tmpTrump = trumpAllPass;
        listOfPlayers[currentPlayerIdx].get()->makeMoveAllPass(difficulty, discardPile, trumpAllPass, gen, gameType, *this);
        if (trumpAllPass != tmpTrump) {
            calculatedTrump = false;
        }
        passMove();
        return;
    }

    if (!calculatedTrump) {
        numberOfTrumpCardsAllPass = listOfPlayers[0].get()->getNumberOfTrumpCards(trumpAllPass);
        calculatedTrump = true;
    }

    checkHoveringAllPass();
    checkCardClicked();
    iterateCardsAndPlay();
}

bool Game::everyoneChoosedRegime() {
    if (!hasEveryoneChoosedRegime) {
        hasEveryoneChoosedRegime = std::all_of(listOfPlayers.begin(), listOfPlayers.end(), [](const auto& player) {
            return player->playerWantedRegime != GameType::None;
            });
    }
    return hasEveryoneChoosedRegime;
}

void Game::processBidding() {
    if (currentPlayerIdx != 0) {
        listOfPlayers[currentPlayerIdx].get()->makeMoveBid(difficulty, discardPile, gen, biddingGrid, *this);
        passMove();
    } if (tookActionBidding) {
        passMove();
        tookActionBidding = false;
    }

    // checks for misere
    if (std::count_if(listOfPlayers.begin(), listOfPlayers.end(), [](const auto& player) {
        return player->playerWantedRegime == GameType::Misere;
        }) >= 1) {
        gameType = GameType::Misere;
        passMove();
        return;
    }

    if (everyoneChoosedRegime()) {
        if (std::count_if(listOfPlayers.begin(), listOfPlayers.end(), [](const auto& player) {
            return player->playerWantedRegime == GameType::Contract;
            }) == 1) {
            gameType = GameType::Contract;
            return;
        }

        if (std::count_if(listOfPlayers.begin(), listOfPlayers.end(), [](const auto& player) {
            return player->playerWantedRegime == GameType::AllPass;
            }) == NUMBER_OF_PLAYERS) {
            gameType = GameType::AllPass;
            return;
        }

    }
}

void Game::playerTakeTalon() {
    for (auto&& card : talon.hand) {
        card.setFront();
        listOfPlayers[0]->hand.push_back(std::move(card));
    }
    std::sort(listOfPlayers[0]->hand.begin(), listOfPlayers[0]->hand.end());
}

bool Game::processChoosingCards() {
    if (playingIdxContract == -1) {
        playingIdxContract = (int)std::distance(listOfPlayers.begin(), std::find_if(listOfPlayers.begin(), listOfPlayers.end(), [](auto&& player) {
            return player->playerWantedRegime == GameType::Contract;
            }));
    }

    if (playingIdxContract != 0) {
        listOfPlayers[playingIdxContract].get()->makeFinalBid(difficulty, talon, gen, biddingGrid, *this);
        return true;
    } else {
        if (!talonDistributed) {
            playerTakeTalon();
            talonDistributed = true;
        }

        if (thrownCards == 2) {
            return false;
        }

        checkHoveringAllPass();
        checkCardClicked();
        iterateCardsAndThrow();

    }
    return false;
}

void Game::iterateCardsAndPlay() {
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {
        if (it->getClicked() && it->getHovered()) {
            if (trumpAllPass == Suit::None) {
                calculatedTrump = false;
                trumpAllPass = it->getSuit();
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

void Game::recalculateTotalContract() {
    auto&& declarer = listOfPlayers[playingIdxContract].get();
    int contractTricks = declarer->bidForContract.getValue();
    int contractValue = static_cast<int>(declarer->bidForContract.getValue()) - NUMBER_OF_LEVEL_BID;

    if (declarer->tricks >= contractTricks) {
        declarer->bullet += contractValue;
    } else {
        int missingTricks = contractTricks - declarer->tricks;
        declarer->penalty += missingTricks * contractValue;
    }

    for (auto&& player : listOfPlayers) {
        if (player.get() != declarer) {
            player->whists += player->tricks * contractValue;
        }
        player->tricks = 0;
    }

    recalculateTotal();
}

void Game::processContract() {
    // this happens only once before the contract game itself 
    if (!finilizedBidding) {
        finilizedBidding = processChoosingCards();
        return;
    }

    trumpContract = biddingGrid.getCurrentBid().getSuit();

    if (round == MAX_HAND_SIZE) {
        round = 0;
        gameState = GameState::End;
        recalculateTotalContract();
        dealt = false;
        return;
    }

    if (playedCards() == NUMBER_OF_PLAYERS) {
        calculateScoreContract();
        return;
    }

    if (currentPlayerIdx != 0) {
        listOfPlayers[currentPlayerIdx].get()->makeMoveContract(difficulty, discardPile, gen, biddingGrid, trumpAllPass, trumpContract, gameType, *this);
        passMove();
        return;
    }

    if (!calculatedTrump) {
        numberOfTrumpCardsAllPass = listOfPlayers[0].get()->getNumberOfTrumpCards(trumpAllPass);
        numberOfTrumpCardsContract = listOfPlayers[0].get()->getNumberOfTrumpCards(trumpContract);
        calculatedTrump = true;
    }

    checkHoveringContract();
    checkCardClicked();
    iterateCardsAndPlay();
}

void Game::iterateCardsAndThrow() {
    for (auto&& it = listOfPlayers[0].get()->hand.rbegin(); it != listOfPlayers[0].get()->hand.rend(); ++it) {
        if (it->getClicked() && it->getHovered()) {
            listOfPlayers[0].get()->hand.erase(std::next(it).base());
            ++thrownCards;
            break;
        }
    }
}

void Game::recalculateTotalMisere() {
    auto&& declarer = listOfPlayers[playingIdxContract].get();

    if (declarer->tricks == 0) {
        declarer->bullet += MULTIPLICATION_FACTOR_MOUNTAIN;
    } else {
        declarer->penalty += declarer->tricks * MULTIPLICATION_FACTOR_MOUNTAIN;
    }

    for (auto&& player : listOfPlayers) {
        player->tricks = 0;
    }

    recalculateTotal();
}

void Game::processMisere() {
    if (!finilizedMisere) {

        if (playingIdxContract == -1) {
            playingIdxContract = (int)std::distance(listOfPlayers.begin(), std::find_if(listOfPlayers.begin(), listOfPlayers.end(), [](auto&& player) {
                return player->playerWantedRegime == GameType::Misere;
                }));
        }

        if (playingIdxContract != 0) {
            listOfPlayers[playingIdxContract]->makeMoveMiserePrepare(difficulty, talon, gen, *this);
            finilizedMisere = true;
            return;
        }

        if (!talonDistributed) {
            playerTakeTalon();
            talonDistributed = true;
        }

        checkHoveringAllPass();
        checkCardClicked();
        iterateCardsAndThrow();

        if (thrownCards == 2) {
            finilizedMisere = true;
            talon.hand.clear();
        }
        return;
    }

    if (round == MAX_HAND_SIZE) {
        round = 0;
        gameState = GameState::End;
        recalculateTotalMisere();
        dealt = false;
        return;
    }

    if (playedCards() == NUMBER_OF_PLAYERS) {
        calculateScoreAllPass();
        return;
    }

    if (currentPlayerIdx != 0) {
        Suit tmpTrump = trumpAllPass;
        listOfPlayers[currentPlayerIdx].get()->makeMoveAllPass(difficulty, discardPile, trumpAllPass, gen, gameType, *this);
        if (trumpAllPass != tmpTrump) {
            calculatedTrump = false;
        }
        passMove();
        return;
    }

    if (!calculatedTrump) {
        numberOfTrumpCardsAllPass = listOfPlayers[0].get()->getNumberOfTrumpCards(trumpAllPass);
        calculatedTrump = true;
    }

    checkHoveringAllPass();
    checkCardClicked();
    iterateCardsAndPlay();
}

MiniGameState Game::exportForMinimax() const {
    MiniGameState state;

    for (const auto& c : listOfPlayers[0]->hand) state.humanHand.emplace_back(c.getValue(), c.getSuit());
    for (const auto& c : listOfPlayers[1]->hand) state.ai1Hand.emplace_back(c.getValue(), c.getSuit());
    for (const auto& c : listOfPlayers[2]->hand) state.ai2Hand.emplace_back(c.getValue(), c.getSuit());

    state.tricksHuman = listOfPlayers[0]->tricks;
    state.tricksAI1 = listOfPlayers[1]->tricks;
    state.tricksAI2 = listOfPlayers[2]->tricks;

    state.currentPlayerIdx = currentPlayerIdx;
    state.round = round;
    state.trumpAllPass = trumpAllPass;
    state.trumpContract = trumpContract;
    state.gameType = gameType;

    for (size_t i = 0; i < discardPile.hand.size(); ++i) {
        if (discardPile.hand[i].getVisible()) {
            state.trickCards.emplace_back(discardPile.hand[i].getValue(), discardPile.hand[i].getSuit());
            state.trickOwners.push_back(i);
        }
    }

    return state;
}

void Game::resetGame() {
    tookActionBidding = false;
    gameType = GameType::None;
    hasEveryoneChoosedRegime = false;
    finilizedBidding = false;
    finilizedMisere = false;
    talonDistributed = false;
    thrownCards = 0;
    playingIdxContract = -1;
    ui.isLockedGrid = false;
    ui.loweredBid = false;

    biddingGrid.setCurrentBidCoords(Bid(Suit::Spades, 6));

    for (auto& player : listOfPlayers) {
        player->playerWantedRegime = GameType::None;
        player->bidForContract = Bid();
        player->tricks = 0;
    }
    reseted = true;
}

void Game::update() {
    switch (gameState) {
    case GameState::Menu:
        break;

    case GameState::Game:
        if (!dealt) { dealt = dealCards(); }
        if (reseted) { reseted = false; }
        if (aiClock.getElapsedTime().asSeconds() < TIME_BETWEEN_MOVE) { return; }
        switch (gameType) {
            case GameType::None:
                gameType = GameType::Bidding;
                break;
            case GameType::Bidding:
                processBidding();
                break;
            case GameType::AllPass:
                processAllPass();
                break;
            case GameType::Contract:
                processContract();
                break;
            case GameType::Misere:
                processMisere();
                break;
            default:
                break;
        }
        break;

    case GameState::End:
        if (!reseted) {
            resetGame();
        }

        break;
    }
}

void Game::render() {
    switch (gameState) {
    case GameState::Menu:
        ui.drawMenu(window, difficulty, gameState, aiClock);
        break;

    case GameState::Game:
        switch (gameType)
        {
        case GameType::None:
            ui.drawTable(window, listOfPlayers, talon, discardPile);
            break;
        case GameType::Bidding:
            if (currentPlayerIdx == 0) {
                tookActionBidding = ui.drawSelectingRegime(window, listOfPlayers, talon, discardPile, biddingGrid);
            } else {
                ui.drawTable(window, listOfPlayers, talon, discardPile);
            }
            break;
        case GameType::Contract:
            if (!finilizedBidding) {
                finilizedBidding = ui.drawSelectingRegimeBid(listOfPlayers, window, talon, discardPile, biddingGrid, thrownCards);
            } else {
                ui.drawTable(window, listOfPlayers, talon, discardPile);
            }
            break;
        case GameType::AllPass:
        case GameType::Misere:
            ui.drawTable(window, listOfPlayers, talon, discardPile);
            break;
        }
        break;

    case GameState::End:
        ui.drawFinalScore(window, listOfPlayers, gameState, gameType);
        break;
    }

    window.display();
}
