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
  updateThread = std::jthread([this](std::stop_token stoken) {
    while (!stoken.stop_requested()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });
}

DsuServer::~DsuServer() {
  updateThread.request_stop();
  if (updateThread.joinable()) {
    updateThread.join();
  }
}

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
  ByteBuffer body;

  switch (req.type) {
  case MessageType::ProtocolVersionMessage: {
    ProtocolVersionResponse pvrs;
    pvrs.version = 1001;
    body = pvrs.serialize();
  } break;
  case MessageType::ControllersInfoMessage: {
    ControllersInfoRequest cirq;
    err = cirq.deserialize(req.body);
    if (err != DeserializeError::None) {
      std::println("ControllersInfoMessage :: Deserialize error :: {}",
                   static_cast<uint8_t>(err));
      return {};
    }

    ControllersInfoResponse cirs;
    body = cirs.serialize();
  } break;
  case MessageType::ControllersDataMessage: {
    ControllersDataRequest cdrq;
    err = cdrq.deserialize(req.body);
    if (err != DeserializeError::None) {
      std::println("ControllersDataMessage :: Deserialize error :: {}",
                   static_cast<uint8_t>(err));
      return {};
    }

    ControllersDataResponse cdrs;
    body = cdrs.serialize();
  } break;
  case MessageType::ControllersMotorsInfoMessage: {
    ControllersMotorsRequest cmim;
    err = cmim.deserialize(req.body);
    if (err != DeserializeError::None) {
      std::println("ControllersMotorsInfoMessage :: Deserialize error :: {}",
                   static_cast<uint8_t>(err));
      return {};
    }
    ControllersMotorsResponse cmirs;
    body = cmirs.serialize();
  } break;
  case MessageType::ControllersMotorsRumbleMessage: {
    ControllersMotorsRumbleRequest cmrq;
    err = cmrq.deserialize(req.body);
    if (err != DeserializeError::None) {
      std::println("ControllersMotorsRumbleMessage :: Deserialize error :: {}",
                   static_cast<uint8_t>(err));
      return {};
    }
  } break;
  default:
    return {};
  }

  PacketHeader header;
  header.magic[0] = 'D';
  header.magic[1] = 'S';
  header.magic[2] = 'U';
  header.magic[3] = 'S';
  header.protocol = 1001;
  header.length = static_cast<uint16_t>(sizeof(MessageType) + body.size());
  header.crc32 = 0; // will be filled later
  header.clientServerID = serverId;

  Packet resp;
  resp.header = header;
  resp.type = req.type;
  resp.body = body;
  return resp.serialize();
}