#include "protocol.hpp"
#include <cstring>

namespace TTT_PROTO {

int serialize(MsgType type, const void *payload, size_t size,
              std::vector<uint8_t> &out) {

  using enum ProtoErr;

  // Validate size is no larger than 255 bytes
  if (size > 255)
    return (int)INVALID_SIZE;

  // Ensure valid payload
  if (payload == nullptr && size > 0)
    return (int)NULL_PAYLOAD;

  // Clear output + reserve space for contents
  out.clear();
  out.reserve(sizeof(MsgHeader) + size);

  // Build header
  MsgHeader h;
  h.type = static_cast<uint8_t>(type);
  h.size = static_cast<uint8_t>(size);

  // Write header bytes
  out.insert(out.end(), reinterpret_cast<uint8_t *>(&h),
             reinterpret_cast<uint8_t *>(&h) + sizeof(h));

  // Write payload bytes
  if (size > 0) {
    out.insert(out.end(), reinterpret_cast<const uint8_t *>(payload),
               reinterpret_cast<const uint8_t *>(payload) + size);
  }

  // Sucessful write, return 0
  return (int)OK;
}

int deserialize(const std::vector<uint8_t> &bytes, MsgHeader &header_r,
                std::vector<uint8_t> &payload_r) {

  using enum ProtoErr;

  // Ensure bytes is the proper size
  if (bytes.size() < sizeof(MsgHeader))
    return (int)BUFFER_TOO_SMALL;

  // Copy header
  std::memcpy(&header_r, bytes.data(), sizeof(MsgHeader));

  // Ensure their isn't a size mismatch
  if (bytes.size() < sizeof(MsgHeader) + header_r.size)
    return (int)SIZE_MISMATCH;

  // Extract payload
  payload_r.assign(bytes.begin() + sizeof(MsgHeader),
                   bytes.begin() + sizeof(MsgHeader) + header_r.size);

  // Successful write, return 0
  return (int)OK;
}

} // namespace TTT_PROTO