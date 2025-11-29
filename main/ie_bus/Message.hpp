//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <cstdint>

struct Message {
    bool isBroadcast;
    std::uint16_t masterAddress;
    std::uint16_t slaveAddress;
    std::uint8_t control;
    std::uint8_t dataLength;
    std::uint8_t data[32];
};