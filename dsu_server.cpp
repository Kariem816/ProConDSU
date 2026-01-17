#include "dsu_server.hpp"

#include <cstring>
#include <iostream>

DsuServer::DsuServer(const std::string &address, uint16_t port)
    : UdpServer(address, port) {
  setMessageHandler(std::bind_front(&DsuServer::handleMessage, this));
}

DsuServer::~DsuServer() {}

ByteBuffer DsuServer::handleMessage(const ByteBuffer &buf, Connection conn) {
  auto n = buf.size();
  if (n < 20) {
    return {};
  }
  if (std::strncmp((const char *)buf.data(), "DSUC", 4) != 0) {
    return {};
  }

  std::string message = "Connected to ProconDSU Server";
  ByteBuffer response;
  response.resize(message.size());
  std::memcpy(response.data(), message.data(), message.size());
  return response;
}