#include "packet.hpp"

#include <cstring>

#include "utils.hpp"

DeserializeError Packet::deserialize(const ByteBuffer &buf) {
  auto err = isValidMessage(buf);
  if (err != DeserializeError::None) {
    return err;
  }

  BinaryReader reader(buf);

  // Read header (20 bytes)
  err = reader.read(header.magic);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(header.protocol);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(header.length);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(header.crc32);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(header.clientServerID);
  if (err != DeserializeError::None)
    return err;

  // Read message type (4 bytes)
  err = reader.read(type);
  if (err != DeserializeError::None)
    return err;

  // Read remaining body
  size_t bodySize = reader.remaining();
  body.clear();
  body.resize(bodySize);
  if (bodySize > 0) {
    err = reader.readBytes(body.data(), bodySize);
    if (err != DeserializeError::None)
      return err;
  }

  return DeserializeError::None;
}

ByteBuffer Packet::serialize() const {
  BinaryWriter writer;

  // Write header
  writer.writeBytes(header.magic, 4);
  writer.write(header.protocol);
  writer.write(header.length);

  // Prepare CRC field (temporarily 0)
  uint32_t crcValue = 0;
  writer.write(crcValue);

  writer.write(header.clientServerID);

  // Write message type
  writer.write(type);

  // Write body
  if (!body.empty()) {
    writer.writeBytes(body.data(), body.size());
  }

  // Calculate and insert CRC32
  ByteBuffer result = writer.getBuffer();
  uint32_t crc = compute_crc32(result.data(), result.size());

  // Write CRC into the buffer at offset 8
  std::memcpy(result.data() + 8, &crc, sizeof(uint32_t));

  return result;
}

DeserializeError PacketHeader::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 20) {
    return DeserializeError::ErrInvalidLength;
  }
  BinaryReader reader(buf);
  auto err = reader.readBytes(magic, 4);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(protocol);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(length);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(crc32);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(clientServerID);
  if (err != DeserializeError::None)
    return err;
  return DeserializeError::None;
}

ByteBuffer PacketHeader::serialize() const {
  BinaryWriter writer;
  writer.writeBytes(magic, 4);
  writer.write(protocol);
  writer.write(length);
  writer.write(crc32);
  writer.write(clientServerID);
  return writer.getBuffer();
}

DeserializeError ProtocolVersionRequest::deserialize(const ByteBuffer &buf) {
  // No body
  // maybe check length?
  return DeserializeError::None;
}

ByteBuffer ProtocolVersionRequest::serialize() const {
  // No body
  return ByteBuffer{};
}

DeserializeError ProtocolVersionResponse::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 2) {
    return DeserializeError::ErrParseError;
  }
  BinaryReader reader(buf);
  return reader.read(version);
}

ByteBuffer ProtocolVersionResponse::serialize() const {
  BinaryWriter writer;
  writer.write(version);
  return writer.getBuffer();
}

DeserializeError ControllerInfoShared::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 11) {
    return DeserializeError::ErrInvalidLength;
  }
  BinaryReader reader(buf);

  auto err = reader.read(slot);
  if (err != DeserializeError::None)
    return err;

  uint8_t stateVal;
  err = reader.read(stateVal);
  if (err != DeserializeError::None)
    return err;
  state = static_cast<ControllerState>(stateVal);

  uint8_t modelVal;
  err = reader.read(modelVal);
  if (err != DeserializeError::None)
    return err;
  model = static_cast<DeviceModel>(modelVal);

  uint8_t connVal;
  err = reader.read(connVal);
  if (err != DeserializeError::None)
    return err;
  connection = static_cast<ConnectionType>(connVal);

  err = reader.readBytes(macAddress, 6);
  if (err != DeserializeError::None)
    return err;

  uint8_t batteryVal;
  err = reader.read(batteryVal);
  if (err != DeserializeError::None)
    return err;
  batteryState = static_cast<BatteryStatus>(batteryVal);

  return DeserializeError::None;
}

ByteBuffer ControllerInfoShared::serialize() const {
  BinaryWriter writer;
  writer.write(slot);
  writer.write(static_cast<uint8_t>(state));
  writer.write(static_cast<uint8_t>(model));
  writer.write(static_cast<uint8_t>(connection));
  writer.writeBytes(macAddress, 6);
  writer.write(static_cast<uint8_t>(batteryState));
  return writer.getBuffer();
}

DeserializeError ControllersInfoRequest::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 4 || buf.size() > 8) {
    return DeserializeError::ErrInvalidLength;
  }

  BinaryReader reader(buf);
  auto err = reader.read(ports);
  if (err != DeserializeError::None)
    return err;

  if (buf.size() < 4 + static_cast<size_t>(ports)) {
    return DeserializeError::ErrParseError;
  }

  slots.clear();
  slots.resize(ports);
  err = reader.readBytes(slots.data(), ports);
  if (err != DeserializeError::None)
    return err;

  return DeserializeError::None;
}

