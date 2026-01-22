#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "common/types.hpp"
#include "controller_manager.hpp"
#include "dsu_client.hpp"
#include "udp_server.hpp"

class DsuServer : public UdpServer {
public:
  DsuServer(const std::string &address = "127.0.0.1", uint16_t port = 26760);
  ~DsuServer();

private:
  ByteBuffer handleMessage(const ByteBuffer &buf, Connection conn);

  // Helper to convert ProController input to DSU format
  ControllersDataResponse buildControllerDataResponse(size_t controller_index);
  ControllersInfoResponse buildControllersInfoResponse();

  ControllerManager controllerManager;

  uint32_t serverId;
  uint32_t packetCounter;

  std::jthread updateThread;
};
