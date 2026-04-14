#include "../include/BiddingGrid.hpp"

void BiddingGrid::decreaseBid() {
	if (currentBidCoordinates.first == 0 && currentBidCoordinates.second == 0) {
		return;
	}

	if (currentBidCoordinates.second == 0) {
		currentBidCoordinates.second = NUMBER_OF_SUITS - 1;
		--currentBidCoordinates.first;
	}
	else {
		--currentBidCoordinates.second;
	}
}

void BiddingGrid::increaseBid() {
	if (currentBidCoordinates.first == NUMBER_OF_LEVEL_BID - 1 &&
		currentBidCoordinates.second == NUMBER_OF_SUITS - 1) {
		return;
	}

	if (++currentBidCoordinates.second == NUMBER_OF_SUITS) {
		currentBidCoordinates.second = 0;
		++currentBidCoordinates.first;
	}
}

BiddingGrid::BiddingGrid() {
	for (int i = 0; i < NUMBER_OF_LEVEL_BID; ++i) {
		for (int j = 0; j < NUMBER_OF_SUITS; ++j) {
			grid[i][j].setBid(static_cast<Suit>(j + 1), i + START_BID);
		}
	}
}