#include "Player.h"

int Player::getNumberOfTrumpCards(const Suit trump) {
	int result = 0;
	for (auto&& card : hand) {
		if (card.getSuit() == trump) {
			result++;
		}
	}
	return result;
}

// never returns empty vector
std::vector<int> Player::getLegalMoves(const std::vector<Card>& hand, Suit trump) {
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
	return result;
}

int AI::getRandomIdx(Suit& trump) {
	int result = -1; // never returns -1
	if (trump == Suit::None) {
		std::uniform_int_distribution<int> dis(0, static_cast<int>(hand.size()) - 1);
		result = dis(gen);
	} else {
		std::vector<int> legalCardIdx = getLegalMoves(hand, trump);
		assert(legalCardIdx.size() != 0);
		std::uniform_int_distribution<int> dis(0, static_cast<int>(legalCardIdx.size()) - 1);
		result = legalCardIdx[dis(gen)];
	}
	return result;
}

void AI::easyMoveAllRandom(DiscardPile& discardPile, Suit& trump) {
	int rndIdx = getRandomIdx(trump);
	assert(rndIdx != -1);
	if (trump == Suit::None) {
		trump = hand[rndIdx].getSuit();
	}
	discardPile.hand[indexAI] = std::move(hand[rndIdx]);
	discardPile.hand[indexAI].setVisible(true);
	hand.erase(hand.begin() + rndIdx);
}

void AI::makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump) {
	switch (difficulty) {
		case GameDifficulty::Easy:
			easyMoveAllRandom(discardPile, trump);
			break;
		case GameDifficulty::Medium:
			break;
		case GameDifficulty::Hard:
			break;
		default:
			break;
	}
}

void Player::initScoreDisplay(sf::Font& font) {
	scoreDisplay.setFont(font);
	scoreDisplay.setCharacterSize(30);
	scoreDisplay.setFillColor(sf::Color::Black);
	scoreDisplay.setPosition(positionForScore);
	updateScoreText();
}

void Player::updateScoreText() {
	scoreDisplay.setString("Score: " + std::to_string(tricks));
}

void Player::initIndicator(sf::Texture& texture) {
	turnIndicator.setTexture(texture);
	turnIndicator.setScale(0.05f, 0.05f);
	turnIndicator.setPosition(positionForHand);
}

// this function is used to "remove" cards, see description DiscardPile
void DiscardPile::setCardsInvisible() {
	for (auto&& card : hand) {
		card.setVisible(false);
	}
}