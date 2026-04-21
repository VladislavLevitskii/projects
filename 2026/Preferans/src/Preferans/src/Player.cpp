#include "../include/Player.hpp"
#include "../include/Game.hpp"

std::string gameTypeToString(const GameType gameType, const Bid bid) {
	std::string result;
	switch (gameType)
	{
	case GameType::None:
		result = "None";
		break;
	case GameType::Bidding:
		result = "Bidding";
		break;
	case GameType::Contract:
		result = std::to_string(bid.getValue()) 
			+ " " + suitToString((bid.getSuit()));
		break;
	case GameType::AllPass:
		result = "AllPass";
		break;
	case GameType::Misere:
		result = "Misere";
		break;
	}
	return result;
}

int Player::getNumberOfTrumpCards(const Suit trump) {
	return (int)std::count_if(hand.begin(), hand.end(), [&](const Card& card) {
		return card.getSuit() == trump;
		});
}

std::vector<int> Player::getLegalMovesAllPass(const Suit trump) {
	std::vector<int> result;
	int numberOfTrumpCards = getNumberOfTrumpCards(trump);
	bool noTrump = false;
	if (numberOfTrumpCards == 0) {
		noTrump = true;
	}

	for (auto it = hand.begin(); it != hand.end(); ++it) {
		if ((*it).getSuit() == trump || trump == Suit::None || noTrump) {
			result.push_back((int)std::distance(hand.begin(), it));
		}
	}

	assert(result.size() != 0);
	return result;
}

std::vector<int> Player::getLegalMovesContract(const Suit trumpAllPass, const Suit trumpContract) {
	std::vector<int> result;
	int numberOfTrumpCardsAllPass = getNumberOfTrumpCards(trumpAllPass);
	int numberOfTrumpCardsContract = getNumberOfTrumpCards(trumpContract);

	for (int i = 0; i < hand.size(); ++i) {
		auto& card = hand[i];

		bool isSuitPlayable = (trumpAllPass == card.getSuit() || trumpAllPass == Suit::None) ||
			(numberOfTrumpCardsAllPass == 0 && numberOfTrumpCardsContract == 0) ||
			(trumpContract == card.getSuit() && numberOfTrumpCardsAllPass == 0);

		if (isSuitPlayable) {
			result.push_back(i);
		}
	}
	return result;
}

int AI::getRandomCardIdx(const Suit trumpAllPass, std::mt19937& gen, const GameType gameType, const Suit trumpContract) {
	int result = -1;
	if (trumpAllPass == Suit::None) {
		std::uniform_int_distribution<int> dis(0, static_cast<int>(hand.size()) - 1);
		result = dis(gen);
	} else {
		std::vector<int> legalCardIdx;
		switch (gameType) {
			case GameType::Bidding:
				break;
			case GameType::Contract:
				legalCardIdx = getLegalMovesContract(trumpAllPass, trumpContract);
				break;
			case GameType::AllPass:
			case GameType::Misere:
				legalCardIdx = getLegalMovesAllPass(trumpAllPass);
				break;
			default:
				break;
		}
		assert(legalCardIdx.size() != 0);
		std::uniform_int_distribution<int> dis(0, static_cast<int>(legalCardIdx.size()) - 1);
		result = legalCardIdx[dis(gen)];
	}
	return result; // never returns -1
}

void AI::playCardAI(int rndIdx, DiscardPile& discardPile) {
	discardPile.hand[indexAI] = std::move(hand[rndIdx]);
	discardPile.hand[indexAI].setVisible(true);
	hand.erase(hand.begin() + rndIdx);
}

void AI::easyMoveAllPass(DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType) {
	int rndIdx = getRandomCardIdx(trump, gen, gameType);
	assert(rndIdx != -1);
	if (trump == Suit::None) {
		trump = hand[rndIdx].getSuit();
	}
	playCardAI(rndIdx, discardPile);
}

