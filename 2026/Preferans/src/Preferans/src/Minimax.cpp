#include "../include/Minimax.hpp"

std::string cardToText(const CardBase& c) {
    std::string s;
    switch (c.getSuit()) {
    case Suit::Spades: s = "S"; break;
    case Suit::Diamonds: s = "D"; break;
    case Suit::Clubs: s = "C"; break;
    case Suit::Hearts: s = "H"; break;
    default: s = "?";
    }
    return s + "-" + std::to_string(c.getValue());
}

class BaseCardComparator {
    Suit trump;
public:
    BaseCardComparator(Suit t) : trump(t) {}
    bool operator()(const CardBase& a, const CardBase& b) const {
        if (a.getSuit() != trump && b.getSuit() == trump) return true;
        if (a.getSuit() == trump && b.getSuit() != trump) return false;
        return a.getValue() < b.getValue(); 
    }
};

std::vector<CardBase>& getCurrentHand(MiniGameState& state) {
    if (state.currentPlayerIdx == 0) return state.humanHand;
    if (state.currentPlayerIdx == 1) return state.ai1Hand;
    return state.ai2Hand;
}

const std::vector<CardBase>& getCurrentHandConst(const MiniGameState& state) {
    if (state.currentPlayerIdx == 0) return state.humanHand;
    if (state.currentPlayerIdx == 1) return state.ai1Hand;
    return state.ai2Hand;
}

int Minimax::getBestMove(const MiniGameState& state, size_t aiIndex, int max_depth) {
    int bestScore = std::numeric_limits<int>::min();
    int bestMoveIdx = -1;

    uint16_t legalMoves = getLegalMoves(state);
    MiniGameState searchState = state;

    while (legalMoves > 0) {
        int moveIdx = std::countr_zero(legalMoves);
        legalMoves &= (legalMoves - 1);
        CardBase card = getCurrentHandConst(searchState)[moveIdx];

        UndoRecord record = applyMove(searchState, moveIdx);
        int score = evaluateState(searchState, aiIndex, -100, 100, 1, max_depth);
        undoMove(searchState, record);

        if (score > bestScore) {
            bestScore = score;
            bestMoveIdx = moveIdx;
        }
    }
    return bestMoveIdx;
}

