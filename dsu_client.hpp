#pragma once

#include "common/types.hpp"
#include "packet/packet.hpp"

struct DsuClient {
  Connection conn;
  Packet req;
  uint32_t packetCounter;
  std::chrono::steady_clock::time_point lastRequest;
};
