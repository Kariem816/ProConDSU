#include "dsu_server.hpp"

#include <cstring>
#include <iostream>
#include <print>

#include "packet/formatters.hpp"
#include "packet/packet.hpp"

DsuServer::DsuServer(const std::string &address, uint16_t port)
    : UdpServer(address, port), packetCounter(0) {
  setMessageHandler(std::bind_front(&DsuServer::handleMessage, this));
  serverId = std::rand();

  // Initialize controller manager
  controllerManager.initialize();
  std::println("ControllerManager initialized with {} controller(s)",
               controllerManager.getConnectedControllerCount());

  updateThread = std::jthread([this](std::stop_token stoken) {
    while (!stoken.stop_requested()) {
      controllerManager.update();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });
}

DsuServer::~DsuServer() {
  updateThread.request_stop();
  if (updateThread.joinable()) {
    updateThread.join();
  }
}

ControllersDataResponse
DsuServer::buildControllerDataResponse(size_t controller_index) {
  ControllersDataResponse cdrs;

  if (controller_index >= controllerManager.getConnectedControllerCount()) {
    cdrs.connected = false;
    return cdrs;
  }

  cdrs.connected = true;
  cdrs.info.slot = static_cast<uint8_t>(controller_index);
  cdrs.info.state = ControllerState::ControllerConnected;
  cdrs.info.model = DeviceModel::DeviceModelFullGyro;
  cdrs.info.connection = ConnectionType::ConnectionTypeBluetooth;
  cdrs.info.batteryState = BatteryStatus::BatteryFull;
  cdrs.packetNum = packetCounter++;

  ProControllerHid::InputStatus input_status;
  if (!controllerManager.getControllerInputStatus(controller_index,
                                                  input_status)) {
    cdrs.connected = false;
    return cdrs;
  }

  // Map button states from ProController to DSU format
  cdrs.buttons.buttons1 = 0;
  cdrs.buttons.buttons2 = 0;

  auto &buttons = input_status.Buttons;

  // D-Pad mapping
  if (buttons.LeftButton)
    cdrs.buttons.buttons1 |= ButtonDPadLeft;
  if (buttons.DownButton)
    cdrs.buttons.buttons1 |= ButtonDPadDown;
  if (buttons.RightButton)
    cdrs.buttons.buttons1 |= ButtonDPadRight;
  if (buttons.UpButton)
    cdrs.buttons.buttons1 |= ButtonDPadUp;
  if (buttons.MinusButton)
    cdrs.buttons.buttons1 |= ButtonOptions;
  if (buttons.RStick)
    cdrs.buttons.buttons1 |= ButtonR3;
  if (buttons.LStick)
    cdrs.buttons.buttons1 |= ButtonL3;
  if (buttons.ShareButton)
    cdrs.buttons.buttons1 |= ButtonShare;

  // Action buttons mapping
  if (buttons.YButton)
    cdrs.buttons.buttons2 |= ButtonY;
  if (buttons.BButton)
    cdrs.buttons.buttons2 |= ButtonB;
  if (buttons.AButton)
    cdrs.buttons.buttons2 |= ButtonA;
  if (buttons.XButton)
    cdrs.buttons.buttons2 |= ButtonX;
  if (buttons.RButton)
    cdrs.buttons.buttons2 |= ButtonR1;
  if (buttons.LButton)
    cdrs.buttons.buttons2 |= ButtonL1;
  if (buttons.RZButton)
    cdrs.buttons.buttons2 |= ButtonR2;
  if (buttons.LZButton)
    cdrs.buttons.buttons2 |= ButtonL2;

  // Home button
  cdrs.home = buttons.HomeButton;

  // Stick mapping (convert from float [-1.0, 1.0] to uint8_t [0, 255])
  cdrs.lStickX =
      static_cast<uint8_t>((input_status.LeftStick.X + 1.0f) * 127.5f);
  cdrs.lStickY =
      static_cast<uint8_t>((input_status.LeftStick.Y + 1.0f) * 127.5f);
  cdrs.rStickX =
      static_cast<uint8_t>((input_status.RightStick.X + 1.0f) * 127.5f);
  cdrs.rStickY =
      static_cast<uint8_t>((input_status.RightStick.Y + 1.0f) * 127.5f);

  // Sensor data mapping
  if (input_status.HasSensorStatus) {
    auto &sensor = input_status.Sensors[0];
    cdrs.accel.x = sensor.Accelerometer.X;
    cdrs.accel.y = sensor.Accelerometer.Y;
    cdrs.accel.z = sensor.Accelerometer.Z;
    cdrs.gyro.x = sensor.Gyroscope.X;
    cdrs.gyro.y = sensor.Gyroscope.Y;
    cdrs.gyro.z = sensor.Gyroscope.Z;
    cdrs.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                         input_status.Timestamp.time_since_epoch())
                         .count();
  }

  return cdrs;
}

ControllersInfoResponse DsuServer::buildControllersInfoResponse() {
  ControllersInfoResponse cirs;
  size_t controller_count = controllerManager.getConnectedControllerCount();

  for (size_t i = 0; i < controller_count; ++i) {
    ControllerInfoResponse cir;
    cir.info.slot = static_cast<uint8_t>(i);
    cir.info.state = ControllerState::ControllerConnected;
    cir.info.model = DeviceModel::DeviceModelFullGyro;
    cir.info.connection = ConnectionType::ConnectionTypeBluetooth;
    cir.info.batteryState = BatteryStatus::BatteryFull;
    cirs.info.push_back(cir);
  }

  return cirs;
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

  // std::println("{}", req);
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

    ControllersInfoResponse cirs = buildControllersInfoResponse();
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

    ControllersDataResponse cdrs =
        buildControllerDataResponse(cdrq.controllerId.slot);
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
    cmirs.info.slot = cmim.controllerId.slot;
    cmirs.info.state = ControllerState::ControllerConnected;
    cmirs.info.model = DeviceModel::DeviceModelFullGyro;
    cmirs.info.connection = ConnectionType::ConnectionTypeBluetooth;
    cmirs.info.batteryState = BatteryStatus::BatteryFull;
    cmirs.motorCount = 2; // Pro Controller has left and right motors
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
    // TODO: Map rumble intensity to ProController and send
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
