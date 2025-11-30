//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <cstdint>
#include <expected>

enum class BitType {
    BIT_0     = 0x00,
    BIT_1     = 0x01,
    START_BIT = 0x10,
};

enum class ReadError {
    BIT_TYPE_WRONG,
};

using Pin  = std::uint8_t;
using Time = std::uint64_t;

class Driver {
public:
    Driver(Pin rx, Pin tx, Pin enable) noexcept;

public:
    auto enable() const -> void;

    auto disable() const -> void;

public:
    [[nodiscard]] auto isEnabled() const -> bool;

    [[nodiscard]] auto isBusLow() const -> bool;

    [[nodiscard]] auto isBusHigh() const -> bool;

public:
    [[nodiscard]] auto readBit() const -> std::expected<BitType, ReadError>;

public:
    auto writeBit(BitType value) const -> void;

private:
    auto waitBusLow() const -> void;

    auto waitBusHigh() const -> void;

private:
    Pin const m_rxPin;
    Pin const m_txPin;
    Pin const m_enablePin;
};
