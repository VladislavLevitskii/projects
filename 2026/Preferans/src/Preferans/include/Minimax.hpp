#pragma once
#include <vector>
#include <algorithm>
#include <limits>
#include <cassert>
#include <bit>
#include "Card.hpp"
#include "GameConfig.hpp"

/**
 * @brief Structure used to record the state of the game before a move is made.
 * It allows the Minimax algorithm to cleanly revert moves during its search.
 */
struct UndoRecord {
    CardBase playedCard;
    size_t previousPlayerIdx;

    size_t trickWinnerIdx                   = 0;
    std::vector<CardBase> oldTrickCards;
    std::vector<size_t> oldTrickOwners;
    Suit previousTrumpAllPass;
    int cardIdxInHand;
    bool trickResolved                      = false;
    bool wasLastCard                        = false;

    /**
     * Constructs an UndoRecord to save the state before a move.
     * @param idx The index of the card in the player's hand.
     * @param card The card being played.
     * @param prevPlayer The index of the player making the move.
     * @param prevTrump The current All Pass trump suit before the move.
     */
    UndoRecord(int idx, CardBase card, size_t prevPlayer, Suit prevTrump)
        : cardIdxInHand(idx), playedCard(card), previousPlayerIdx(prevPlayer),
        previousTrumpAllPass(prevTrump) {
    }
};

/**
 * @brief A lightweight representation of the game state used exclusively by the Minimax algorithm.
 * Contains only the necessary data to simulate turns recursively.
 */
struct MiniGameState {
    std::vector<CardBase> humanHand;
    std::vector<CardBase> ai1Hand;
    std::vector<CardBase> ai2Hand;

    std::vector<size_t> trickOwners;
    std::vector<CardBase> trickCards;

    size_t round = 0;
    size_t currentPlayerIdx = 0;

    int tricksHuman = 0;
    int tricksAI1 = 0;
    int tricksAI2 = 0;

    Suit trumpAllPass = Suit::None;
    Suit trumpContract = Suit::None;
    GameType gameType = GameType::None;
};

/**
 * @class Minimax
 * @brief Mathematical engine for AI decision-making.
 * Provides static methods to compute the most optimal card play using the 
 * minimax algorithm with alpha-beta pruning, based on a simplified snapshot 
 * of the current game state.
 */
class Minimax {
public:

    /**
     * Calculates the optimal move using the minimax algorithm.
     * @param state The current simplified game state.
     * @param aiIndex The index of the AI player making the move.
     * @param max_depth The maximum depth for the search tree.
     * @return The index of the best card to play from the current hand.
     */
    static int getBestMove(const MiniGameState& state, size_t aiIndex, int max_depth);

private:

    /**
     * Evaluates the game state recursively with alpha-beta pruning.
     * @param state The current simplified game state.
     * @param maximizingPlayerIdx The index of the player trying to maximize the score.
     * @param alpha The alpha value for pruning.
     * @param beta The beta value for pruning.
     * @param depth The current depth in the search tree.
     * @param max_depth The maximum allowed depth.
     * @return The heuristic score of the evaluated state.
     */
    static int evaluateState(MiniGameState& state, size_t maximizingPlayerIdx, int alpha, int beta, int depth, int max_depth);

    /**
     * Checks if the current game state represents a finished game.
     * @param state The current simplified game state.
     * @return True if all players' hands are empty, false otherwise.
     */
    static bool isGameOver(const MiniGameState& state);

    /**
     * Retrieves all valid moves for the current player as a bitmask.
     * @param state The current simplified game state.
     * @return A bitmask representing the legal card indices.
     */
    static uint16_t getLegalMoves(const MiniGameState& state);

    /**
     * Executes a move and returns the information needed to undo it.
     * @param state The current simplified game state.
     * @param cardIdxInHand The index of the card to play.
     * @return An UndoRecord containing the state before the move.
     */
    static UndoRecord applyMove(MiniGameState& state, int cardIdxInHand);

    /**
     * Reverts a previously applied move.
     * @param state The current simplified game state.
     * @param record The UndoRecord containing the information needed to restore the state.
     */
    static void undoMove(MiniGameState& state, const UndoRecord& record);

    /**
     * Calculates legal moves specifically for the All Pass game mode.
     * @param hand The vector of cards in the current player's hand.
     * @param trumpAllPass The current trump suit for the trick.
     * @return A bitmask representing the legal card indices.
     */
    static uint16_t getLegalMovesAllPass(const std::vector<CardBase>& hand, Suit trumpAllPass);

    /**
     * Calculates legal moves specifically for the Contract game mode.
     * @param hand The vector of cards in the current player's hand.
     * @param trumpAllPass The current trump suit for the trick.
     * @param trumpContract The trump suit of the active contract.
     * @return A bitmask representing the legal card indices.
     */
    static uint16_t getLegalMovesContract(const std::vector<CardBase>& hand, Suit trumpAllPass, Suit trumpContract);
};
