#include "utils.hpp"

#include <cstring>
#include <format>

template <> struct std::formatter<DeserializeError> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const DeserializeError &e, std::format_context &ctx) const {
    switch (e) {
    case DeserializeError::None:
      return std::format_to(ctx.out(), "No Error");
    case DeserializeError::ErrInvalidPacket:
      return std::format_to(ctx.out(), "Invalid Packet");
    case DeserializeError::ErrInvalidLength:
      return std::format_to(ctx.out(), "Invalid Length");
    case DeserializeError::ErrParseError:
      return std::format_to(ctx.out(), "Parse Error");
    default:
      return std::format_to(ctx.out(), "Unknown Packet Parse Error (0x{:x})",
                            static_cast<uint8_t>(e));
    }
  }
};

DeserializeError isValidMessage(const ByteBuffer &buf) {
  if (buf.size() < 20) {
    return DeserializeError::ErrInvalidLength;
  }
  if (std::strncmp((char *)buf.data(), "DSUC", 4) != 0) {
    return DeserializeError::ErrInvalidPacket;
  }

  // TODO: verify checksum

  return DeserializeError::None;
}

// Simple CRC32 implementation (IEEE polynomial)
void init_crc32_table() {
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
    crc32_table[i] = crc;
  }
}

uint32_t compute_crc32(const uint8_t *data, size_t len) {
  static bool initialized = false;
  if (!initialized) {
    init_crc32_table();
    initialized = true;
  }

  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < len; i++) {
    crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFF;
}