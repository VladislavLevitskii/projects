#pragma once

#include <array>
#include <cassert>
#include "GameConfig.hpp"

/**
 * @class Bid
 * @brief Represents a single bid in the game.
 * Encapsulates the core data of a bid during the auction phase, consisting 
 * of a specific suit and a numerical value (level).
 */
class Bid {
public:

	Bid() = default;

	/**
     * Constructs a Bid with a specific suit and value.
     * @param suit_ The suit of the bid.
     * @param value_ The value (level) of the bid.
     */
	Bid(Suit suit_, int value_) : suit(suit_), value(value_) {}

	/**
     * Sets the suit and value of the bid.
     * @param suit_ The new suit to set.
     * @param value_ The new value to set.
     */
	void setBid(Suit suit_, int value_) {
		suit = suit_;
		value = value_;
	}

	/**
     * Retrieves the suit of the bid.
     * @return The current suit of the bid.
     */
	Suit getSuit() const {
		return suit;
	}

	/**
     * Retrieves the value of the bid.
     * @return The current value of the bid.
     */
	int getValue() const {
		return value;
	}

	/**
     * Compares two bids for equality.
     * @param other The other bid to compare with.
     * @return True if both suit and value are equal, false otherwise.
     */
	bool operator==(const Bid other) const {
		return other.getSuit() == this->getSuit() && other.getValue() == this->getValue();
	}

private:
	Suit suit = Suit::Clubs;
	int value = 0;
};

/**
 * @class BiddingGrid
 * @brief Manages the grid of all possible bids.
 * Handles the logic for traversing available bids in the correct order 
 * (by level and suit hierarchy) and stores the current state of the auction.
 */
class BiddingGrid {
public:
	BiddingGrid();

	/** * Increments the current bid to the next possible value.
     * Advances the suit in the standard order (Spades < Clubs < Diamonds < Hearts).
     * If the current suit is Spades, increments the bid value and resets suit to Clubs.
     * @note Aborts via assert if the bid exceeds the maximum allowed value.
     */
	void increaseBid();

	/**
     * Decreases the current bid to the previous possible value.
     * Works exactly like increaseBid but in reverse.
     */
	void decreaseBid();

	/**
     * Retrieves the current bid from the bidding grid.
     * @return The currently selected Bid object.
     */
	Bid getCurrentBid() { 
		return grid[currentBidCoordinates.first][currentBidCoordinates.second];
	}

	/**
     * Retrieves the current coordinates of the bid within the grid.
     * @return A std::pair where the first element is the level and the second is the suit index.
     */
	auto getCurrentCoords() const { return currentBidCoordinates; }

	/**
     * Gets the corresponding bid in the bidding grid at specified coordinates.
     * @param level The row index representing the bid value.
     * @param suit The column index representing the suit.
     * @return The Bid object located at the specified level and suit.
     */
	Bid getBidAt(int level, int suit) const { return grid[level][suit]; }

	/**
     * Sets the current bid coordinates based on the provided bid.
     * @param bid The target Bid used to update the current coordinates.
     */
	void setCurrentBidCoords(Bid bid);

private:
	std::array<std::array<Bid, NUMBER_OF_SUITS>, NUMBER_OF_LEVEL_BID> grid;
	std::pair<int, int> currentBidCoordinates = {0, 0};
};

const Bid FINAL_BID = Bid(Suit::Hearts, 10);
