#pragma once

#include <vector>

#include "udp_server.hpp"

class DsuServer : public UdpServer {
public:
  DsuServer(const std::string &address = "127.0.0.1", uint16_t port = 26760);
  ~DsuServer();

private:
  ByteBuffer handleMessage(const ByteBuffer &buf, Connection conn);
  std::vector<Connection> connections;
};