#pragma once

#include <array>
#include <cassert>
#include "GameConfig.hpp"

class Bid {
public:

	Bid() = default;
	Bid(Suit suit_, int value_) : suit(suit_), value(value_) {}

	void setBid(Suit suit_, int value_) {
		suit = suit_;
		value = value_;
	}

	Suit getSuit() const {
		return suit;
	}

	int getValue() const {
		return value;
	}

	bool operator==(const Bid other) const {
		return other.getSuit() == this->getSuit() && other.getValue() == this->getValue();
	}

private:
	Suit suit = Suit::Clubs;
	int value = 0;
};

class BiddingGrid {
public:
	BiddingGrid();
	void increaseBid();
	void decreaseBid();

	Bid getCurrentBid() { 
		return grid[currentBidCoordinates.first][currentBidCoordinates.second];
	}

private:
	std::array<std::array<Bid, NUMBER_OF_SUITS>, NUMBER_OF_LEVEL_BID> grid;
	std::pair<int, int> currentBidCoordinates = {0, 0};
};

const Bid FINAL_BID = Bid(Suit::Hearts, 10);