void AI::easyMoveBid(BiddingGrid& biddingGrid, std::mt19937& gen) {
	if (playerWantedRegime == GameType::None) {
		std::uniform_int_distribution<int> dis(0, 99);
		int result = dis(gen);
		if (result < AI_CHANCE_TO_MISERE_EASY) {
			playerWantedRegime = GameType::Misere;
		} else if (result < AI_CHANCE_TO_MISERE_EASY + AI_CHANCE_TO_CONTRACT_EASY) {
			playerWantedRegime = GameType::Contract;
			bidForContract = biddingGrid.getCurrentBid();
			biddingGrid.increaseBid();
		} else {
			playerWantedRegime = GameType::AllPass;
		}
	} else if (playerWantedRegime == GameType::Contract) { // case when AI needs to increase bid or pass 
		std::uniform_int_distribution<int> dis(0, 99);
		int result = dis(gen);
		if (result < AI_CHANCE_TO_STAY_BID_EASY || biddingGrid.getCurrentBid() == FINAL_BID) {
			playerWantedRegime = GameType::AllPass;
			return;
		}

		bidForContract = biddingGrid.getCurrentBid();
		biddingGrid.increaseBid();
	}
}

void AI::makeMoveContract(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, Suit& trumpAllPass, Suit& trumpContract, const GameType gameType, const Game& game) {
	switch (difficulty)
	{
	case GameDifficulty::Easy:
		easyMoveContract(discardPile, trumpContract, trumpAllPass, gen, gameType);
		break;
	case GameDifficulty::Medium: {
		MiniGameState state = game.exportForMinimax();
		int bestMove = Minimax::getBestMove(state, indexAI, MEDIUM_DEPTH_MINIMAX);

		if (trumpAllPass == Suit::None) {
			trumpAllPass = hand[bestMove].getSuit();
		}

		playCardAI(bestMove, discardPile);
		break;
	}
	case GameDifficulty::Hard: {
		MiniGameState state = game.exportForMinimax();
		int bestMove = Minimax::getBestMove(state, indexAI, HARD_DEPTH_MINIMAX);

		if (trumpAllPass == Suit::None) {
			trumpAllPass = hand[bestMove].getSuit();
		}

		playCardAI(bestMove, discardPile);
		break;
	}
	default:
		break;
	}
}

void AI::makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType, const Game& game) {
	switch (difficulty) {
		case GameDifficulty::Easy:
			easyMoveAllPass(discardPile, trump, gen, gameType);
			break;
		case GameDifficulty::Medium: {
			MiniGameState state = game.exportForMinimax();
			int bestMove = Minimax::getBestMove(state, indexAI, MEDIUM_DEPTH_MINIMAX);

			if (trump == Suit::None) 
				trump = hand[bestMove].getSuit();

			playCardAI(bestMove, discardPile);
			break;
		}
		case GameDifficulty::Hard: {
			MiniGameState state = game.exportForMinimax();
			int bestMove = Minimax::getBestMove(state, indexAI, HARD_DEPTH_MINIMAX);

			if (trump == Suit::None)
				trump = hand[bestMove].getSuit();

			playCardAI(bestMove, discardPile);
			break;
		}
		default:
			break;
	}
}

