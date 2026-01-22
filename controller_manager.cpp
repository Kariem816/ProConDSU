#include "controller_manager.hpp"

#include <iostream>
#include <print>

ControllerManager::ControllerManager() = default;

ControllerManager::~ControllerManager() {
  // Controllers will be automatically cleaned up when unique_ptrs are destroyed
}

void ControllerManager::initialize() {
  auto device_paths = enumerateDevices();
  std::println("Found {} ProController device(s)", device_paths.size());

  for (const auto &path : device_paths) {
    if (connectController(path.c_str(), true)) {
      std::println("Connected to controller: {}", path);
    } else {
      std::println("Failed to connect to controller: {}", path);
    }
  }
}

void ControllerManager::update() {
  // Controllers update their input status via callbacks internally
  // This method is reserved for future batch updates if needed
}

bool ControllerManager::connectController(const char *device_path,
                                          bool enable_imu) {
  auto controller = ProControllerHid::ProController::Connect(
      device_path, enable_imu,
      [](const char *log) { std::println("ProController: {}", log); }, false);

  if (!controller) {
    return false;
  }

  // Set up input callback to cache the latest state
  size_t controller_index = controllers.size();
  controller->SetInputStatusCallback(
      [this, controller_index](const ProControllerHid::InputStatus &status) {
        if (controller_index < lastInputStates.size()) {
          lastInputStates[controller_index] = status;
        }
      });

  controllers.push_back(std::move(controller));
  lastInputStates.push_back(ProControllerHid::InputStatus{});

  return true;
}

std::vector<std::string> ControllerManager::enumerateDevices() const {
  return ProControllerHid::ProController::EnumerateProControllerDevicePaths();
}

size_t ControllerManager::getConnectedControllerCount() const {
  return controllers.size();
}

bool ControllerManager::getControllerInputStatus(
    size_t index, ProControllerHid::InputStatus &status) const {
  if (index >= lastInputStates.size()) {
    return false;
  }
  status = lastInputStates[index];
  return true;
}

void ControllerManager::setPlayerLed(size_t index, uint8_t player_led_bits) {
  if (index < controllers.size()) {
    controllers[index]->SetPlayerLed(player_led_bits);
  }
}

void ControllerManager::setRumble(
    size_t index, const ProControllerHid::ProController::BasicRumble &rumble) {
  if (index < controllers.size()) {
    controllers[index]->SetRumble(rumble);
  }
}
