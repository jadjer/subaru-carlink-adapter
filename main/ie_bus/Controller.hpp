//
// Created by jadjer on 29.11.2025.
//

#pragma once

#include <optional>
#include <thread>

#include "Driver.hpp"
#include "Message.hpp"
#include "Queue.hpp"


enum class MessageError {
    BUS_READ_ERROR,
    BIT_SEQUENCE_ERROR,
    BIT_PARITY_ERROR,
    BITS_COUNT_ERROR,
    START_BIT_READ_ERROR,
    BROADCAST_BIT_READ_ERROR,
    MASTER_ADDRESS_READ_ERROR,
    SLAVE_ADDRESS_READ_ERROR,
    CONTROL_DATA_READ_ERROR,
    DATA_LENGTH_READ_ERROR,
    DATA_READ_ERROR,
    ACKNOWLEDGE_ERROR,
};

/**
 * @class Controller
 * IEBus Controller
 */
class Controller {
private:
    using Thread       = std::thread;
    using MessageQueue = Queue<Message>;

public:
    Controller(Driver::Pin rx, Driver::Pin tx, Driver::Pin enable, Address address);
    ~Controller();

public:
    Controller(Controller const&)                = delete;
    Controller& operator=(Controller const&)     = delete;
    Controller(Controller&&) noexcept            = delete;
    Controller& operator=(Controller&&) noexcept = delete;

public:
    /**
     * Enable IEBus driver
     */
    auto enable() -> void;
    /**
     * Enable IEBus driver
     */
    auto disable() -> void;

public:
    /**
     *
     * @return
     */
    [[nodiscard]] auto isEnabled() const -> bool;

public:
    [[nodiscard]] auto readMessage() const -> std::optional<Message>;
    [[nodiscard]] auto writeMessage(Message const& message) const -> bool;

public:
    /**
     * Get message from queue
     * @return
     */
    [[nodiscard]] auto getMessage() -> std::optional<Message>;
    auto putMessage(Message const& message) -> void;

private:
    auto loop() -> void;



private:
    /**
     * Check parity for calculated parity
     * @param data Data for check
     * @param size Data size
     * @param parity Etalon parity
     * @return Comparison result
     */
    static auto checkParity(Data data, Size size, Bit parity) -> Bit;
    /**
     * Calculate parity for calculated parity
     * @param data Data for calculate
     * @param size Data size
     * @return Parity bit
     */
    static auto calculateParity(Data data, Size size) -> Bit;

private:
    Address const m_address;

private:
    Driver m_driver;
    Thread m_processThread;
    MessageQueue m_receiveQueue;
    MessageQueue m_transmitQueue;

private:
    Message m_currentMessage = Message();
};
