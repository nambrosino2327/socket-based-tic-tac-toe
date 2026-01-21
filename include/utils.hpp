#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include "game.hpp"

// Player 1 color (X)
#define C_P1 "\x1b[38;5;27m"
// Player 2 color (O)
#define C_P2 "\x1b[38;5;167m"
// Default color
#define C_RST "\x1b[0m"

// Prints fatal errors (in red), then terminates program
void fatal_error(int errCode, const char* m);

// Display the entire 3x3 grid
void displayBoard(Game& g);
// Display the entire 3x3 grid
void displayBoard(const Game& g);

#endif