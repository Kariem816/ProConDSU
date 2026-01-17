#pragma once

#include <cstdint>
#include <vector>

// https://github.com/v1993/cemuhook-protocol

using byte = uint8_t;

struct PacketHeader {
  byte magic[4]; // Magic string — DSUS if it's message by server (you), DSUC
                 // if by client (cemuhook).
  uint16_t protocol; // Protocol version used in message. Currently 1001.
  uint16_t length;   // Length of packet without header. Drop packet if it's too
                     // short, truncate if it's too long.
  uint32_t crc32; // CRC32 of whole packet while this field was zeroed out. Be
                  // careful with endianness here!
  uint32_t clientServerID; // Client or server ID who sent this packet. Should
                           // stay the same on one run. Can be randomly
                           // generated on startup.
};

enum class MessageType : uint32_t {
  ProtocolVersionMessage = 0x100000, // Protocol version information (doesn't
                                     // seem to be ever requested)
  ControllersInfoMessage = 0x100001, // Information about connected controllers
  ControllersDataMessage = 0x100002, // Actual controllers data
  ControllersMotorsInfoMessage =
      0x110001, // (Unofficial) Information about controller motors
  ControllersMotorsRumbleMessage =
      0x110002 // (Unofficial) Rumble controller motor
};

struct Packet {
  PacketHeader header;
  MessageType type; // Event type. Read below to learn possible ones.
  std::vector<uint8_t> body;
};

struct ProtocolVersionRequest {};
struct ProtocolVersionResponse {
  uint16_t version; // Maximal protocol version supported by your application.
};

enum class BatteryStatus : uint8_t {
  BatteryNotApplicable = 0x00,
  BatteryDying = 0x01,
  BatteryLow = 0x02,
  BatteryMedium = 0x03,
  BatteryHigh = 0x04,
  BatteryFull = 0x05,
  BatteryCharging = 0xEE,
  BatteryCharged = 0xEF
};

enum class ControllerState : uint8_t {
  ControllerDisconnected = 0x00,
  ControllerReserved = 0x01,
  ControllerConnected = 0x02
};

enum class DeviceModel : uint8_t {
  DeviceModelNotApplicable = 0x00,
  DeviceModelNoGyro = 0x01,
  DeviceModelFullGyro = 0x02
};

enum class ConnectionType : uint8_t {
  ConnectionTypeNotApplicable = 0x00,
  ConnectionTypeUSB = 0x01,
  ConnectionTypeBluetooth = 0x02
};

struct ControllerInfoShared {
  uint8_t slot; // Slot you're reporting about. Must be the same as byte value
                // you read.
  ControllerState state; // Slot state: 0 if not connected, 1 if reserved (?), 2
                         // if connected.
  DeviceModel model; // Device model: 0 if not applicable, 1 if no or partial
                     // gyro 2 for full gyro. Value 3 exist but should not be
                     // used (go with VR, guys).
  ConnectionType connection; // Connection type: 0 if not applicable, 1 for USB,
                             // 2 for bluetooth.
  byte macAddress[6]; // MAC address of device. It's used to detect same device
                      // between launches. Zero out if not applicable.
  BatteryStatus batteryState; // Battery status. See below for possible values.
};

struct ControllersInfoRequest {
  int32_t ports; // Amount of ports you should report about. Always less than 5.
  std::vector<byte> slots; // Each byte represent number of slot you should
                           // report about. Count of bytes here is determined by
                           // value above. Each value is less than 4.
};
struct ControllersInfoResponse {
  ControllerInfoShared info;
  byte _; // Zero byte (\0).
};

enum class ControllerIdType : uint8_t {
  ControllerIdTypeSlot = 1,
  ControllerIdTypeMAC = 2
};

struct ControllerIdentifier {
  ControllerIdType type; // Bitmask of actions you should take. Valid flags are
                         // 1 for slot-based registration, 2 for MAC-based
                         // registration, no bits (all set to 0) to subscribe to
                         // all controllers.
  uint8_t slot;
  byte mac[6]; // If MAC-based registration is requested, MAC of device to
               // report about.
};

