//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Message.hpp"

enum class Bit {
    BIT_UNKNOWN = -1,
    BIT_0 = 0,
    BIT_1 = 1,
};

using Bits = std::vector<Bit>;

struct Bitstream {
    Bits data;
    Bit parity;
};

using Pin = std::uint8_t;

class Driver {
public:
    Driver(Pin rx, Pin tx, Pin enable);

public:
    void setRxPin(Pin pin);

    void setTxPin(Pin pin);

    void setEnablePin(Pin pin);

public:
    void enable();

    void disable();

public:
    auto readMessage() -> std::optional<Message>;

public:
    [[nodiscard]] bool isEnabled() const;

private:
    [[nodiscard]] auto readStartBit() const -> bool;

    [[nodiscard]] auto readBroadCastBit() const -> Bit;

    [[nodiscard]] auto readMasterAddress() const -> std::uint16_t;

    [[nodiscard]] auto readSlaveAddress() const -> std::uint16_t;

    [[nodiscard]] auto readControlBits() const -> std::uint8_t;

    [[nodiscard]] auto readDataLength() const -> std::uint8_t;

    [[nodiscard]] auto readData() const -> std::uint8_t;

    [[nodiscard]] auto readAcknowledge() const -> Bit;

    [[nodiscard]] auto readParity() const -> Bit;

    [[nodiscard]] auto readBit() const -> Bit;

    [[nodiscard]] auto readBits(std::size_t count) const -> Bitstream;

private:
    [[nodiscard]] auto inputIsClear() const -> bool;

    [[nodiscard]] auto inputIsSet() const -> bool;

private:
    void reconfigurePins() const;

private:
    Pin m_rx;
    Pin m_tx;
    Pin m_enable;
    bool m_isEnabled;
};