int Minimax::evaluateState(MiniGameState& state, size_t maxIdx, int alpha, int beta, int depth, int max_depth) {
    if (isGameOver(state) || depth >= max_depth) {
        return (state.gameType == GameType::AllPass || state.gameType == GameType::Misere)
            ? state.tricksHuman : -state.tricksHuman;
    }

    uint16_t legalMoves = getLegalMoves(state);

    if (legalMoves == 0) {
        return (state.gameType == GameType::AllPass || state.gameType == GameType::Misere)
            ? state.tricksHuman : -state.tricksHuman;
    }

    bool isMaximizing = (state.currentPlayerIdx != 0);    

    if (isMaximizing) {
        int maxEval = std::numeric_limits<int>::min();
        while (legalMoves > 0) {
            int moveIdx = std::countr_zero(legalMoves);
            legalMoves &= (legalMoves - 1);

            UndoRecord record = applyMove(state, moveIdx);
            int eval = evaluateState(state, maxIdx, alpha, beta, depth + 1, max_depth);
            undoMove(state, record);

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        while (legalMoves > 0) {
            int moveIdx = std::countr_zero(legalMoves);
            legalMoves &= (legalMoves - 1);

            UndoRecord record = applyMove(state, moveIdx);
            int eval = evaluateState(state, maxIdx, alpha, beta, depth + 1, max_depth);
            undoMove(state, record);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

bool Minimax::isGameOver(const MiniGameState& state) {
    return state.humanHand.empty() && state.ai1Hand.empty() && state.ai2Hand.empty();
}

uint16_t Minimax::getLegalMovesAllPass(const std::vector<CardBase>& hand, Suit trumpAllPass) {
    uint16_t result = 0;

    if (trumpAllPass == Suit::None) {
        for (size_t i = 0; i < hand.size(); ++i) result |= (1 << i);
        return result;
    }

    int numberOfTrumpCards = (int)std::count_if(hand.begin(), hand.end(),
        [&](const CardBase& c) { return c.getSuit() == trumpAllPass; });

    for (size_t i = 0; i < hand.size(); ++i) {
        if (hand[i].getSuit() == trumpAllPass || numberOfTrumpCards == 0) {
            result |= (1 << i);
        }
    }

    assert(result != 0 && "Legal moves for AllPass cannot be empty");
    return result;
}

uint16_t Minimax::getLegalMovesContract(const std::vector<CardBase>& hand, Suit trumpAllPass, Suit trumpContract) {
    uint16_t result = 0;

    if (trumpAllPass == Suit::None) {
        for (size_t i = 0; i < hand.size(); ++i) result |= (1 << i);
        return result;
    }

    int numberOfTrumpCardsAllPass = (int)std::count_if(hand.begin(), hand.end(),
        [&](const CardBase& c) { return c.getSuit() == trumpAllPass; });
    int numberOfTrumpCardsContract = (int)std::count_if(hand.begin(), hand.end(),
        [&](const CardBase& c) { return c.getSuit() == trumpContract; });

    for (size_t i = 0; i < hand.size(); ++i) {
        const auto& card = hand[i];
        bool isSuitPlayable = (trumpAllPass == card.getSuit()) ||
            (numberOfTrumpCardsAllPass == 0 && numberOfTrumpCardsContract == 0) ||
            (trumpContract == card.getSuit() && numberOfTrumpCardsAllPass == 0);

        if (isSuitPlayable) {
            result |= (1 << i);
        }
    }

    assert(result != 0 && "Legal moves for Contract cannot be empty!");
    return result;
}

uint16_t Minimax::getLegalMoves(const MiniGameState& state) {
    const auto& hand = getCurrentHandConst(state);

    if (hand.empty()) return {};

    if (state.gameType == GameType::Contract) {
        return getLegalMovesContract(hand, state.trumpAllPass, state.trumpContract);
    } else if (state.gameType == GameType::AllPass || state.gameType == GameType::Misere) {
        return getLegalMovesAllPass(hand, state.trumpAllPass);
    }

    uint16_t result = 0;
    for (size_t i = 0; i < hand.size(); ++i) result |= (1 << i);
    return result;
}

UndoRecord Minimax::applyMove(MiniGameState& state, int cardIdxInHand) {
    auto& hand = getCurrentHand(state);
    CardBase playedCard = hand[cardIdxInHand];

    UndoRecord record(cardIdxInHand, playedCard, state.currentPlayerIdx, state.trumpAllPass);

    record.wasLastCard = (cardIdxInHand == hand.size() - 1);

    if (!record.wasLastCard) {
        hand[cardIdxInHand] = hand.back();
    }
    hand.pop_back();

    state.trickCards.push_back(playedCard);
    state.trickOwners.push_back(state.currentPlayerIdx);

    if (state.trumpAllPass == Suit::None && state.trickCards.size() == 1) {
        state.trumpAllPass = playedCard.getSuit();
    }

    if (state.trickCards.size() >= 3) {
        record.trickResolved = true;
        record.oldTrickCards = state.trickCards;
        record.oldTrickOwners = state.trickOwners;

        size_t winningTrickIdx = 0;
        if (state.gameType == GameType::Contract) {
            auto it = std::max_element(state.trickCards.begin(), state.trickCards.end(), BaseCardComparator(state.trumpContract));
            if (it->getSuit() != state.trumpContract) {
                it = std::max_element(state.trickCards.begin(), state.trickCards.end(), BaseCardComparator(state.trumpAllPass));
            }
            winningTrickIdx = std::distance(state.trickCards.begin(), it);
        } else {
            auto it = std::max_element(state.trickCards.begin(), state.trickCards.end(), BaseCardComparator(state.trumpAllPass));
            winningTrickIdx = std::distance(state.trickCards.begin(), it);
        }

        size_t actualWinner = state.trickOwners[winningTrickIdx];
        record.trickWinnerIdx = actualWinner;

        if (actualWinner == 0) state.tricksHuman++;
        else if (actualWinner == 1) state.tricksAI1++;
        else state.tricksAI2++;

        state.trickCards.clear();
        state.trickOwners.clear();

        state.trumpAllPass = Suit::None;
        state.currentPlayerIdx = actualWinner;
        state.round++;
    }
    else {
        state.currentPlayerIdx = (state.currentPlayerIdx + 1) % 3;
    }

    return record;
}

void Minimax::undoMove(MiniGameState& state, const UndoRecord& record) {
    if (record.trickResolved) {
        if (record.trickWinnerIdx == 0) state.tricksHuman--;
        else if (record.trickWinnerIdx == 1) state.tricksAI1--;
        else state.tricksAI2--;

        state.round--;
        state.trickCards = record.oldTrickCards;
        state.trickOwners = record.oldTrickOwners;
    } else {
        state.trickCards.pop_back();
        state.trickOwners.pop_back();
    }

    state.trumpAllPass = record.previousTrumpAllPass;
    state.currentPlayerIdx = record.previousPlayerIdx;

    auto& hand = getCurrentHand(state);

    if (!record.wasLastCard) {
        hand.push_back(hand[record.cardIdxInHand]);
        hand[record.cardIdxInHand] = record.playedCard;
    } else {
        hand.push_back(record.playedCard);
    }
}