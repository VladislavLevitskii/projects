#pragma once
#include "Card.hpp"
#include "GameConfig.hpp"
#include "BiddingGrid.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <assert.h>
#include <string>
#include <iostream>

std::string gameTypeToString(const GameType gameType, const Bid bid);

class DiscardPile;
class Talon;

class CardContainer {
public:
	std::vector<Card> hand;
	virtual ~CardContainer() = default;
};

class TablePile : public CardContainer {
public:
	sf::Vector2f position;
	TablePile(sf::Vector2f pos) : position(pos) {}
	virtual ~TablePile() = default;
};

class Talon : public TablePile {
public:
	Talon(sf::Vector2f posCards)
		: TablePile(posCards) {
	}
};

class DiscardPile : public TablePile {
public:
	std::vector<sf::Vector2f> slotPositions;

	DiscardPile(sf::Vector2f posCards)
		: TablePile(posCards) {
		hand.resize(NUMBER_OF_PLAYERS);

		slotPositions.emplace_back(posCards.x + PADDING_BOTTOM, posCards.y + (PADDING_BOTTOM * 2));
		slotPositions.emplace_back(posCards.x, posCards.y);
		slotPositions.emplace_back(posCards.x + (PADDING_BOTTOM * 2), posCards.y);
	}

	void setCardsInvisible();
};

class Player : public CardContainer {
public:
	Player(const sf::Vector2f posCards, const sf::Vector2f posHand, const sf::Vector2f posScore, const sf::Vector2f posRegime)
		: positionForCards(posCards), positionForHand(posHand), positionForScore(posScore), positionForRegime(posRegime) {}

	virtual ~Player() = default;

	sf::Vector2f positionForCards;
	sf::Vector2f positionForHand;
	sf::Vector2f positionForScore;
	sf::Vector2f positionForRegime;
	sf::Text scoreDisplay;
	sf::Text regimeDisplay;
	sf::Sprite turnIndicator;
	Bid bidForContract;
	GameType playerWantedRegime = GameType::None;
	int tricks = 0;
	int whists = 0;
	int penalty = 0;
	int bullet = 0;
	int total = 0;
	bool hasMove = false;
	bool hasTrump = false;

	virtual void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType) = 0;
	virtual void makeMoveBid(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid) = 0;
	virtual void makeFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, BiddingGrid& biddingGrid) = 0;
	virtual void makeMoveContract(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, Suit& trumpAllPass, Suit& trumpContract, const GameType gameType) = 0;
	virtual void makeMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen) = 0;

	virtual std::vector<int> getLegalMovesContract(const Suit trumpAllPass, const Suit trumpContract)	;
	virtual std::vector<int> getLegalMovesAllPass(Suit trump);
	virtual void initIndicator(sf::Texture& texture);

	int getNumberOfTrumpCards(const Suit trump);
	void updateFullInfo();
	void updateScoreText();
	void initScoreDisplay(const sf::Font& font);
	void initRegimesDisplay(const sf::Font& font);
};

class AI : public Player {
public:
	AI(const sf::Vector2f posCards, const sf::Vector2f posHand, int index, const sf::Vector2f posScore, const sf::Vector2f posRegime)
		: Player(posCards, posHand, posScore, posRegime), indexAI(index) {}

	void makeMoveContract(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid, Suit& trumpAllPass, Suit& trumpContract, const GameType gameType) override;
	void makeMoveAllPass(GameDifficulty difficulty, DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType) override;
	void makeMoveBid(GameDifficulty difficulty, DiscardPile& discardPile, std::mt19937& gen, BiddingGrid& biddingGrid) override;
	void makeMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen) override;
	void makeFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen, BiddingGrid& biddingGrid) override;

	void easyMoveFinalBid(GameDifficulty difficulty, Talon& talon, std::mt19937& gen);
	void easyMoveAllPass(DiscardPile& discardPile, Suit& trump, std::mt19937& gen, GameType gameType);
	void easyMoveBid(BiddingGrid& biddingGrid, std::mt19937& gen);
	void easyMoveContract(DiscardPile& discardPile, const Suit& trump, Suit& trumpAllPass, std::mt19937& gen, const GameType gameType);
	void easyMoveMiserePrepare(GameDifficulty difficulty, Talon& talon, std::mt19937& gen);
	void playCardAI(int rndIdx, DiscardPile& discardPile);
	int getRandomCardIdx(const Suit trumpAllPass, std::mt19937& gen, const GameType gameType, const Suit trumpContract = Suit::None);

private:
	int indexAI = 0;
	std::random_device rd;
};

class HumanPlayer : public Player {
public:
	using Player::Player;

	void makeMoveContract(GameDifficulty, DiscardPile&, std::mt19937&, BiddingGrid&, Suit&, Suit&, const GameType) override {}
	void makeMoveAllPass(GameDifficulty, DiscardPile&, Suit&, std::mt19937&, GameType) override {}
	void makeMoveBid(GameDifficulty, DiscardPile&, std::mt19937&, BiddingGrid&) override {}
	void makeMoveMiserePrepare(GameDifficulty, Talon&, std::mt19937&) override {}
	void makeFinalBid(GameDifficulty, Talon&, std::mt19937&, BiddingGrid&) override {}
};