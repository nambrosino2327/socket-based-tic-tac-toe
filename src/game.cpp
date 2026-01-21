#include "game.hpp"

// All possible combinations
static const int WIN_COMBOS[8][3] = {
	{0, 1, 2}, // Row 1
	{3, 4, 5}, // Row 2
	{6, 7, 8}, // Row 3

	{0, 3, 6}, // Col A
	{1, 4, 7}, // Col B
	{2, 5, 8}, // Col C

	{0, 4, 8}, // Diag A1:C3
	{2, 4, 6}, // Diag C1:A3
};

Game::Game() { reset(); }

void Game::reset() {
	m_board.fill(Cell::EMPTY);
	m_current = Player::P1;
}

bool Game::move(int pos, Player p) {
	if (!isValidMove(pos))
		return false;
	Cell c = (p == Player::P1 ? Cell::X : Cell::O);

	m_board[pos] = c;
	return true;
}

bool Game::isValidMove(int pos) const {
	return pos >= 0 && pos < 9 && m_board[pos] == Cell::EMPTY;
}

bool Game::checkWin(Player p) const {
	Cell c = (p == Player::P1 ? Cell::X : Cell ::O);

	for (const auto& combo : WIN_COMBOS) {
		if (m_board[combo[0]] == c && m_board[combo[1]] == c &&
			m_board[combo[2]] == c)
			return true;
	}

	return false;
}

bool Game::isDraw() const {
	if (checkWin(Player::P1) || checkWin(Player::P2)) {
		return false;
	}
	for (int i = 0; i < 9; i++) {
		if (m_board[i] == Cell::EMPTY)
			return false;
	}

	return true;
}

std::array<Cell, 9>& Game::board() { return m_board; }

const std::array<Cell, 9>& Game::board() const { return m_board; }

Player Game::activePlayer() const { return m_current; }

void Game::switchPlayer() {
	if (m_current == Player::P1) {
		m_current = Player::P2;
	} else {
		m_current = Player::P1;
	}
}