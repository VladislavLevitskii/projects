# Documentation Preferans Card Game
## Project Overview
The goal of this project is to implement the Preferans card game for a single player against two AI opponents with a different difficulties.

## Game Configuration & Difficulty
Before starting the game, the user must select one of three difficulty levels via the main menu. This selection determines the logic used by the two computer opponents:

Easy: AI performs semi-random moves, adhering only to basic game rules (valid card plays) without strategic planning.

Medium: AI utilizes a standard Minimax algorithm with limited recursion depth and a basic heuristic evaluation function.

Hard: AI utilizes a Minimax algorithm with extended depth. Additionally, the AI has "perfect information" (it can see the user's cards) to simulate an expert-level opponent.

## User Interface & Interaction Flow
The application flow is linear, cycling through the following phases:

### Main Menu
Elements: "Start" button, "Difficulty" selector (Easy, Medium, Hard).

Action: Start" initializes the deck and deals cards to the 3 players (User + 2 Bots).

### Bidding Phase
Display: The user sees their own 10 cards sorted by suit/rank.

Controls: A bidding panel displays available bids (e.g., Pass, Whist, Play).

Validation: Bids lower than the current highest bid must be disabled (greyed out).

Interaction: The user clicks a button to announce their bid.

### The Talon & Discard (Active Player Only)
Condition: If the user wins the bidding.

Display: The two talon cards are revealed.

Interaction: The user must select 2 cards from their hand (now 12 cards total) to discard.

Mechanism: Click to select/deselect cards, then click "Confirm Discard".

Contract Selection: A modal/popup appears asking for the final contract (Trump Suit or Misère).

### The Play (Trick-Taking)
Display:

User's hand (bottom, face up).

Opponents' hands (left/right, backs visible).

Played cards (center of the table).

Interaction: The user clicks on a card in their hand to play it.

Logic & Validation: The system must strictly enforce Preferans rules:

Must follow suit.

Must Trump if void in suit (if applicable by rule variation).

Error Handling: If an invalid card is clicked, the move is rejected The turn does not advance until a valid card is played.

### End of Round & Scoring
Trigger: Triggered after the 10th trick is played.

Display: A summary screen showing the calculated score:

Whists: Points for tricks taken.

Soup (Pool): Contract fulfillment points.

Mountain: Penalty points.

Interaction: Buttons for "Next Round" (keeps current score, redeals) or "Exit to Menu".

## Technical Requirements
Players: Strictly fixed to 3 participants (1 Human, 2 AI).

Deck: 32-card deck (7 through Ace).

AI Evaluation: A heuristic function must be implemented to assign a numerical value to a game state (board position + hand strength) to support the Minimax algorithm.

## Steps to install the game:

### linux: 

```
sudo dnf install SFML SFML-devel

cd build

cmake ..

make

./Preferans
```

#### in case you want to uninstall packages you can use: 

```
sudo dnf uninstall SFML SFML-devel
```
### win: 

```
cd build

cmake ..

cd .. 

cmake --build build_vs --config Release

cd .\build\Release\

.\Preferans.exe
```