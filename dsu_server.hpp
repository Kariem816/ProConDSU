#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "common/types.hpp"
#include "dsu_client.hpp"
#include "udp_server.hpp"

class DsuServer : public UdpServer {
public:
  DsuServer(const std::string &address = "127.0.0.1", uint16_t port = 26760);
  ~DsuServer();

private:
  ByteBuffer handleMessage(const ByteBuffer &buf, Connection conn);

  std::map<uint32_t, DsuClient> clients;
  std::mutex clientsMutex;

  uint32_t serverId;

  std::jthread updateThread;
};