#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "ProControllerHid/ProController.h"
#include "packet/packet.hpp"

class ControllerManager {
public:
  ControllerManager();
  ~ControllerManager();

  // Initialize and scan for controllers
  void initialize();

  // Update controller states (call this regularly)
  void update();

  // Connect to a specific controller by device path
  bool connectController(const char *device_path, bool enable_imu = false);

  // Enumerate available controller device paths
  std::vector<std::string> enumerateDevices() const;

  // Get the number of connected controllers
  size_t getConnectedControllerCount() const;

  // Get input status from a specific controller
  bool getControllerInputStatus(size_t index,
                                ProControllerHid::InputStatus &status) const;

  // Set player LED for a controller
  void setPlayerLed(size_t index, uint8_t player_led_bits);

  // Set rumble for a controller
  void setRumble(size_t index,
                 const ProControllerHid::ProController::BasicRumble &rumble);

private:
  std::vector<std::unique_ptr<ProControllerHid::ProController>> controllers;
  std::vector<ProControllerHid::InputStatus> lastInputStates;
};
