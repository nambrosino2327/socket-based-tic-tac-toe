#include "game.hpp"
#include "protocol.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

// Number of currently connected clients
std::atomic<int> current_connections(0);
// Shared game object
Game g;
// Mutex to protect game state
std::mutex game_mutex;
// Server sockfd
int serv_fd = -1;
// Array to hold both player sockfd's
int player_socket[] = {-1, -1};

using namespace TTT_PROTO;

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

// Helper method to send board data to a socket
static bool send_board(int sockfd) {
  PL_Board pl;

  const auto &b = g.board();
  for (int i = 0; i < 9; i++)
    pl.cells[i] = static_cast<uint8_t>(b[i]);
  std::vector<uint8_t> out;
  if (serialize(MsgType::BOARD_UPDATE, &pl, sizeof(pl), out)) {
    std::cerr << "Error serializing board" << std::endl;
    return false;
  }
  if (!send_all(sockfd, out)) {
    std::cerr << "Error sending board" << std::endl;
    return false;
  }

  return true;
}

void handle_quit(int) {
  if (serv_fd != -1)
    close(serv_fd);
  std::cout << "\nShutting down server" << std::endl;
  exit(0);
}

// Method used to handle logic for individual clients
void handle_client(int sockfd, int player_id) {
  using std::cout, std::endl;

  cout << "Player " << player_id << " connected on socket " << sockfd << endl;

  /**
   * Welcome the player
   */
  {
    PL_Welcome pl{(u_int8_t)player_id};
    std::vector<uint8_t> out;
    serialize(MsgType::WELCOME, &pl, sizeof(pl), out);
    send_all(sockfd, out);
  }

  /**
   * After both players join, send the empty board + turn
   */
  {
    std::lock_guard<std::mutex> lock(game_mutex);
    if (player_socket[0] != -1 && player_socket[1] != -1) {
      if (player_id == 2) {
        send_board(player_socket[0]);
        send_board(player_socket[1]);

        uint8_t active_pl = (g.activePlayer() == Player::P1 ? 1 : 2);
        std::vector<uint8_t> buf;
        serialize(MsgType::TURN, &active_pl, sizeof(active_pl), buf);
        send_all(player_socket[0], buf);
        send_all(player_socket[1], buf);
      }
    }
  }

  /**
   * Main game loop
   */
  while (true) {

    /**
     * Read header + payload, break if either don't send,
     * indicating a disconnection
     */
    MsgHeader hdr;
    if (!recv_all(sockfd, &hdr, sizeof(hdr))) {
      cout << "Player " << player_id << " disconnected (header read failed)\n";
      break;
    }

    std::vector<uint8_t> pl(hdr.size);
    if (hdr.size > 0) {
      if (!recv_all(sockfd, pl.data(), hdr.size)) {
        cout << "Player " << player_id
             << " disconnected (payload read failed)\n";
        break;
      }
    }

    /**
     * Read user request (either to move or to quit)
     */
    MsgType type = static_cast<MsgType>(hdr.type);

    // Ensure move request is not malformed
    if (type == MsgType::MOVE_REQUEST) {
      if (pl.size() < sizeof(PL_MovReq)) {
        std::string m = "ERR: MALFORMED MOVE REQUEST";
        std::vector<uint8_t> out;
        serialize(MsgType::ERROR, m.data(), m.size(), out);
        send_all(sockfd, out);
        continue;
      }

      PL_MovReq mv_req;
      std::memcpy(&mv_req, pl.data(), sizeof(mv_req));
      int pos = mv_req.pos;

      std::lock_guard<std::mutex> lock(game_mutex);

      // Ensure player doesn't send request out of turn
      int active = (g.activePlayer() == Player::P1 ? 1 : 2);
      if (player_id != active) {
        std::string m = "Not your turn";
        std::vector<uint8_t> out;
        serialize(MsgType::ERROR, m.data(), m.size(), out);
        send_all(sockfd, out);
        continue;
      }

      // Check move request to ensure it's valid. Send result to client
      Player p = (player_id == 1 ? Player::P1 : Player::P2);
      bool valid = g.move(pos, p);
      {
        PL_MovRes mv_res;
        mv_res.status = valid ? 0 : 1;
        std::vector<uint8_t> out;
        serialize(MsgType::MOVE_RESULT, &mv_res, sizeof(mv_res), out);
        send_all(player_socket[player_id - 1], out);
      }
      if (!valid)
        continue;

      // Display board
      send_board(player_socket[0]);
      send_board(player_socket[1]);

      /**
       * Check win/draw conditions
       */

      // Win
      if (g.checkWin(p)) {
        uint8_t r = player_id;
        std::vector<uint8_t> out;
        serialize(MsgType::WIN, &r, sizeof(r), out);
        send_all(player_socket[0], out);
        send_all(player_socket[1], out);
        break;
      }

      // Draw
      if (g.isDraw()) {
        std::vector<uint8_t> out;
        serialize(MsgType::DRAW, nullptr, 0, out);
        send_all(player_socket[0], out);
        send_all(player_socket[1], out);
        break;
      }

      /**
       * Valid move, switch and send turn notification
       */
      g.switchPlayer();
      {
        uint8_t p = (g.activePlayer() == Player::P1 ? 1 : 2);
        std::vector<uint8_t> out;
        serialize(MsgType::TURN, &p, sizeof(p), out);
        send_all(player_socket[0], out);
        send_all(player_socket[1], out);
      }

    } else if (type == MsgType::QUIT_REQUEST) {
      cout << "QUIT REQUEST RECEIVED" << endl;
      handle_quit(0);
    } else {
      std::string m = "Unexpected message type";
      std::vector<uint8_t> out;
      serialize(MsgType::ERROR, m.data(), m.size(), out);
      send_all(sockfd, out);
    }
  } // main loop

  /**
   * Cleanup after game: close client socket and clear
   * socket slot
   */
  close(sockfd);
}

