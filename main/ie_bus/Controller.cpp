//
// Created by jadjer on 29.11.2025.
//

#include "Controller.hpp"

#include <esp_log.h>
#include <esp_timer.h>

namespace {
    auto constexpr TAG             = "IEBusController";
    Size constexpr MAX_DATA_LENGTH = 32;

    Driver::Time getTimeUs() {
        return esp_timer_get_time();
    }

    void delayUs(Driver::Time const delay) {
        auto const startTime = getTimeUs();

        bool enable = true;
        while (enable) {
            auto const currentTime = getTimeUs();
            auto const differenceTime = currentTime - startTime;
            auto const isTimeOut = differenceTime >= delay;

            if (isTimeOut) {
                enable = false;
            }
        }
    }
} // namespace

Controller::Controller(Driver::Pin const rx, Driver::Pin const tx, Driver::Pin const enable, Address const address)
    : m_address(address), m_driver(rx, tx, enable) {}

Controller::~Controller() {
    disable();

    // if (m_processThread.joinable()) {
    //     m_processThread.join();
    // }
}

auto Controller::enable() -> void {
    m_driver.enable();

    // m_processThread = std::thread(&Controller::loop, this);
    // m_processThread.detach();
}

auto Controller::disable() -> void {
    m_driver.disable();
}

auto Controller::isEnabled() const -> bool {
    return m_driver.isEnabled();
}

auto Controller::getMessage() -> std::optional<Message> {
    if (m_receiveQueue.isEmpty()) {
        return std::nullopt;
    }

    auto const message = m_receiveQueue.pop();

    return message;
}

auto Controller::putMessage(Message const& message) -> void {
    m_transmitQueue.push(message);
}

auto Controller::loop() -> void {
    while (isEnabled()) {
        auto const optionalMessage = readMessage();
        if (optionalMessage.has_value()) {
            m_receiveQueue.push(optionalMessage.value());
        }

        if (not m_transmitQueue.isEmpty()) {
            auto const message = m_transmitQueue.pop();
            auto const isWritten = writeMessage(message);

            if (not isWritten) {
                m_transmitQueue.push(message);
            }
        }
    }
}

auto Controller::readMessage() const -> std::optional<Message> {
    if (not m_driver.receiveStartBit()) {
        return std::nullopt;
    }

    Message message = {};

    {
        if (auto const broadcastBit = m_driver.receiveBit(); broadcastBit == 0) {
            message.broadcast = BroadcastType::BROADCAST;
        } else {
            message.broadcast = BroadcastType::TO_DEVICE;
        }
    }

    {
        message.master = m_driver.receiveBits(12);

        auto const masterParityBit = m_driver.receiveBit();
        if (auto const isParityValid = checkParity(message.master, 12, masterParityBit); not isParityValid) {
            ESP_LOGW(TAG, "Master address parity error");
            return std::nullopt;
        }
    }

    {
        message.slave = m_driver.receiveBits(12);

        auto const parityBit     = m_driver.receiveBit();
        auto const isParityValid = checkParity(message.slave, 12, parityBit);

        auto const slaveAck = m_driver.receiveBit();
        auto const isAnswer = message.broadcast == BroadcastType::TO_DEVICE and message.slave == m_address and slaveAck;

        if (not isParityValid) {
            if (isAnswer) {
                m_driver.sendAckBit(AcknowledgmentType::NAK);
            }

            ESP_LOGW(TAG, "Slave address parity error");
            return std::nullopt;
        }

        if (isAnswer) {
            m_driver.sendAckBit(AcknowledgmentType::ACK);
        }
    }

    {
        message.control = m_driver.receiveBits(4);

        auto const parityBit     = m_driver.receiveBit();
        auto const isParityValid = checkParity(message.control, 4, parityBit);

        auto const slaveAck = m_driver.receiveBit();
        auto const isAnswer = message.broadcast == BroadcastType::TO_DEVICE and message.slave == m_address and slaveAck;

        if (not isParityValid) {
            if (isAnswer) {
                m_driver.sendAckBit(AcknowledgmentType::NAK);
            }

            ESP_LOGW(TAG, "Control parity error");
            return std::nullopt;
        }

        if (isAnswer) {
            m_driver.sendAckBit(AcknowledgmentType::ACK);
        }
    }

    {
        message.dataLength = m_driver.receiveBits(8);

        auto const parityBit     = m_driver.receiveBit();
        auto const isParityValid = checkParity(message.dataLength, 8, parityBit);

        auto const slaveAck = m_driver.receiveBit();
        auto const isAnswer = message.broadcast == BroadcastType::TO_DEVICE and message.slave == m_address and slaveAck;

        if (auto const isDataLengthValid = message.dataLength <= MAX_DATA_LENGTH;
            not isParityValid or not isDataLengthValid) {
            if (isAnswer) {
                m_driver.sendAckBit(AcknowledgmentType::NAK);
            }

            if (not isParityValid) {
                ESP_LOGW(TAG, "Length parity error");
            }
            if (not isDataLengthValid) {
                ESP_LOGW(TAG, "Data length too large: %u", message.dataLength);
            }

            return std::nullopt;
        }

        if (isAnswer) {
            m_driver.sendAckBit(AcknowledgmentType::ACK);
        }
    }

    for (Size i = 0; i < message.dataLength; i++) {
        message.data[i] = m_driver.receiveBits(8);

        auto const parityBit     = m_driver.receiveBit();
        auto const isParityValid = checkParity(message.data[i], 8, parityBit);

        auto const slaveAck = m_driver.receiveBit();
        auto const isAnswer = message.broadcast == BroadcastType::TO_DEVICE and message.slave == m_address and slaveAck;

        if (not isParityValid) {
            if (isAnswer) {
                m_driver.sendAckBit(AcknowledgmentType::NAK);
            }

            ESP_LOGW(TAG, "Data byte %u parity error", i);
            return std::nullopt;
        }

        if (isAnswer) {
            m_driver.sendAckBit(AcknowledgmentType::ACK);
        }
    }

    return message;
}

