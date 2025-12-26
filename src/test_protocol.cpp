#include "protocol.hpp"
#include <cassert>
#include <cstring>
#include <iostream>

/**
 * TEST: Serialize + deserialize a Welcome payload
 */
void test_welcome() {
  using namespace TTT_PROTO;

  PL_Welcome w{1};
  std::vector<uint8_t> bytes;

  assert(serialize(MsgType::WELCOME, &w, sizeof(w), bytes) == 0);

  MsgHeader h;
  std::vector<uint8_t> payload;
  assert(deserialize(bytes, h, payload) == 0);

  assert(h.type == (uint8_t)MsgType::WELCOME);
  assert(h.size == sizeof(PL_Welcome));
  assert(payload.size() == sizeof(PL_Welcome));

  PL_Welcome w_out;
  std::memcpy(&w_out, payload.data(), sizeof(w_out));
  assert(w_out.p_id == 1);
}

int main() {
  test_welcome();
  std::cout << "All tests passed!" << std::endl;

  return 0;
}