// Main method
int main() {
  // Install signal handlers
  signal(SIGINT, handle_quit);
  signal(SIGTERM, handle_quit);

  // Very common members from std namespace
  using std::cout, std::endl, std::cerr;
  using std::string;

  int newsockfd, portno = 8080;
  string address = "127.0.0.1";
  socklen_t cliLen;
  struct sockaddr_in serv_addr, cli_addr;

  /**
   * Open socket
   */
  if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    fatal_error(1, "Error opening socket");
  int opt = 1;
  setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  /**
   * Set address and port number to given values (fixed to localhost:8080 in
   * version 1.0)
   */
  serv_addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
    fatal_error(1, "Invalid or unsupported address");
  serv_addr.sin_port = htons(portno);

  /**
   * Bind the host address
   */
  if (bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    fatal_error(1, "Error on binding");

  /**
   * Begin listening for clients, this process will sleep and
   * await incoming connections.
   */
  if (listen(serv_fd, 5) < 0)
    fatal_error(1, "Error on listen");
  cliLen = sizeof(cli_addr);

  // Loop to accept incoming connections
  while (true) {
    if ((newsockfd = accept(serv_fd, (struct sockaddr *)&cli_addr, &cliLen)) <
        0)
      fatal_error(1, "Error on accept");

    // Ignore new connection if two clients are already connected
    if (current_connections >= 2) {
      std::cerr << "Connection refused: 2 clients are already connected"
                << std::endl;
      close(newsockfd);
      continue;
    }

    // Assign player id
    int player_id = 0;
    {
      std::lock_guard<std::mutex> lock(game_mutex);
      if (player_socket[0] == -1) { // Player 1 joins
        player_socket[0] = newsockfd;
        player_id = 1;
      } else if (player_socket[1] == -1) { // Player 2 joins
        player_socket[1] = newsockfd;
        player_id = 2;
      } else { // Race condition fallback
        close(newsockfd);
        continue;
      }
    }

    current_connections.fetch_add(1);

    std::thread([newsockfd, player_id]() {
      handle_client(newsockfd, player_id);
      current_connections.fetch_sub(1);
    }).detach();

  } // Incoming connection loop

  // Close socket when done
  close(serv_fd);
  return 0;
}