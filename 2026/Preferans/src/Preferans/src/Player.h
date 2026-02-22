#pragma once
#include "Card.h"
#include "GameConfig.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <assert.h>

class DiscardPile;

class Player {
public:

	Player(const sf::Vector2f& posCards, const sf::Vector2f& posHand, const sf::Vector2f& posScore)
		: positionForCards(posCards), positionForHand(posHand), positionForScore(posScore) {}

	virtual ~Player() = default;

	std::vector<Card> hand;
	sf::Vector2f positionForCards;
	sf::Vector2f positionForHand;
	sf::Vector2f positionForScore;
	sf::Text scoreDisplay;
	sf::Sprite turnIndicator;
	int tricks = 0;
	bool hasMove = false;
	bool hasTrump = false;

	void updateScoreText();
	void initScoreDisplay(sf::Font& font);
	void initIndicator(sf::Texture& texture);
	int getNumberOfTrumpCards(const Suit trump);
	std::vector<int> getLegalMoves(const std::vector<Card>& hand, Suit trump);
	virtual void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump) {};
};

class AI : public Player {
public:
	AI(const sf::Vector2f& posCards, const sf::Vector2f& posHand, int index, const sf::Vector2f& posScore)
		: Player(posCards, posHand, posScore), indexAI(index) {
		std::random_device rd;
		gen.seed(rd());
	}

	void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump);
	void easyMoveAllRandom(DiscardPile& discardPile, Suit& trump);
	int getRandomIdx(Suit& trump);

private:
	std::mt19937 gen;
	int indexAI = 0;
};

class Talon : public Player {
public:
	Talon(sf::Vector2f posCards)
		: Player(posCards, sf::Vector2f(0.f, 0.f), sf::Vector2f(0.f, 0.f)) {
	}
};

class DiscardPile : public Talon {
public:
	std::vector<sf::Vector2f> slotPositions;

	DiscardPile(sf::Vector2f posCards)
		: Talon(posCards) {
		hand.resize(NUMBER_OF_PLAYERS);

		slotPositions.push_back({ posCards.x + PADDING_BOTTOM, posCards.y + (PADDING_BOTTOM * 2) });
		slotPositions.push_back({ posCards.x, posCards.y });
		slotPositions.push_back({ posCards.x + (PADDING_BOTTOM * 2), posCards.y });
	}

	void setCardsInvisible();
};