auto Controller::writeMessage(Message const& message) const -> bool {
    // Ждём освобождения шины
    while (not m_driver.isBusFree()) {
        delayUs(100);
    }

    m_driver.transmitStartBit();

    if (message.broadcast == BroadcastType::BROADCAST) {
        m_driver.transmitBit(0);
    } else {
        m_driver.transmitBit(1);
    }

    {
        m_driver.transmitBits(message.master, 12);

        auto const parityBit = calculateParity(message.master, 12);
        m_driver.transmitBit(parityBit);
    }

    {
        m_driver.transmitBits(message.slave, 12);

        auto const parityBit = calculateParity(message.slave, 12);
        m_driver.transmitBit(parityBit);

        if (auto const ackBit = m_driver.waitAckBit(); ackBit == AcknowledgmentType::NAK) {
            ESP_LOGE(TAG, "No ACK for address");
            return false;
        }
    }

    {
        m_driver.transmitBits(message.control, 4);

        auto const parityBit = calculateParity(message.control, 4);
        m_driver.transmitBit(parityBit);

        if (auto const ackBit = m_driver.waitAckBit(); ackBit == AcknowledgmentType::NAK) {
            ESP_LOGE(TAG, "No ACK for control");
            return false;
        }
    }

    {
        m_driver.transmitBits(message.dataLength, 8);

        auto const parityBit = calculateParity(message.dataLength, 8);
        m_driver.transmitBit(parityBit);

        if (auto const ackBit = m_driver.waitAckBit(); ackBit == AcknowledgmentType::NAK) {
            ESP_LOGE(TAG, "No ACK for data length");
            return false;
        }
    }

    for (Size i = 0; i < message.dataLength; i++) {
        m_driver.transmitBits(message.data[i], 8);

        auto const parityBit = calculateParity(message.data[i], 8);
        m_driver.transmitBit(parityBit);

        if (auto const ackBit = m_driver.waitAckBit(); ackBit == AcknowledgmentType::NAK) {
            ESP_LOGE(TAG, "No ACK for data byte %u", i);
            return false;
        }
    }

    return true;
}

auto Controller::checkParity(Data const data, Size const size, Bit const parity) -> Bit {
    return calculateParity(data, size) == parity;
}

auto Controller::calculateParity(Data const data, Size const size) -> Bit {
    Bit parity = 0;

    for (auto i = 0; i < size; i++) {
        parity ^= data >> i & 1;
    }

    return parity;
}