void AI::moveBidHeuristic(BiddingGrid& biddingGrid) {
	if (playerWantedRegime == GameType::AllPass || playerWantedRegime == GameType::Misere) {
		return;
	}

	Bid currentBid = biddingGrid.getCurrentBid();

	int misereDanger = 0;
	for (const auto& c : hand) {
		if (c.getValue() >= CARDS_ONE_SUIT - 1) misereDanger++;
	}

	if (playerWantedRegime == GameType::None && misereDanger <= 1) {
		playerWantedRegime = GameType::Misere;
		return;
	}

	int bestEstimatedTricks = 0;
	Suit bestSuit = Suit::None;

	std::vector<Suit> options = { Suit::Spades, Suit::Diamonds, Suit::Clubs, Suit::Hearts, Suit::None };
	for (Suit s : options) {
		int e_tricks = estimateTricks(hand, s);
		if (e_tricks > bestEstimatedTricks) {
			bestEstimatedTricks = e_tricks;
			bestSuit = s;
		}
	}

	int requiredTricksToOverbid = currentBid.getValue();

	if (playerWantedRegime == GameType::None) {
		if (bestEstimatedTricks >= requiredTricksToOverbid - 1) {
			playerWantedRegime = GameType::Contract;
			bidForContract = currentBid;
			biddingGrid.increaseBid();
		} else {
			playerWantedRegime = GameType::AllPass;
		}
	} else if (playerWantedRegime == GameType::Contract) {
		if (currentBid == FINAL_BID) {
			playerWantedRegime = GameType::AllPass;
			return;
		}

		if (bestEstimatedTricks >= requiredTricksToOverbid) {
			bidForContract = currentBid;
			biddingGrid.increaseBid();
		} else {
			playerWantedRegime = GameType::AllPass;
		}
	}
}

void AI::makeMoveBid(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) {
	switch (difficulty) {
	case GameDifficulty::Easy:
		easyMoveBid(biddingGrid, gen);
		break;
	case GameDifficulty::Medium:
	case GameDifficulty::Hard: {
		moveBidHeuristic(biddingGrid);
		break;
	}
	default:
		break;
	}
}

void AI::easyMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen) {
	easyMoveFinalBid(difficulty, talon, gen);
}

void AI::makeMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, const Game& game) {
	switch (difficulty)
	{
	case GameDifficulty::Easy:
		easyMoveMiserePrepare(difficulty, talon, gen);
		break;

	case GameDifficulty::Medium:
	case GameDifficulty::Hard: {
		for (auto&& card : talon.hand) {
			card.setFront();
			hand.push_back(std::move(card));
		}
		talon.hand.clear();

		for (int i = 0; i < 2; ++i) {
			auto it = std::max_element(hand.begin(), hand.end(), [](const Card& a, const Card& b) {
				return a.getValue() < b.getValue();
				});

			if (it != hand.end()) {
				hand.erase(it);
			}
		}
		break;
	}
	default:
		break;
	}

	std::sort(hand.begin(), hand.end());
}

void AI::easyMoveFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen) {
	for (auto&& card : talon.hand) {
		hand.push_back(std::move(card));
	}
	talon.hand.clear();
	std::uniform_int_distribution<int> dis(0, 11);
	int firstCardIdx = -1;
	int secondCardIdx = -1;
	do {
		firstCardIdx = dis(gen);
		secondCardIdx = dis(gen);
	} while (firstCardIdx == secondCardIdx);
	
	hand.erase(hand.begin() + std::max(firstCardIdx, secondCardIdx));
	hand.erase(hand.begin() + std::min(firstCardIdx, secondCardIdx));
}

void AI::easyMoveContract(DiscardPile& discardPile, const Suit& trumpContract, Suit& trumpAllPass, std::mt19937& gen, const GameType gameType) {
	int rndIdx = getRandomCardIdx(trumpAllPass, gen, gameType, trumpContract);
	assert(rndIdx != -1);
	if (trumpAllPass == Suit::None) {
		trumpAllPass = hand[rndIdx].getSuit();
	}
	playCardAI(rndIdx, discardPile);
}

void AI::performSmartDiscard(Suit trump) {
	for (int i = 0; i < 2; ++i) {
		auto it = std::min_element(hand.begin(), hand.end(), [&](const Card& a, const Card& b) {
			if (a.getSuit() == trump && b.getSuit() != trump) return false;
			if (a.getSuit() != trump && b.getSuit() == trump) return true;

			bool aIsHigh = (a.getValue() >= 13);
			bool bIsHigh = (b.getValue() >= 13);
			if (aIsHigh && !bIsHigh) return false;
			if (!aIsHigh && bIsHigh) return true;

			return a.getValue() < b.getValue();
			});

		if (it != hand.end()) {
			hand.erase(it);
		}
	}
}

