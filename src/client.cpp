#include "game.hpp"
#include "protocol.hpp"
#include "utils.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace TTT_PROTO;

// Local sockfd
int sockfd = -1;

// Helper method to recv all bytes from a vector to a socket at
static bool recv_all(int sockfd, void *buf, size_t len) {
  u_int8_t *p = reinterpret_cast<uint8_t *>(buf);
  size_t total = 0;
  ssize_t n;
  while (total < len) {
    if ((n = recv(sockfd, p + total, len - total, 0)) <= 0)
      return false; // disconnection or error
    total += static_cast<size_t>(n);
  }
  return true;
}

// Helper method to send all bytes from a vector to a socket at
static bool send_all(int sockfd, const std::vector<uint8_t> &data) {
  size_t total = 0, len = data.size();
  ssize_t n;
  while (total < len) {
    if ((n = send(sockfd, data.data() + total, len - total, 0)) <= 0)
      return false;
    total += static_cast<size_t>(n);
  }
  return true;
}

/**
 * Handle player input. Returns:
 * 0: Successful
 * 1: Quit input
 * 2: Unsucessful
 */
static int handle_input(int local_id) {
  using std::cout, std::endl;
  const char *color = (local_id == 1 ? C_P1 : C_P2);

  cout << color << "Your move\nSelect cell 1-9\nPress q to quit: " << C_RST;
  std::string in;
  std::getline(std::cin, in);

  // Quit input
  if (in == "q") {
    std::vector<uint8_t> out;
    serialize(MsgType::QUIT_REQUEST, nullptr, 0, out);
    send_all(sockfd, out);
    cout << "You have exited the game. Goodbye." << endl;
    close(sockfd);
    return 1;
  }

  // Empty input check
  if (in.empty()) {
    cout << color << "Invalid input!" << C_RST << endl;
    return 2;
  }

  // Non-numeric input check
  bool numeric = std::all_of(in.begin(), in.end(), ::isdigit);
  if (!numeric) {
    cout << color << "Invalid input!" << C_RST << endl;
    return 2;
  }

  int pos = std::stoi(in) - 1;

  // Validate number range
  if (pos < 0 || pos > 8) {
    cout << color << "Cell must be between 1 and 9" << C_RST << endl;
    return 2;
  }

  PL_MovReq req{(uint8_t)pos};
  std::vector<uint8_t> out;
  serialize(MsgType::MOVE_REQUEST, &req, sizeof(req), out);
  send_all(sockfd, out);
  return 0;
}

// Handle SIGINT (ctrl + c) as another means to quit, as well as SIGTERM
static void handle_quit(int) {
  if (sockfd != -1) {
    std::vector<uint8_t> out;
    serialize(MsgType::QUIT_REQUEST, nullptr, 0, out);
    send_all(sockfd, out);
    std::cout << "\nYou have exited the game. Goodbye." << std::endl;
    close(sockfd);
  }
  exit(0);
}

// Main method
int main() {
  using std::cout, std::endl;

  /**
   * Open socket
   */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    fatal_error(1, "Error opening socket");

  /**
   * Connect to server on localhost:8080
   */
  int portno = 8080;
  sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  if (connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    fatal_error(1, "Error connecting to server");

  cout << "Successfully connected to server" << endl;
  signal(SIGINT, handle_quit);
  signal(SIGTERM, handle_quit); // Another way of quitting (rarer)

  // Store the player's id locally
  int local_id = 0;
  // Local game state
  Game local_game;

  /**
   * Main game loop
   */
  while (true) {
    /**
     * Read header
     */
    MsgHeader hdr;
    if (!recv_all(sockfd, &hdr, sizeof(hdr))) {
      cout << "Disconnected from server" << endl;
      break;
    }

    /**
     * Read payload
     */
    std::vector<uint8_t> pl(hdr.size);
    if (hdr.size > 0) {
      if (!recv_all(sockfd, pl.data(), hdr.size)) {
        cout << "Disconnected from server" << endl;
        break;
      }
    }

    /**
     * Determine signal type and respond accordingly
     */
    MsgType type = static_cast<MsgType>(hdr.type);
    switch (type) {
    case MsgType::WELCOME: {
      PL_Welcome welcome;
      memcpy(&welcome, pl.data(), sizeof(welcome));
      local_id = welcome.p_id;

      const char *color = (local_id == 1 ? C_P1 : C_P2);
      cout << "\x1b[38;5;206m"
              "\x1b[1m"
              "/// Socket-based Tic Tac Toe /// " C_RST "\n\x1b[38;5;206m"
              "By Nathan Ambrosino\n\n" C_RST
           << color << "Welcome, you are player " << local_id << C_RST << endl;
    } break;

    case MsgType::BOARD_UPDATE: {
      PL_Board pl_b;
      memcpy(&pl_b, pl.data(), sizeof(pl_b));

      // Update local board state
      auto &b = local_game.board();
      for (int i = 0; i < 9; i++) {
        b[i] = static_cast<Cell>(pl_b.cells[i]);
      }
      displayBoard(local_game);
    } break;

    case MsgType::TURN: {
      uint8_t turn = pl[0];
      const char *color = (turn == 1 ? C_P1 : C_P2);
      cout << color << "It's Player " << (int)turn << "'s turn" << C_RST
           << endl;

      // Current player's turn
      if (turn == local_id)
        while (true) {
          int r = handle_input(local_id);
          if (r == 0)
            break;
          if (r == 1)
            return 0;
        }
    } break;

    case MsgType::MOVE_RESULT: {
      PL_MovRes mv_rs;
      memcpy(&mv_rs, pl.data(), sizeof(mv_rs));

      if (mv_rs.status == 0) {
        cout << "Move successfully applied!" << endl;
      } else {
        cout << "Invalid move!" << endl;
        while (true) {
          int r = handle_input(local_id);
          if (r == 0)
            break;
          if (r == 1)
            return 0;
        }
      }
    } break;

    case MsgType::WIN: {
      uint8_t winner = pl[0];
      // Bright green for win, bright red for loss
      const char *color =
          (winner == local_id ? "\x1b[38;5;46m" : "\x1b[38;5;196m");
      cout << color << "*** GAME OVER: ";
      if (winner == local_id) {
        cout << "YOU WIN! ***" << C_RST << endl;
      } else {
        cout << "YOU LOSE! ***" << C_RST << endl;
      }
      displayBoard(local_game);
      close(sockfd);
      return 0;
    } break;

    case MsgType::DRAW: {
      cout << "\x1b[38;5;51m"
        "*** GAME OVER: It's a draw! ***"
        "\x1b[0m\n";
      displayBoard(local_game);
      close(sockfd);
      return 0;
    } break;

    case MsgType::ERROR: {
      std::string m(pl.begin(), pl.end());
      cout << "Server err: " << m << endl;
    } break;

    default: {
      cout << "ERR: Undefined message type received!" << endl;
    } break;
    } // signal switch

  } // main game loop

  /**
   * End client session
   */
  close(sockfd);
  return 0;
}