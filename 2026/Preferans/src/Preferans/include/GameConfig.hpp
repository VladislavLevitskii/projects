#pragma once
#include <SFML/Graphics.hpp>

#include <vector>
#include <memory>

inline constexpr int   MAX_HAND_SIZE					= 10;
inline constexpr int   NUMBER_OF_PLAYERS				= 3;
inline constexpr int   NUMBER_OF_CARDS					= 32;
inline constexpr float TIME_BETWEEN_MOVE				= 0.3f;
inline constexpr float TIME_BETWEEN_CLICKS_BUTTON		= 0.3f;
inline constexpr float SHIFT_BETWEEN_CARDS				= 50.f;
inline constexpr int   NUMBER_OF_DIFFICULTIES			= 3;
inline constexpr int   NUMBER_OF_REGIMES				= 3;
inline constexpr int   MULTIPLICATION_FACTOR_MOUNTAIN	= 10;
inline constexpr int   NUMBER_OF_SUITS					= 4;
inline constexpr int   NUMBER_OF_LEVEL_BID				= 5;
inline constexpr int   START_BID						= 6;

// %
inline constexpr int	AI_CHANCE_TO_MISERE_EASY		= 2;
inline constexpr int	AI_CHANCE_TO_CONTRACT_EASY		= 50;
inline constexpr int	AI_CHANCE_TO_STAY_BID_EASY		= 90;

inline const sf::Vector2f PLAYER_POSITION_FOR_CARDS    { 200,  600 };
inline const sf::Vector2f AI1_POSITION_FOR_CARDS       { 50,   25 };
inline const sf::Vector2f AI2_POSITION_FOR_CARDS       { 1000, 25 };

inline const sf::Vector2f PLAYER_POSITION_FOR_HANDS    { 250,  450 };
inline const sf::Vector2f AI1_POSITION_FOR_HANDS       { 200,  25 };
inline const sf::Vector2f AI2_POSITION_FOR_HANDS       { 850,  25 };

inline const sf::Vector2f TALON_POSITION_FOR_CARDS     { 500,  25 };
inline const sf::Vector2f DISCARD_PILE_FOR_CARDS       { 450,  300 };

inline const sf::Vector2f PLAYER_POSITION_FOR_SCORE    { 375,  550 };
inline const sf::Vector2f AI1_POSITION_FOR_SCORE       { 200,  200 };
inline const sf::Vector2f AI2_POSITION_FOR_SCORE       { 800,  200 };

inline const sf::Vector2f PLAYER_POSITION_FOR_REGIME	{ 600,	550 };
inline const sf::Vector2f AI1_POSITION_FOR_REGIME		{ 200,	250 };
inline const sf::Vector2f AI2_POSITION_FOR_REGIME		{ 800,	250 };

inline constexpr int CARD_WIDTH							= 523;
inline constexpr int CARD_HEIGHT						= 760;
inline constexpr int CARDS_ONE_SUIT						= 13;

inline constexpr size_t MEDIUM_DEPTH_MINIMAX			= 7;
inline constexpr size_t HARD_DEPTH_MINIMAX				= 10;

inline constexpr float		PADDING_BOTTOM				= 50.f;
inline constexpr int		CHARACTER_SIZE_BIG			= 24;
inline constexpr int		CHARACTER_SIZE_NORMAL		= 20;
inline constexpr float		DIFFICULTY_TEXT_POSITION_Y	= 50.f;
inline const sf::Color		DIFFICULTY_TEXT_COLOR		= { 255, 255, 255 };

inline const sf::Vector2f START_BUTTON_SIZE			= { 150.f, 50.f };
inline const sf::Vector2f BUTTON_DIFFICULTY_SIZE	= { 120.f, 40.f };

inline const sf::Color BUTTON_START_DIFFICULTY_COLOR			= { 120, 120, 120 };
inline const sf::Color BUTTON_START_DIFFICULTY_COLOR_HOVER		= { 100, 100, 100 };
inline const sf::Color BUTTON_HARD_DIFFICULTY_COLOR				= { 200, 100, 100 };
inline const sf::Color BUTTON_HARD_DIFFICULTY_COLOR_HOVER		= { 150, 50,  50 };
inline const sf::Color BUTTON_MEDIUM_DIFFICULTY_COLOR			= { 200, 200, 100 };
inline const sf::Color BUTTON_MEDIUM_DIFFICULTY_COLOR_HOVER		= { 150, 150, 50 };
inline const sf::Color BUTTON_EASY_DIFFICULTY_COLOR				= { 100, 200, 100 };
inline const sf::Color BUTTON_EASY_DIFFICULTY_COLOR_HOVER		= { 50,  150, 50 };
inline const sf::Color BUTTON_RESTART_COLOR						= { 120, 120, 120 };
inline const sf::Color BUTTON_RESTART_COLOR_HOVER				= { 100, 100, 100 };

inline const std::string DIFFICULTY_EASY_STRING		= "Difficulty: Easy";
inline const std::string DIFFICULTY_MEDIUM_STRING	= "Difficulty: Medium";
inline const std::string DIFFICULTY_HARD_STRING		= "Difficulty: Hard";

enum class GameState { Menu, Game, End };
enum class GameDifficulty { Easy, Medium, Hard };
enum class Suit { None, Spades, Clubs, Diamonds, Hearts };
enum class GameType { None, Bidding, Contract, AllPass, Misere };

class Player;
using PlayerList = std::vector<std::unique_ptr<Player>>;