struct ControllersDataRequest {
  ControllerIdentifier controllerId;
};

using GamePadButton = uint8_t;

// D-Pad Left, D-Pad Down, D-Pad Right, D-Pad Up, Options (?), R3, L3, Share
// (?)
constexpr GamePadButton ButtonDPadLeft = 1 << 7;
constexpr GamePadButton ButtonDPadDown = 1 << 6;
constexpr GamePadButton ButtonDPadRight = 1 << 5;
constexpr GamePadButton ButtonDPadUp = 1 << 4;
constexpr GamePadButton ButtonOptions = 1 << 3;
constexpr GamePadButton ButtonR3 = 1 << 2;
constexpr GamePadButton ButtonL3 = 1 << 1;
constexpr GamePadButton ButtonShare = 1 << 0;

// Y, B, A, X, R1, L1, R2, L2
constexpr GamePadButton ButtonY = 1 << 7;
constexpr GamePadButton ButtonB = 1 << 6;
constexpr GamePadButton ButtonA = 1 << 5;
constexpr GamePadButton ButtonX = 1 << 4;
constexpr GamePadButton ButtonR1 = 1 << 3;
constexpr GamePadButton ButtonL1 = 1 << 2;
constexpr GamePadButton ButtonR2 = 1 << 1;
constexpr GamePadButton ButtonL2 = 1 << 0;

struct GamePadButtons {
  GamePadButton buttons1; // Bitmask D-Pad Left, D-Pad Down, D-Pad Right, D-Pad
                          // Up, Options (?), R3, L3, Share (?)
  GamePadButton buttons2; // Bitmask Y, B, A, X, R1, L1, R2, L2
};

struct Touch {
  bool active; // Is touch active (1 if active, else 0)
  uint8_t id;  // Touch id (should be the same for one continuous touch)
  uint16_t x;  // Touch X position
  uint16_t y;  // Touch Y position
};

struct Vectors3f {
  float x;
  float y;
  float z;
};

struct ControllersDataResponse {
  ControllerInfoShared info;
  bool connected;     // Is controller connected (1 if connected, 0 if not)
  uint32_t packetNum; // Packet number (for this client)
  GamePadButtons buttons;
  bool home;          // HOME Button (0 or 1)
  bool touchButton;   // Touch Button (0 or 1)
  uint8_t lStickX;    // Left stick X (plus rightward)
  uint8_t lStickY;    // Left stick Y (plus upward)
  uint8_t rStickX;    // Right stick X (plus rightward)
  uint8_t rStickY;    // Right stick Y (plus upward)
  uint8_t aDPadL;     // Analog D-Pad Left
  uint8_t aDPadD;     // Analog D-Pad Down
  uint8_t aDPadR;     // Analog D-Pad Right
  uint8_t aDPadU;     // Analog D-Pad Up
  uint8_t aY;         // Analog Y
  uint8_t aB;         // Analog B
  uint8_t aA;         // Analog A
  uint8_t aX;         // Analog X
  uint8_t aR1;        // Analog R1
  uint8_t aL1;        // Analog L1
  uint8_t aR2;        // Analog R2
  uint8_t aL2;        // Analog L2
  Touch touch1;       // First touch (see below)
  Touch touch2;       // Second touch (see below)
  uint64_t timestamp; // Motion data timestamp in microseconds, update only
                      // with accelerometer (but not gyro only) changes
  Vectors3f accel;    // Accelerometer data in Gs
  Vectors3f gyro;     // Gyroscope data in degrees per second
};

struct ControllersMotorsRequest {
  ControllerIdentifier controllerId;
};
struct ControllersMotorsResponse {
  ControllerInfoShared info;
  uint8_t motorCount; // Motor count - common values are 0 (no rumble
                      // support), 1 (single motor), and 2 (left/right motors)
};

struct ControllersMotorsRumbleRequest {
  ControllerIdentifier controllerId;
  uint8_t motorID;   // Motor id, 0~motor count-1
  uint8_t intensity; // Motor vibration intensity, 0~255 (0 means no vibration)
};
