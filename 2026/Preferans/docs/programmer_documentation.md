# Programmer Documentation

## Game Architecture

The game is built in C++ using the SFML library for 
rendering. The design separates game logic, graphical 
representation (UI), and minimax algorithm.

### The Card Container & Player Hierarchy

- `CardContainer`: The root class. It simply contains 
a std::vector<Card> hand and a virtual destructor, 
allowing any derived entity to store and manage cards.

- The Pile Branch (`TablePile`): Inherits from 
CardContainer and adds basic 2D positional data 
for rendering.

- `Talon` and `DiscardPile`: they inherit from TablePile, 
adding specific logic and layout data for their roles 
on the board (e.g., rendering specific slots for thrown 
cards). 

	Note that `DiscardPile` should have constant size 
	(3), therefore you can't call `clear()`, it is done to 
	make moves easier. For example current player would call 
	something like: 

	`discardPile.hand[PlayerIdx] = std::move(hand[cardIdx]);`

	This line above overrides cards, so you need to call 
	this after:

	`discardPile.hand[0].setVisible(true);`
    `discardPile.hand[0].setFront();`

	But because after the round game should "clear" this
	pile, it just turns this cards invisible.

- The Player Branch (`Player`): Inherits from CardContainer. 
It adds player-specific data (scores, chosen regime, 
UI text) and defines the core pure virtual functions 
for every phase of the game (e.g., makeMoveAllPass, 
makeMoveContract...).

- `HumanPlayer`: Implements these virtual methods with 
empty or passive bodies. Human moves are not algorithmic; 
they are driven by UI click events processed in the 
main Game loop.

- `AI`: Overrides these methods to trigger decision-making 
algorithms (heuristics or the Minimax search) to 
automatically determine and execute the best move.

In the Game class, players are stored polymorphically 
as a list of base pointers: 
`std::vector<std::unique_ptr<Player>> listOfPlayers`.

This allows the game loop to be entirely agnostic 
about who is taking a turn. The game simply calls 
`listOfPlayers[currentPlayerIdx]->makeMoveContract(...)`. 

If it's a bot, it calculates and plays. If it's a human, 
the function does nothing and the game waits for valid 
graphical input.

### The Card Hierarchy

- `CardBase`: A lightweight data class holding only 
the Suit and numeric value. It has no knowledge of 
rendering.

- `Card`: Inherits from CardBase and adds SFML 
specifics (sf::Sprite, texture pointers, coordinates, 
hover/click states).

This is done for performance. When the AI simulates 
thousands of future game states, it only uses CardBase. 
Trying to copy SFML graphical objects during a recursive 
search would destroy performance.

### Bidding grid

The bidding phase is managed by the `BiddingGrid` class, 
which encapsulates the logic for valid bid progression 
using a 2D coordinate system.

- `Bid`: A lightweight data object that stores a specific 
contract bet, combining a Suit and an integer value. 

- `BiddingGrid Structure`: The BiddingGrid initializes 
a 2D array. The rows represent the bid's numerical level 
and the columns represent the suit hierarchy (Spades, 
Clubs, Diamonds, Hearts). This pre-generation ensures 
that all valid bids exist in memory from the start.

- **State and Progression Logic**:
The grid maintains its current state using a coordinate 
pair. Instead of manually validating whether a new bid 
is legally higher than the previous one, the game 
simply calls `increaseBid()` or `decreaseBid()`.

## Game State

The game holds the SFML window, the UI instance, 
and the player list. It operates on a strictly defined 
state machine using `GameState` (Menu, Game, End) and 
`GameType` (None, Bidding, AllPass, Contract, Misere).

The main `update()` loop reads the current state and 
routes execution to specific handlers (processBidding, 
processAllPass, processContract). This prevents the game 
loop from turning into a massive, unreadable set of 
if/else statements. 

Time delays (`aiClock`) are used to 
ensure that AI moves don't happen instantaneously, 
allowing the human player to visually track what cards 
the bots are playing.

## UI System & Interactions

The graphical interface is encapsulated within the 
UI class. The Game class does not draw shapes directly; 
instead, it passes references of the current game state 
(players, piles) to the UI methods (e.g., `ui.drawTable`, 
`ui.drawSelectingRegime`).

Note that communication between UI and Game is not one 
way, some UI methods like `drawSelectingRegime()`, returns
bool for the Game, that is because game can't interact 
with the user directly, so it uses UI.

### Buttons

The Button class abstracts basic SFML rectangles and text, 
handling hover states, colors, and click detection 
(`isClicked()`).

#### Hover and click Logic

Inside the Game class (`checkHoveringAllPass`, 
`checkCardClicked`), mouse coordinates are mapped to the 
world space and checked against the bounds of the Card 
sprites in the human player's hand. If a card is legally 
playable (checked against the current trump and game rules), 
it reacts visually (moves up).

## The Minimax Algorithm

The AI's decision-making in complex game modes relies 
on the Minimax algorithm with Alpha-Beta pruning. 
Because Preferans involves hidden information 
(other players' hands), the current implementation gives 
the AI **perfect information** (it knows the human's cards) 
on Medium and Hard difficulty, creating a formidable 
opponent.

Note that the Minimax algorithm is not used in Easy 
difficulty. This is because this difficulty is random, 
meaning the AI doesn't strategically evaluate cards. 
Since it always has at least one possible move, the AI 
just uniformly and randomly selects one of the possible 
cards to play.

### State Representation - `MiniGameState`

To search efficiently, the AI creates a detached, 
lightweight snapshot of the game: MiniGameState. 
It strips away all SFML textures, buttons, and UI 
elements. It only contains `std::vector<CardBase>`, 
current scores, and trump suits. This allows the 
algorithm to copy and evaluate game states in memory 
instantly.

### Move Generation and bitwise Operations

Finding legal moves is a optimized process. 
Instead of returning arrays of cards, the engine uses 
bitmasks (uint16_t).

A 16-bit integer perfectly represents a hand (maximum 
10 cards).

For example: if the 3rd bit is 1, the 3rd card in the 
hand is a legal move.

The function `std::countr_zero(legalMoves)` is used to 
quickly find the index of the lowest set bit. The bit 
is then cleared (`legalMoves &= (legalMoves - 1)`). 
This allows the AI to loop through available moves using 
single CPU clock-cycle instructions rather than slow 
loop iterators.

### The Search Tree and Undo System

Instead of copying the MiniGameState for every possible 
future move (which would cause massive memory allocation 
overhead), the AI uses an Apply/Undo architecture.

- **Apply**: applyMove alters the current state (moves a 
card to the trick, updates score if a trick is finished) 
and returns an UndoRecord.

- **Evaluate**: The AI recursively calls evaluateState.

- **Undo**: undoMove uses the UndoRecord to perfectly 
restore the state to how it was before the move.

- **Heuristic Evaluation**: The evaluation function 
(`evaluateState`) is the "brain" of the AI. When the 
search reaches its maximum depth, or the game ends, 
it evaluates the board.

The primary metric is tricksHuman (the number of tricks 
the human player has won).

In All Pass / Misere: The human wants to win zero tricks. 
Therefore, the AI acts to force tricks onto the human. 
The evaluation returns tricksHuman (positive), meaning 
the AI maximizes this number to ensure the human gets 
heavily penalized.

- **Alpha-Beta Pruning**: As the AI searches, it keeps 
track of the worst-case scenario (alpha and beta). 
If it discovers a branch where the human has a move that 
completely destroys the AI's score, it stops searching 
that branch immediately (pruning), saving time.
