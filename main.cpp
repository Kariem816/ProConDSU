#include <condition_variable>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>

#include "dsu_server.hpp"
#include <windows.h>

std::mutex mtx;
std::condition_variable cv;
bool shutdown_requested = false;

BOOL ConsoleCtrlHandler(DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
    std::lock_guard<std::mutex> lock(mtx);
    shutdown_requested = true;
    cv.notify_all();
    return TRUE;
  }
  return FALSE;
}

void waitForCtrlC() {
  std::unique_lock<std::mutex> lock(mtx);
  cv.wait(lock, [] { return shutdown_requested; });
}

int main() {
  if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE)) {
    std::cerr << "ERROR: Could not set console control handler" << std::endl;
    return 1;
  }

  DsuServer server("0.0.0.0", 26760);

  server.start();
  waitForCtrlC();
  std::cout << "Shutting down..." << std::endl;
  server.stop();
  server.wait();

  return 0;
}
