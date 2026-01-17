#pragma once

#include "packet.hpp"

#include <format>
#include <string_view>

template <>
struct std::formatter<PacketHeader> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const PacketHeader &h, std::format_context &ctx) const {
    return std::format_to(
        ctx.out(),
        "PacketHeader{{Version: {}, Length: {}, CRC32: 0x{:x}, "
        "ClientServerID: 0x{:x}}}",
        h.protocol, h.length, h.crc32, h.clientServerID);
  }
};

template <>
struct std::formatter<MessageType> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const MessageType &t, std::format_context &ctx) const {
    switch (t) {
    case MessageType::ProtocolVersionMessage:
      return std::format_to(ctx.out(), "Protocol Version");
    case MessageType::ControllersInfoMessage:
      return std::format_to(ctx.out(), "Controllers Info");
    case MessageType::ControllersDataMessage:
      return std::format_to(ctx.out(), "Controllers Data");
    case MessageType::ControllersMotorsInfoMessage:
      return std::format_to(ctx.out(), "Controllers Motors Info");
    case MessageType::ControllersMotorsRumbleMessage:
      return std::format_to(ctx.out(), "Controllers Motors Rumble");
    default:
      return std::format_to(ctx.out(), "Unknown (0x{:x})",
                            static_cast<uint32_t>(t));
    }
  }
};

template <> struct std::formatter<Packet> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Packet &p, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "Packet{{Header: {}, Type: {}, Body: [{} Bytes]}}",
                          p.header, p.type, p.body.size());
  }
};

template <>
struct std::formatter<ProtocolVersionRequest>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ProtocolVersionRequest &pvrq,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "ProtocolVersionRequest{{}}");
  }
};

template <>
struct std::formatter<ProtocolVersionResponse>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ProtocolVersionResponse &pvrs,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "ProtocolVersionResponse{{Version: {}}}",
                          pvrs.version);
  }
};

template <>
struct std::formatter<BatteryStatus> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const BatteryStatus &b, std::format_context &ctx) const {
    switch (b) {
    case BatteryStatus::BatteryNotApplicable:
      return std::format_to(ctx.out(), "Battery Not Applicable");
    case BatteryStatus::BatteryDying:
      return std::format_to(ctx.out(), "Battery Dying");
    case BatteryStatus::BatteryLow:
      return std::format_to(ctx.out(), "Battery Low");
    case BatteryStatus::BatteryMedium:
      return std::format_to(ctx.out(), "Battery Medium");
    case BatteryStatus::BatteryHigh:
      return std::format_to(ctx.out(), "Battery High");
    case BatteryStatus::BatteryFull:
      return std::format_to(ctx.out(), "Battery Full");
    case BatteryStatus::BatteryCharging:
      return std::format_to(ctx.out(), "Battery Charging");
    case BatteryStatus::BatteryCharged:
      return std::format_to(ctx.out(), "Battery Charged");
    default:
      return std::format_to(ctx.out(), "Unknown (0x{:x})",
                            static_cast<uint8_t>(b));
    }
  }
};

template <>
struct std::formatter<ControllerState> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllerState &s, std::format_context &ctx) const {
    switch (s) {
    case ControllerState::ControllerDisconnected:
      return std::format_to(ctx.out(), "Controller Disconnected");
    case ControllerState::ControllerReserved:
      return std::format_to(ctx.out(), "Controller Reserved");
    case ControllerState::ControllerConnected:
      return std::format_to(ctx.out(), "Controller Connected");
    default:
      return std::format_to(ctx.out(), "Unknown (0x{:x})",
                            static_cast<uint8_t>(s));
    }
  }
};

template <>
struct std::formatter<DeviceModel> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const DeviceModel &m, std::format_context &ctx) const {
    switch (m) {
    case DeviceModel::DeviceModelNotApplicable:
      return std::format_to(ctx.out(), "Device Model Not Applicable");
    case DeviceModel::DeviceModelNoGyro:
      return std::format_to(ctx.out(), "Device Model No Gyro");
    case DeviceModel::DeviceModelFullGyro:
      return std::format_to(ctx.out(), "Device Model Full Gyro");
    default:
      return std::format_to(ctx.out(), "Unknown (0x{:x})",
                            static_cast<uint8_t>(m));
    }
  }
};

template <>
struct std::formatter<ConnectionType> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ConnectionType &c, std::format_context &ctx) const {
    switch (c) {
    case ConnectionType::ConnectionTypeNotApplicable:
      return std::format_to(ctx.out(), "Connection Type Not Applicable");
    case ConnectionType::ConnectionTypeUSB:
      return std::format_to(ctx.out(), "Connection Type USB");
    case ConnectionType::ConnectionTypeBluetooth:
      return std::format_to(ctx.out(), "Connection Type Bluetooth");
    default:
      return std::format_to(ctx.out(), "Unknown (0x{:x})",
                            static_cast<uint8_t>(c));
    }
  }
};

template <>
struct std::formatter<ControllerInfoShared> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllerInfoShared &cis, std::format_context &ctx) const {
    return std::format_to(
        ctx.out(),
        "ControllerInfoShared{{Slot: {}, State: {}, Device Model: {}, "
        "Connection Type: {}, MAC Address: {}, Battery Status: {}}}",
        cis.slot, cis.state, cis.model, cis.connection, cis.macAddress,
        cis.batteryState);
  }
};

