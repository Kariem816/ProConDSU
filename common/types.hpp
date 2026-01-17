#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

using byte = uint8_t;
using ByteBuffer = std::vector<uint8_t>;

struct Connection {
  struct sockaddr_in addr;
  Connection() = default;
  Connection(struct sockaddr_in _addr) : addr(_addr) {}
  std::string ip() const {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipStr, INET_ADDRSTRLEN);
    return std::string(ipStr);
  }
  uint16_t port() const { return ntohs(addr.sin_port); }
};
