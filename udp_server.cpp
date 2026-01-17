#include "udp_server.hpp"

#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

UdpServer::UdpServer(const std::string &address, uint16_t port)
    : msgHandler(defaultMessageHandler) {
  std::cout << "Initialising Winsock..." << std::endl;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    throw std::runtime_error("WSAStartup failed: " +
                             std::to_string(WSAGetLastError()));
  }
  std::cout << "Initialised." << std::endl;

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
    throw std::runtime_error("Could not create socket: " +
                             std::to_string(WSAGetLastError()));
  }
  std::cout << "Socket created." << std::endl;

  u_long mode = 1; // 1 for non-blocking, 0 for blocking
  if (ioctlsocket(s, FIONBIO, &mode) != 0) {
    closesocket(s);
    WSACleanup();
    throw std::runtime_error("ioctlsocket failed: " +
                             std::to_string(WSAGetLastError()));
  }

  server.sin_family = AF_INET;
  inet_pton(AF_INET, address.c_str(), &server.sin_addr);
  server.sin_port = htons(port);

  if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
    closesocket(s);
    WSACleanup();
    throw std::runtime_error("Bind failed with error code : " +
                             std::to_string(WSAGetLastError()));
  }
  std::cout << "Bind done on port " << port << std::endl;
}

UdpServer::~UdpServer() {
  closesocket(s);
  WSACleanup();
}

void UdpServer::start() {
  listenThread = std::jthread(std::bind_front(&UdpServer::listen, this));
}

void UdpServer::wait() {
  if (listenThread.joinable()) {
    listenThread.join();
  }
}

void UdpServer::stop() { listenThread.request_stop(); }

void UdpServer::setMessageHandler(MsgHandler _handler) {
  msgHandler = std::move(_handler);
}

void UdpServer::listen(std::stop_token stoken) {
  struct sockaddr_in si_other;
  int slen = sizeof(si_other);
  char recv_buf[512];

  while (!stoken.stop_requested()) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);

    timeval tv;
    tv.tv_sec = 1; // 1 second timeout
    tv.tv_usec = 0;

    int select_result = select(0, &readfds, nullptr, nullptr, &tv);
    if (select_result == SOCKET_ERROR) {
      std::cerr << "select failed with error code : " << WSAGetLastError()
                << std::endl;
      break;
    }

    if (select_result == 0) { // timeout
      continue;
    }

    // Data is available
    std::memset(recv_buf, '\0', 512);
    int recv_len =
        recvfrom(s, recv_buf, 512, 0, (struct sockaddr *)&si_other, &slen);
    if (recv_len == SOCKET_ERROR) {
      std::cerr << "recvfrom failed with error code : " << WSAGetLastError()
                << std::endl;
      break;
    }

    std::string clientIp;
    clientIp.resize(INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(si_other.sin_addr), clientIp.data(), INET_ADDRSTRLEN);
    clientIp.resize(clientIp.c_str() + strlen(clientIp.c_str()) -
                    clientIp.data());

    Connection conn(si_other);
    std::vector<uint8_t> buf;
    buf.resize(recv_len);
    std::memcpy(buf.data(), recv_buf, recv_len);

    auto retBuffer = msgHandler(buf, conn);
    send(retBuffer, conn);
  }
}

ByteBuffer UdpServer::defaultMessageHandler(const ByteBuffer &buf,
                                            Connection conn) {
  std::cout << conn.ip() << ":" << conn.port() << " => ";
  for (uint8_t byte : buf) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
              << (int)byte << " ";
  }
  std::cout << std::dec << std::endl;
  return buf;
}

void UdpServer::send(const ByteBuffer &buf, Connection conn) {
  if (buf.size() > 0) {
    sendto(s, (const char *)buf.data(), buf.size(), 0,
           (struct sockaddr *)&conn.addr, sizeof(conn.addr));
  }
}