ByteBuffer ControllersInfoRequest::serialize() const {
  BinaryWriter writer;
  writer.write(ports);
  if (!slots.empty()) {
    writer.writeBytes(slots.data(), slots.size());
  }
  return writer.getBuffer();
}

DeserializeError ControllersInfoResponse::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 12) {
    return DeserializeError::ErrInvalidLength;
  }

  // Deserialize the first 11 bytes as ControllerInfoShared
  ByteBuffer infoBuf(buf.begin(), buf.begin() + 11);
  auto err = info.deserialize(infoBuf);
  if (err != DeserializeError::None)
    return err;

  // Skip zero byte at data[11]
  return DeserializeError::None;
}

ByteBuffer ControllersInfoResponse::serialize() const {
  ByteBuffer result = info.serialize();
  result.push_back(_); // Add padding byte
  return result;
}

DeserializeError ControllerIdentifier::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 8) {
    return DeserializeError::ErrInvalidLength;
  }

  BinaryReader reader(buf);
  uint8_t typeVal;
  auto err = reader.read(typeVal);
  if (err != DeserializeError::None)
    return err;
  type = typeVal;

  err = reader.read(slot);
  if (err != DeserializeError::None)
    return err;

  err = reader.readBytes(mac, 6);
  if (err != DeserializeError::None)
    return err;

  return DeserializeError::None;
}

ByteBuffer ControllerIdentifier::serialize() const {
  BinaryWriter writer;
  writer.write(static_cast<uint8_t>(type));
  writer.write(slot);
  writer.writeBytes(mac, 6);
  return writer.getBuffer();
}

DeserializeError ControllersDataRequest::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 8) {
    return DeserializeError::ErrInvalidLength;
  }
  return controllerId.deserialize(buf);
}

ByteBuffer ControllersDataRequest::serialize() const {
  return controllerId.serialize();
}

DeserializeError GamePadButtons::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 2) {
    return DeserializeError::ErrInvalidLength;
  }
  BinaryReader reader(buf);
  auto err = reader.read(buttons1);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(buttons2);
  if (err != DeserializeError::None)
    return err;
  return DeserializeError::None;
}

ByteBuffer GamePadButtons::serialize() const {
  BinaryWriter writer;
  writer.write(buttons1);
  writer.write(buttons2);
  return writer.getBuffer();
}

DeserializeError Touch::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 4) {
    return DeserializeError::ErrInvalidLength;
  }
  BinaryReader reader(buf);

  uint8_t activeVal;
  auto err = reader.read(activeVal);
  if (err != DeserializeError::None)
    return err;
  active = (activeVal != 0);

  err = reader.read(id);
  if (err != DeserializeError::None)
    return err;

  err = reader.read(x);
  if (err != DeserializeError::None)
    return err;

  err = reader.read(y);
  if (err != DeserializeError::None)
    return err;

  return DeserializeError::None;
}

ByteBuffer Touch::serialize() const {
  BinaryWriter writer;
  writer.write(static_cast<uint8_t>(active ? 1 : 0));
  writer.write(id);
  writer.write(x);
  writer.write(y);
  return writer.getBuffer();
}

DeserializeError Vectors3f::deserialize(const ByteBuffer &buf) {
  if (buf.size() < 12) {
    return DeserializeError::ErrInvalidLength;
  }
  BinaryReader reader(buf);
  auto err = reader.read(x);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(y);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(z);
  if (err != DeserializeError::None)
    return err;
  return DeserializeError::None;
}

ByteBuffer Vectors3f::serialize() const {
  BinaryWriter writer;
  writer.write(x);
  writer.write(y);
  writer.write(z);
  return writer.getBuffer();
}

