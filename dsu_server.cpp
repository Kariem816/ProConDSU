#include "dsu_server.hpp"

#include <cstring>
#include <iostream>
#include <print>

#include "packet/formatters.hpp"
#include "packet/packet.hpp"


DsuServer::DsuServer(const std::string &address, uint16_t port)
    : UdpServer(address, port) {
  setMessageHandler(std::bind_front(&DsuServer::handleMessage, this));
  serverId = std::rand();
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

  Packet req;
  auto err = req.deserialize(buf);
  if (err != DeserializeError::None) {
    return {};
  }

  std::println("{}", req);

  ProtocolVersionResponse pvrs;
  pvrs.version = 1001;
  auto body = pvrs.serialize();

  PacketHeader header;
  header.magic[0] = 'D';
  header.magic[1] = 'S';
  header.magic[2] = 'U';
  header.magic[3] = 'S';
  header.protocol = 1001;
  header.length = static_cast<uint16_t>(sizeof(MessageType) + body.size());
  header.crc32 = 0;
  header.clientServerID = serverId;

  Packet resp;
  resp.header = header;
  resp.type = MessageType::ProtocolVersionMessage;
  resp.body = body;
  return resp.serialize();
}