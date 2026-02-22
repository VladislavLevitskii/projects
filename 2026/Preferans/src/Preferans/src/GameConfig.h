#pragma once
#include <SFML/Graphics.hpp>

constexpr int   MAX_HAND_SIZE           = 10;
constexpr int   NUMBER_OF_PLAYERS       = 3;
constexpr int   NUMBER_OF_CARDS         = 32;
constexpr float TIME_BETWEEN_MOVE       = 0.1f;
constexpr int   NUMBER_OF_DIFFICULTIES  = 3;

const sf::Vector2f PLAYER_POSITION_FOR_CARDS    { 200,  600 };
const sf::Vector2f PLAYER_POSITION_FOR_HANDS    { 250,  450 };
const sf::Vector2f AI1_POSITION_FOR_CARDS       { 50,   25 };
const sf::Vector2f AI1_POSITION_FOR_HANDS       { 200,  25 };
const sf::Vector2f AI2_POSITION_FOR_CARDS       { 1000, 25 };
const sf::Vector2f AI2_POSITION_FOR_HANDS       { 850,  25 };
const sf::Vector2f TALON_POSITION_FOR_CARDS     { 500,  25 };
const sf::Vector2f DISCARD_PILE_FOR_CARDS       { 450,  300 };
const sf::Vector2f PLAYER_POSITION_FOR_SCORE    { 375,  550 };
const sf::Vector2f AI1_POSITION_FOR_SCORE       { 200,  200 };
const sf::Vector2f AI2_POSITION_FOR_SCORE       { 850,  200 };

const int CARD_WIDTH        = 523;
const int CARD_HEIGHT       = 760;
const int CARDS_ONE_SUIT    = 13;

constexpr float PADDING_BOTTOM = 50.f;

constexpr int CHARACTER_SIZE_BIG = 24;
constexpr int CHARACTER_SIZE_NORMAL = 20;
const sf::Color DIFFICULTY_TEXT_COLOR = sf::Color::White;
constexpr float DIFFICULTY_TEXT_POSITION_Y = 50.f;

const sf::Vector2f START_BUTTON_SIZE = { 150.f, 50.f };
const sf::Vector2f BUTTON_DIFFICULTY_SIZE = { 120.f, 40.f };

const sf::Color BUTTON_START_DIFFICULTY_COLOR           = { 120, 120, 120 };
const sf::Color BUTTON_START_DIFFICULTY_COLOR_HOVER     = { 100, 100, 100 };
const sf::Color BUTTON_HARD_DIFFICULTY_COLOR            = { 200, 100, 100 };
const sf::Color BUTTON_HARD_DIFFICULTY_COLOR_HOVER      = { 150, 50,  50 };
const sf::Color BUTTON_MEDIUM_DIFFICULTY_COLOR          = { 200, 200, 100 };
const sf::Color BUTTON_MEDIUM_DIFFICULTY_COLOR_HOVER    = { 150, 150, 50 };
const sf::Color BUTTON_EASY_DIFFICULTY_COLOR            = { 100, 200, 100 };
const sf::Color BUTTON_EASY_DIFFICULTY_COLOR_HOVER      = { 50,  150, 50 };

const std::string DIFFICULTY_EASY_STRING = "Difficulty: Easy";
const std::string DIFFICULTY_MEDIUM_STRING = "Difficulty: Medium";
const std::string DIFFICULTY_HARD_STRING = "Difficulty: Hard";

enum class GameState { Menu, Game, End };
enum class GameDifficulty { Easy, Medium, Hard };
enum class Suit { None, Spades, Clubs, Diamonds, Hearts };
enum class GameType { None, Bidding, Contract, AllPass, Misere };