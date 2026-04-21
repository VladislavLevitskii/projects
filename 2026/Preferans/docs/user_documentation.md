# Preferans Card Game

## Project Overview
The goal of this project is to implement the Preferans 
card game for a single player against two AI opponents 
with a different difficulties.

## Installation

### Requirements

C++ compiler, CMake (or IDE e.g. Visual Studio)

### Linux (fedora): 

to install on the distribution use corresponding 
package manager e.g. `apt`

```bash
sudo dnf install SFML SFML-devel

mkdir build && cd build

cmake ..
make

./Preferans
```

Note that you need to use SFML 2.6 
(some packages already has 3.0 version)

In case you want to uninstall packages you can use: 

```bash
sudo dnf uninstall SFML SFML-devel
```
### Windows: 

```powershell
mkdir build
cd build

cmake ..

cmake --build . --config Release

cd Release
.\Preferans.exe
```

## Game Configuration & Difficulty

Before starting the game, the user must select one of 
three difficulty levels via the main menu. This selection 
determines the logic used by the two computer opponents:

Easy: AI performs semi-random moves, adhering only to 
basic game rules (valid card plays) without strategic 
planning.

Medium: AI utilizes a standard Minimax algorithm with 
limited recursion depth and a basic heuristic evaluation 
function. The AI has "perfect information".

Hard: AI utilizes a Minimax algorithm with extended depth.

## Basic rules

These rules are applied to every game mode:

After player/AI playes the card, next player that would 
play is clockwise.

All game is divided into 10 rounds, and every round has
its own trump suit.

### Mariage

This game is similar to auction mariage (known as mariáš).
But it has some improvements in scoring and has
more tactical possibilities.

### Bid

Bid is a combination of suit and value of a card.

### Legal Cards to Play

Players MUST follow the suit of the first card 
played in the trick.

- If a player does not have a card of the leading 
  suit, they MUST play a trump card.

- If a player has neither the leading suit nor a trump 
  card, they can play ANY card.

- The player who starts the trick (when the discard 
  pile is empty) can play ANY card.

### Scoring

After 10 rounds of the game, the score is calculated 
according to the game mode.

Note that precise points you get (or lose) doesn't 
matter much, what really matters is how many tricks
you get.

The scoring system defines 3 main components:

- Bullet (reward)
- Penalty (mountain)
- Whists (defender's score)

Constants:

- $L$ = Level of the bid<br>
- $M$ = Multiplication of the mountain (10)<br>
- $N$ = Number of players

#### Total recalculation

This is done at the end of each round:

Converts the Bullet and Penalty into a unified final 
score (Total) by comparing players against each other.

- Baseline Penalty: Find the player with the lowest 
Penalty score: $P_{\min}$

- Adjusted Penalty ($P_{adj}$): Subtract the minimum 
penalty from each player's penalty

$$P_{adj} = \text{Penalty} - P_{\min}$$

- Average Compensation ($C_{avg}$): The sum of all 
adjusted penalties multiplied by factor of the 
mountain M, distributed equally among all players.

$$C_{avg} = \frac{\sum P_{adj} \times M}{N}$$

- Final Score: 

$$\text{Total} = (\text{Bullet} \times M) + (C_{avg} - (P_{adj} \times M)) + \text{Whists}$$

## Main Menu

Display has "Start" button and "Difficulty" selector (Easy, 
Medium, Hard).

Action: Start" initializes the deck and deals cards to 
the 3 players (User + 2 Bots).

## Game modes

Game provides 3 possible game modes to play, note that 
they have different scoring systems.

### Contract

Before every contract game is bidding phase:

#### Bidding Phase

If player didn't pass, he is able to announce higher
bid or pass (if AI wants play too).

Player can choose always higher bid.

Note that if player plays higher bid, later if 
he fulfills the contract he gets more points.

The standard order of the suits:

Spades < Clubs < Diamonds < Hearts

That means that if one player claims to take 8 spades,
next player have to increase to 8 Clubs or pass.
And 8 clubs is becoming the minimum of possible contract.

#### Contract rules

If player overtook the AI, he takes 2 cards from 
the talon and the game waits until player discards 
2 other cards (by clicking on them, then they disappears).

Then the button with the bids turns red (active),
but before player can choose higher bid.

#### Contract game

After that the contract itself starts. The rules 
are pretty much the same as default one. But 
the trump is always the same as player stated in
the contract. 

In the case that discard pile doesn't have trump
suit, there is temporary trump that is first card
that was played.

#### Scoring 

A player that started contract (obviously) wants
to fulfill the contract and if he gets less tricks
than he stated, he gets minus points, if not he gets 
more points.

Note that if player stated e.g. 6 of clubs and 
gets 9 tricks, he will get much less points than if 
he would stated 9 of clubs.

The contract value ($V$) is derived from the 
target contract tricks ($C$):

$$V = C - L$$

Declarer succeeds (Tricks $\ge C$): Earns a 
reward in the Bullet.

$$\text{Bullet} \mathrel{+}= V$$

Declarer fails: Receives penalty points for each 
missing trick.

$$\text{Penalty} \mathrel{+}= (C - \text{Tricks}) \times V$$

Defenders (others): Earn whists for every trick they 
take.

$$\text{Whists} \mathrel{+}= \text{Tricks} \times V$$

### All pass

That is different game mode from the contract 
(and much simpler).

This game mode is enabled if and only if every player
passes.

Players try to have as less as possible tricks.

There is no main trump, only temporary one in discard pile
as stated in default rules.

#### Scoring

The most tricks player gets minus points and the least
tricks player gets positive points.

Note that if 2 players gets 5 and 5 tricks and the 
other one gets 0 tricks. Both these 2 players get minus 
points. That means that sum of the all points doesn't 
change.

### Misere

The rules are the same as the all pass game mode,
but the player that stated misere, he claims that
he would get <b>0</b> tricks.

#### Scoring

Pointswise this game mode is equal to 9 value contract 
play (fulfilled).

Declarer succeeds (0 tricks taken):

$$\text{Bullet} \mathrel{+}= 9$$

Declarer fails: Receives a penalty for every trick 
taken.

$$\text{Penalty} \mathrel{+}= \text{Tricks} \times 9$$

## End

After 10 rounds player can see every other players
points and he can start a new game.