DeserializeError ControllersDataResponse::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 80) {
    return DeserializeError::ErrInvalidLength;
  }

  BinaryReader reader(buf);

  // Deserialize ControllerInfoShared (11 bytes)
  ByteBuffer infoBuf(buf.begin(), buf.begin() + 11);
  auto err = info.deserialize(infoBuf);
  if (err != DeserializeError::None)
    return err;
  reader.readBytes(nullptr, 11); // Skip ahead

  // Read remaining fields
  uint8_t connectedVal;
  err = reader.read(connectedVal);
  if (err != DeserializeError::None)
    return err;
  connected = (connectedVal != 0);

  err = reader.read(packetNum);
  if (err != DeserializeError::None)
    return err;

  // Read button states (2 bytes)
  err = reader.read(buttons.buttons1);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(buttons.buttons2);
  if (err != DeserializeError::None)
    return err;

  uint8_t homeVal;
  err = reader.read(homeVal);
  if (err != DeserializeError::None)
    return err;
  home = (homeVal != 0);

  uint8_t touchButtonVal;
  err = reader.read(touchButtonVal);
  if (err != DeserializeError::None)
    return err;
  touchButton = (touchButtonVal != 0);

  // Analog stick and button values (14 bytes)
  err = reader.read(lStickX);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(lStickY);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(rStickX);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(rStickY);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aDPadL);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aDPadD);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aDPadR);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aDPadU);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aY);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aB);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aA);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aX);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aR1);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aL1);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aR2);
  if (err != DeserializeError::None)
    return err;
  err = reader.read(aL2);
  if (err != DeserializeError::None)
    return err;

  // First touch
  ByteBuffer touch1Buf(buf.begin() + reader.position(),
                       buf.begin() + reader.position() + 4);
  err = touch1.deserialize(touch1Buf);
  if (err != DeserializeError::None)
    return err;
  reader.readBytes(nullptr, 4);

  // Second touch
  ByteBuffer touch2Buf(buf.begin() + reader.position(),
                       buf.begin() + reader.position() + 4);
  err = touch2.deserialize(touch2Buf);
  if (err != DeserializeError::None)
    return err;
  reader.readBytes(nullptr, 4);

  // Timestamp and motion data
  err = reader.read(timestamp);
  if (err != DeserializeError::None)
    return err;

  ByteBuffer accelBuf(buf.begin() + reader.position(),
                      buf.begin() + reader.position() + 12);
  err = accel.deserialize(accelBuf);
  if (err != DeserializeError::None)
    return err;
  reader.readBytes(nullptr, 12);

  ByteBuffer gyroBuf(buf.begin() + reader.position(),
                     buf.begin() + reader.position() + 12);
  err = gyro.deserialize(gyroBuf);
  if (err != DeserializeError::None)
    return err;

  return DeserializeError::None;
}

ByteBuffer ControllersDataResponse::serialize() const {
  BinaryWriter writer;

  // Write ControllerInfoShared
  ByteBuffer infoBuf = info.serialize();
  writer.writeBytes(infoBuf.data(), infoBuf.size());

  // Write remaining fields
  writer.write(static_cast<uint8_t>(connected ? 1 : 0));
  writer.write(packetNum);

  writer.write(buttons.buttons1);
  writer.write(buttons.buttons2);

  writer.write(static_cast<uint8_t>(home ? 1 : 0));
  writer.write(static_cast<uint8_t>(touchButton ? 1 : 0));

  writer.write(lStickX);
  writer.write(lStickY);
  writer.write(rStickX);
  writer.write(rStickY);
  writer.write(aDPadL);
  writer.write(aDPadD);
  writer.write(aDPadR);
  writer.write(aDPadU);
  writer.write(aY);
  writer.write(aB);
  writer.write(aA);
  writer.write(aX);
  writer.write(aR1);
  writer.write(aL1);
  writer.write(aR2);
  writer.write(aL2);

  ByteBuffer touch1Buf = touch1.serialize();
  writer.writeBytes(touch1Buf.data(), touch1Buf.size());

  ByteBuffer touch2Buf = touch2.serialize();
  writer.writeBytes(touch2Buf.data(), touch2Buf.size());

  writer.write(timestamp);

  ByteBuffer accelBuf = accel.serialize();
  writer.writeBytes(accelBuf.data(), accelBuf.size());

  ByteBuffer gyroBuf = gyro.serialize();
  writer.writeBytes(gyroBuf.data(), gyroBuf.size());

  return writer.getBuffer();
}

DeserializeError ControllersMotorsRequest::deserialize(const ByteBuffer &buf) {
  return controllerId.deserialize(buf);
}

ByteBuffer ControllersMotorsRequest::serialize() const {
  return controllerId.serialize();
}

DeserializeError ControllersMotorsResponse::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 12) {
    return DeserializeError::ErrInvalidLength;
  }

  ByteBuffer infoBuf(buf.begin(), buf.begin() + 11);
  auto err = info.deserialize(infoBuf);
  if (err != DeserializeError::None)
    return err;

  motorCount = buf[11];
  return DeserializeError::None;
}

ByteBuffer ControllersMotorsResponse::serialize() const {
  ByteBuffer result = info.serialize();
  result.push_back(motorCount);
  return result;
}

DeserializeError
ControllersMotorsRumbleRequest::deserialize(const ByteBuffer &buf) {
  if (buf.size() != 10) {
    return DeserializeError::ErrInvalidLength;
  }

  auto err = controllerId.deserialize(buf);
  if (err != DeserializeError::None)
    return err;

  motorID = buf[8];
  intensity = buf[9];
  return DeserializeError::None;
}

ByteBuffer ControllersMotorsRumbleRequest::serialize() const {
  ByteBuffer result = controllerId.serialize();
  result.push_back(motorID);
  result.push_back(intensity);
  return result;
}
