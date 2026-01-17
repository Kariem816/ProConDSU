add_library(packet STATIC
    packet.cpp
    utils.cpp
    formatters.hpp
    packet.hpp
    utils.hpp
)

target_include_directories(packet PRIVATE ${CMAKE_SOURCE_DIR})