#pragma once
#ifndef GAME_HPP
#define GAME_HPP

#include <array>

// Enum for cells on the 3x3 board
enum class Cell { EMPTY, X, O };

// Enum for player
enum class Player { P1, P2 };

// Game class to encapsulate game logic to be ran on the server
class Game {
  public:
	// Game class constructor
	Game();
	// Reset game
	void reset();
	// Apply a player's move to a cell on the board
	bool move(int pos, Player p);
	// Check if a cell is occupied
	bool isValidMove(int pos) const;

	// Check to see if a win condition is met
	bool checkWin(Player p) const;
	// Check to see if a draw occurs
	bool isDraw() const;

	// Accessor for the game's board
	std::array<Cell, 9>& board();
	// Accessor for the game's board
	const std::array<Cell, 9>& board() const;

	// Currently active player (p1 or p2)
	Player activePlayer() const;
	// Switch turn to the other player
	void switchPlayer();

  private:
	// Private board member
	std::array<Cell, 9> m_board;
	// Private member to hold current player
	Player m_current;
};

#endif