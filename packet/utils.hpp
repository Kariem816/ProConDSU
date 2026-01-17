#pragma once

#include <cstring>
#include <stdexcept>

#include "common/types.hpp"

enum class DeserializeError : uint8_t {
  None = 0,
  ErrInvalidPacket,
  ErrInvalidLength,
  ErrParseError
};

DeserializeError isValidMessage(const ByteBuffer &buf);

struct Serializable {
  virtual ~Serializable() = default;
  virtual ByteBuffer serialize() const = 0;
  virtual DeserializeError deserialize(const ByteBuffer &buf) = 0;
};
#define SERIALIZABLE_IMPL()                                                    \
  ByteBuffer serialize() const override;                                       \
  DeserializeError deserialize(const ByteBuffer &buf) override;

// Binary read/write helpers for little-endian serialization
class BinaryReader {
  const ByteBuffer &buf;
  size_t pos = 0;

public:
  BinaryReader(const ByteBuffer &b) : buf(b) {}

  template <typename T> DeserializeError read(T &value) {
    if (pos + sizeof(T) > buf.size()) {
      return DeserializeError::ErrParseError;
    }
    std::memcpy(&value, buf.data() + pos, sizeof(T));
    pos += sizeof(T);
    return DeserializeError::None;
  }

  DeserializeError readBytes(uint8_t *dest, size_t count) {
    if (pos + count > buf.size()) {
      return DeserializeError::ErrParseError;
    }
    std::memcpy(dest, buf.data() + pos, count);
    pos += count;
    return DeserializeError::None;
  }

  size_t remaining() const { return buf.size() - pos; }
  size_t position() const { return pos; }
};

class BinaryWriter {
  ByteBuffer buf;

public:
  template <typename T> void write(const T &value) {
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&value);
    buf.insert(buf.end(), ptr, ptr + sizeof(T));
  }

  void writeBytes(const uint8_t *src, size_t count) {
    buf.insert(buf.end(), src, src + count);
  }

  ByteBuffer getBuffer() const { return buf; }
};

static uint32_t crc32_table[256];
void init_crc32_table();
uint32_t compute_crc32(const uint8_t *data, size_t len);
