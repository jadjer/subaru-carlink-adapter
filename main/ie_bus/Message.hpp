//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <cstdint>

using Byte    = std::uint8_t;
using Address = std::uint16_t;

enum class BroadcastType {
    BROADCAST = 0,
    TO_DEVICE = 1,
};

#pragma pack(push, 1)
struct Message {
    BroadcastType broadcast;
    Address master;
    Address slave;
    Byte control;
    Byte dataLength;
    Byte data[32];

    // Проверка целостности
    [[nodiscard]] auto isValid() const -> bool {
        return dataLength <= 32;
    }
};
#pragma pack(pop)