template <>
struct std::formatter<ControllersInfoRequest>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersInfoRequest &cirq,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "ControllersInfoRequest{{Ports: {}, Slots: {}}}",
                          cirq.ports, cirq.slots);
  }
};

template <>
struct std::formatter<ControllerInfoResponse>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllerInfoResponse &cir,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(), "ControllerInfoResponse{{{}}}", cir.info);
  }
};

template <>
struct std::formatter<ControllersInfoResponse>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersInfoResponse &cirs,
              std::format_context &ctx) const {
    std::string joinedInfo;
    for (size_t i = 0; i < cirs.info.size(); ++i) {
      joinedInfo += std::format("{}", cirs.info[i]);
      if (i < cirs.info.size() - 1) {
        joinedInfo += ", ";
      }
    }
    return std::format_to(ctx.out(), "ControllersInfoResponse{{{}}}",
                          joinedInfo);
  }
};

template <>
struct std::formatter<ControllerIdType> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllerIdType &cidt, std::format_context &ctx) const {
    auto slot = (cidt & ControllerIdTypeSlot) != 0;
    auto mac = (cidt & ControllerIdTypeMAC) != 0;

    if (slot && mac) {
      return std::format_to(ctx.out(), "Slot and MAC");
    }

    if (slot) {
      return std::format_to(ctx.out(), "Slot");
    }

    if (mac) {
      return std::format_to(ctx.out(), "MAC");
    }

    return std::format_to(ctx.out(), "Unknown (0x{:x})",
                          static_cast<uint8_t>(cidt));
  }
};

template <>
struct std::formatter<ControllerIdentifier> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllerIdentifier &cid, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "ControllerIdentifier{{Type: {}, Slot: "
                          "{}, MAC Address: {}}}",
                          cid.type, cid.slot, cid.mac);
  }
};

template <>
struct std::formatter<ControllersDataRequest>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersDataRequest &cdrq,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "ControllersDataRequest{{ControllerId: {}}}",
                          cdrq.controllerId);
  }
};

template <> struct std::formatter<Touch> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Touch &t, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "Touch{{Active: {}, ID: {}, X: {}, Y: {}}}", t.active,
                          t.id, t.x, t.y);
  }
};

template <>
struct std::formatter<GamePadButtons> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const GamePadButtons &bs, std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "GamePadButtons{{Buttons1: 0b{:b}, "
                          "Buttons2: 0b{:b}}}",
                          bs.buttons1, bs.buttons2);
  }
};

template <>
struct std::formatter<Vectors3f> : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Vectors3f &v, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "Vectors3f{{X: {}, Y: {}, Z: {}}}", v.x,
                          v.y, v.z);
  }
};

template <>
struct std::formatter<ControllersDataResponse>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersDataResponse &cdrs,
              std::format_context &ctx) const {
    return std::format_to(
        ctx.out(),
        "ControllersDataResponse{{Info: {}, Connected: {}, PacketNum: {}, "
        "Buttons: {}, Home: {}, TouchButton: {}, LStickX: {}, LStickY: {}, "
        "RStickX: {}, RStickY: {}, ADPadL: {}, ADPadD: {}, ADPadR: {}, "
        "ADPadU: {}, AY: {}, AB: {}, AA: {}, AX: {}, AR1: {}, AL1: {}, AR2: "
        "{}, AL2: {}, Touch1: {}, Touch2: {}, Timestamp: {}, Acc: {}, Gyro: "
        "{}}}",
        cdrs.info, cdrs.connected, cdrs.packetNum, cdrs.buttons, cdrs.home,
        cdrs.touchButton, cdrs.lStickX, cdrs.lStickY, cdrs.rStickX,
        cdrs.rStickY, cdrs.aDPadL, cdrs.aDPadD, cdrs.aDPadR, cdrs.aDPadU,
        cdrs.aY, cdrs.aB, cdrs.aA, cdrs.aX, cdrs.aR1, cdrs.aL1, cdrs.aR2,
        cdrs.aL2, cdrs.touch1, cdrs.touch2, cdrs.timestamp, cdrs.accel,
        cdrs.gyro);
  }
};

template <>
struct std::formatter<ControllersMotorsRequest>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersMotorsRequest &cmrq,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "ControllersMotorsRequest{{ControllerId: {}}}",
                          cmrq.controllerId);
  }
};

template <>
struct std::formatter<ControllersMotorsResponse>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersMotorsResponse &cmrs,
              std::format_context &ctx) const {
    return std::format_to(ctx.out(),
                          "ControllersMotorsResponse{{Info: {}, MotorCount: "
                          "{}}}",
                          cmrs.info, cmrs.motorCount);
  }
};

template <>
struct std::formatter<ControllersMotorsRumbleRequest>
    : std::formatter<std::string_view> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const ControllersMotorsRumbleRequest &cmrrq,
              std::format_context &ctx) const {
    return std::format_to(
        ctx.out(),
        "ControllersMotorsRumbleRequest{{ControllerId: {}, MotorID: {}, "
        "Intensity: {}}}",
        cmrrq.controllerId, cmrrq.motorID, cmrrq.intensity);
  }
};
