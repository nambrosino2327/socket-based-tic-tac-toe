#include "utils.hpp"
#include "game.hpp"
#include <iostream>

void fatal_error(int errCode, const char* m) {
	std::cerr << "\x1b[38;5;196m" << m << "\x1b[0m" << std::endl;
	exit(errCode);
}

static void displayCell(const Cell c) {
	switch (c) {
	case Cell::X:
		std::cout << C_P1 << 'X' << C_RST;
		break;
	case Cell::O:
		std::cout << C_P2 << 'O' << C_RST;
		break;
	default:
		std::cout << '.';
		break;
	}
}

void displayBoard(Game& g) {
	using std::cout;

	auto& b = g.board();

	for (int row = 0; row < 3; row++) {
		cout << " ";
		displayCell(b[3 * row]);
		cout << " | ";
		displayCell(b[3 * row + 1]);
		cout << " | ";
		displayCell(b[3 * row + 2]);
		std::cout << "\n";

		if (row < 2)
			cout << "---+---+----\n";
	}
}

void displayBoard(const Game& g) {
	using std::cout;

	auto& b = g.board();

	for (int row = 0; row < 3; row++) {
		cout << " ";
		displayCell(b[3 * row]);
		cout << " | ";
		displayCell(b[3 * row + 1]);
		cout << " | ";
		displayCell(b[3 * row + 2]);
		std::cout << "\n";

		if (row < 2)
			cout << "---+---+----\n";
	}
}