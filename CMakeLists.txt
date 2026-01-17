cmake_minimum_required(VERSION 3.12)
project(ProConDSU LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(packet)

add_executable(proconDSU main.cpp udp_server.cpp udp_server.hpp dsu_server.cpp dsu_server.hpp dsu_client.hpp)
target_include_directories(proconDSU PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(proconDSU PRIVATE packet ws2_32 stdc++exp)