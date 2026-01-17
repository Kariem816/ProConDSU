#pragma once

#include <cstdint>
#include <functional>
#include <stop_token>
#include <string>
#include <thread>
#include <winsock2.h>

#include "common/types.hpp"

struct Connection {
  std::string ip;
  uint16_t port;
};

class UdpServer {
  using MsgHandler = std::function<ByteBuffer(const ByteBuffer &, Connection)>;

public:
  UdpServer(const std::string &address = "127.0.0.1", uint16_t port = 26760);
  ~UdpServer();

  void start();
  void wait();
  void stop();

  void setMessageHandler(MsgHandler _handler);
  static ByteBuffer defaultMessageHandler(const ByteBuffer &buf,
                                          Connection conn);

private:
  void listen(std::stop_token token);

private:
  std::jthread listenThread;
  MsgHandler msgHandler;

  WSADATA wsa;
  SOCKET s;
  struct sockaddr_in server;
};