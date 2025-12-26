#pragma once
#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstdint>
#include <vector>

// Protocol namespace containing serialization + deserialization members +
// functions
namespace TTT_PROTO {

/**
 * MsgType enum, used to give serialized vectors a "header" byte,
 * indicating what type of struct they were serialized from
 */

enum class MsgType : uint8_t {
  WELCOME = 1,
  SERVER_FULL,
  BOARD_UPDATE,
  TURN,
  MOVE_RESULT,
  WIN,
  DRAW,
  ERROR,
  MOVE_REQUEST = 100,
  QUIT_REQUEST
};

enum class ProtoErr : int {
  OK = 0,
  INVALID_TYPE,
  INVALID_SIZE,
  NULL_PAYLOAD,
  BUFFER_TOO_SMALL,
  SIZE_MISMATCH
};

struct MsgHeader {
  uint8_t type;
  uint8_t size;
};

struct PL_Welcome {
  uint8_t p_id;
};
struct PL_Board {
  uint8_t cells[9];
};
struct PL_MovReq {
  uint8_t pos;
};
struct PL_MovRes {
  uint8_t status;
};

/**
 * Serialize + deserialize functions
 */

// Serialize a payload into an array of bytes. Return 0 if successful
int serialize(MsgType type, const void *payload, size_t size,
              std::vector<uint8_t> &out);
// Deserialize a byte array into a header + payload. Return 0 if successful
int deserialize(const std::vector<uint8_t> &bytes, MsgHeader &header_r,
                std::vector<uint8_t> &payload_r);
} // namespace TTT_PROTO

#endif