# Socket-Based Tic-Tac-Toe

A lightweight, terminal-based Tic-Tac-Toe game implemented in modern C++.  
It features a **multithreaded server**, a **compact binary protocol**, and **two responsive clients** communicating over TCP.

This project is designed to be simple to run, easy to read, and a solid example of small-scale network programming in C++.

---

## Features

- **Multithreaded server**  
  Each client connection is handled in its own thread, with shared game state protected by a mutex.

- **Binary protocol**  
  Messages between client and server are compact, structured, and endian-safe.

- **Real-time board updates**  
  Both clients receive board updates and turn notifications immediately after each move.

- **Server-side validation**  
  The server enforces turn order, move validity, win/draw detection, and clean shutdown.

- **Graceful quitting**  
  Clients can exit with `q` or Ctrl‑C, and the server handles disconnects cleanly.

---

## Build Instructions
1. Run $make with a C++17 compatiable compiler (default is g++ version 20)
This will produce:
    - bin/server
    - bin/client

## How to Play
1. Start the server with bin/server. This opens a TCP listener on 127.0.0.1:8080
1. Start two clients in separte terminals with bin/client. The first client
becomes Player 1 (X), the second Player 2 (O)
1. Play the game  
    - On your turn, enter a number 1–9 to place your mark.  
    - Enter `q` to quit at any time.  
    - The board updates after every valid move.
1. The game ends when a player gets 3 symbols in a row, or the board fills up

## Future improvements
- More rigid and extensible protocol
- More customization options (e.g. name, player color)
- Better unit tests and error handling