int AI::estimateTricks(const std::vector<Card>& currentHand, Suit trump) {
	int e_tricks = 0;
	std::map<Suit, int> counts;
	std::map<Suit, bool> hasAce;
	std::map<Suit, bool> hasKing;

	for (const auto& c : currentHand) {
		counts[c.getSuit()]++;
		if (c.getValue() == CARDS_ONE_SUIT + 1) hasAce[c.getSuit()] = true;
		if (c.getValue() == CARDS_ONE_SUIT) hasKing[c.getSuit()] = true;
	}

	e_tricks += std::max(0, counts[trump] - 2);
	if (hasAce[trump]) e_tricks++;

	for (int s = 0; s < NUMBER_OF_SUITS; ++s) {
		Suit st = static_cast<Suit>(s);
		if (st == trump) continue;

		if (hasAce[st]) e_tricks++;
		else if (hasKing[st] && counts[st] >= 2) e_tricks++;
	}

	return e_tricks;
}

void AI::makeFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, BiddingGrid& biddingGrid, const Game& game) {
	for (auto&& card : talon.hand) {
		card.setFront();
		hand.push_back(std::move(card));
	}
	talon.hand.clear();

	switch (difficulty) {
	case GameDifficulty::Easy: {
		easyMoveFinalBid(difficulty, talon, gen);
		break;
	}

	case GameDifficulty::Medium:
	case GameDifficulty::Hard: {
		auto coords = biddingGrid.getCurrentCoords();
		int startLevel = coords.first;
		int startSuit = coords.second;

		Bid bestBid = biddingGrid.getCurrentBid();
		int maxSurplus = -100;

		for (int l = startLevel; l < NUMBER_OF_LEVEL_BID; ++l) {
			int sStart = (l == startLevel) ? startSuit : 0;
			for (int s = sStart; s < NUMBER_OF_SUITS; ++s) {
				Bid potentialBid = biddingGrid.getBidAt(l, s);
				Suit potentialTrump = potentialBid.getSuit();

				int estimated = estimateTricks(hand, potentialTrump);
				int surplus = estimated - potentialBid.getValue();

				if (surplus > maxSurplus) {
					maxSurplus = surplus;
					bestBid = potentialBid;
				}
			}
		}

		biddingGrid.setCurrentBidCoords(bestBid);
		performSmartDiscard(bestBid.getSuit());
		break;
	}
	}

	std::sort(hand.begin(), hand.end());
}

void Player::initScoreDisplay(const sf::Font& font) {
	scoreDisplay.setFont(font);
	scoreDisplay.setCharacterSize(CHARACTER_SIZE_BIG);
	scoreDisplay.setFillColor(sf::Color::Black);
	scoreDisplay.setPosition(positionForScore);
	updateScoreText();
}

void Player::initGameModeDisplay(const sf::Font& font) {
	gameModeDisplay.setFont(font);
	gameModeDisplay.setCharacterSize(CHARACTER_SIZE_BIG);
	gameModeDisplay.setFillColor(sf::Color::Black);
	gameModeDisplay.setPosition(positionForRegime);
	gameModeDisplay.setString(gameTypeToString(playerWantedRegime, Bid()));
}

void Player::updateScoreText() {
	scoreDisplay.setString("Tricks: " + std::to_string(tricks));
}

void Player::updateFullInfo() {
	std::string fullScore =
		"Bullet:  " + std::to_string(bullet) + "\n" +
		"Penalty: " + std::to_string(penalty) + "\n" +
		"Whists:  " + std::to_string(whists) + "\n\n" +
		"TOTAL:   " + std::to_string(total);
	scoreDisplay.setString(fullScore);
}

void Player::initIndicator(sf::Texture& texture) {
	turnIndicator.setTexture(texture);
	turnIndicator.setScale(0.05f, 0.05f);
	turnIndicator.setPosition(positionForHand);
}

// this function is used to "remove" cards of DiscardPile
void DiscardPile::setCardsInvisible() {
	for (auto&& card : hand) {
		card.setVisible(false);
	}
}