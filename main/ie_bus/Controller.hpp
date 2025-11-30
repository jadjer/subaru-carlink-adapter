//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <expected>
#include <optional>

#include "Driver.hpp"
#include "Message.hpp"

using Bit = bool;

enum class MessageError {
    BUS_READ_ERROR,
    BIT_SEQUENCE_ERROR,
    BIT_PARITY_ERROR,
    BITS_COUNT_ERROR,
};

class Controller {
public:
    Controller(Pin rx, Pin tx, Pin enable, Address device);

public:
    auto enable() const -> void;
    auto disable() const -> void;

public:
    [[nodiscard]] auto isEnabled() const -> bool;

public:
    [[nodiscard]] auto readMessage() const -> std::optional<Message>;

private:
    [[nodiscard]] auto readStartBit() const -> bool;
    [[nodiscard]] auto readBroadcastBit() const -> std::expected<bool, MessageError>;
    [[nodiscard]] auto readData(std::size_t bitsCount) const -> std::expected<std::uint32_t, MessageError>;

private:
    [[nodiscard]] auto writeAck() const -> bool;
    [[nodiscard]] auto skipAck() const -> bool;
    [[nodiscard]] auto checkParity(std::uint8_t parity) const -> bool;

private:
    Address const m_device;

private:
    Driver m_driver;
};
