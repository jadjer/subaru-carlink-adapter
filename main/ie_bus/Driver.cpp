//
// Created by jadjer on 29.11.2025.
//

#include "Driver.hpp"

#include <esp_timer.h>
#include <vector>
#include <driver/gpio.h>

namespace {
    auto constexpr START_BIT_PULSE_WIDTH = 170;
    auto constexpr BIT_0_PULSE_WIDTH = 33;
    auto constexpr BIT_1_PULSE_WIDTH = 20;
    auto constexpr BIT_PULSE_WIDTH_THRESHOLD = 5;
    auto constexpr START_BIT_MIN_PULSE_WIDTH = START_BIT_PULSE_WIDTH - BIT_PULSE_WIDTH_THRESHOLD;
    auto constexpr START_BIT_MAX_PULSE_WIDTH = START_BIT_PULSE_WIDTH + BIT_PULSE_WIDTH_THRESHOLD;
    auto constexpr BIT_0_MIN_PULSE_WIDTH = BIT_0_PULSE_WIDTH - BIT_PULSE_WIDTH_THRESHOLD;
    auto constexpr BIT_0_MAX_PULSE_WIDTH = BIT_0_PULSE_WIDTH + BIT_PULSE_WIDTH_THRESHOLD;
    auto constexpr BIT_1_MIN_PULSE_WIDTH = BIT_1_PULSE_WIDTH - BIT_PULSE_WIDTH_THRESHOLD;
    auto constexpr BIT_1_MAX_PULSE_WIDTH = BIT_1_PULSE_WIDTH + BIT_PULSE_WIDTH_THRESHOLD;
}

Driver::Driver(Pin const rx, Pin const tx, Pin const enable) : m_rx(rx), m_tx(tx), m_enable(enable),
                                                               m_isEnabled(false) {
    reconfigurePins();
}

void Driver::setRxPin(Pin const pin) {
    m_rx = pin;
    reconfigurePins();
}

void Driver::setTxPin(Pin const pin) {
    m_tx = pin;
    reconfigurePins();
}

void Driver::setEnablePin(Pin const pin) {
    m_enable = pin;
    reconfigurePins();
}

void Driver::enable() {
    gpio_set_level(static_cast<gpio_num_t>(m_enable), 1);
    m_isEnabled = true;
}

auto Driver::readMessage() -> std::optional<Message> {
    Message message{};

    auto const isStartBit = readStartBit();
    if (not isStartBit) {
        return std::nullopt;
    }

    message.isBroadcast = readBroadCastBit();


    message.masterAddress = readBits(12);


    return message;
}

bool Driver::isEnabled() const {
    return m_isEnabled;
}

auto Driver::readStartBit() const -> bool {
    while (inputIsSet()) {
    }

    auto const pulseStartTime = esp_timer_get_time();

    while (inputIsClear()) {
        auto const currentTime = esp_timer_get_time();
        auto const pulseWidth = currentTime - pulseStartTime;

        if (pulseWidth > START_BIT_MAX_PULSE_WIDTH) {
            return false;
        }
    }

    auto const pulseStopTime = esp_timer_get_time();
    auto const pulseWith = pulseStopTime - pulseStartTime;

    if (pulseWith < START_BIT_MIN_PULSE_WIDTH or pulseWith > START_BIT_MAX_PULSE_WIDTH) {
        return false;
    }

    return true;
}

auto Driver::readBroadCastBit() const -> Bit {
    auto const isBroadCast = readBit();
    return isBroadCast;
}

auto Driver::readMasterAddress() const -> std::uint16_t {
}

auto Driver::readSlaveAddress() const -> std::uint16_t {
}

auto Driver::readControlBits() const -> std::uint8_t {
}

auto Driver::readDataLength() const -> std::uint8_t {
}

auto Driver::readData() const -> std::uint8_t {
}

auto Driver::readAcknowledge() const -> Bit {
}

auto Driver::readParity() const -> Bit {
}

auto Driver::readBit() const -> Bit {
    while (inputIsSet()) {
    }

    auto const pulseStartTime = esp_timer_get_time();

    while (inputIsClear()) {
    }

    auto const pulseStopTime = esp_timer_get_time();
    auto const pulseWith = pulseStopTime - pulseStartTime;

    if (pulseWith >= BIT_0_MIN_PULSE_WIDTH and pulseWith <= BIT_0_MAX_PULSE_WIDTH) {
        return Bit::BIT_0;
    }

    if (pulseWith >= BIT_1_MIN_PULSE_WIDTH and pulseWith <= BIT_1_MAX_PULSE_WIDTH) {
        return Bit::BIT_1;
    }

    return Bit::BIT_UNKNOWN;
}

auto Driver::readBits(std::size_t const count) const -> Bitstream {
    Bitstream bits;
    bits.data.reserve(count);

    for (auto i = 0; i < count; i++) {
        bits.data.push_back(readBit());
        bits.parity ^= bits.data.back();
    }

    // for (size_t i = 0; i < count; i++) {
    //     uint8_t bit = 0;
    //
    //     // Wait until rising edge of new bit
    //     while (gpio_get_level(static_cast<gpio_num_t>(m_rx)) == 0) {
    //     }
    //
    //     // Reset timer to measure bit length
    //     int64_t startTime = esp_timer_get_time();
    //
    //     // Wait until falling edge
    //     while (gpio_get_level(static_cast<gpio_num_t>(m_rx)) == 1) {
    //     }
    //
    //     int64_t duration = esp_timer_get_time() - startTime;
    //
    //     // Compare duration to determine if it's a 1 or 0
    //     if (duration < 26) {
    //         // 26us threshold
    //         bit = 1;
    //     }
    //
    //     bits.push_back(bit);
    // }

    return bits;
}

auto Driver::inputIsClear() const -> bool {
    return gpio_get_level(static_cast<gpio_num_t>(m_rx)) == 0;
}

auto Driver::inputIsSet() const -> bool {
    return gpio_get_level(static_cast<gpio_num_t>(m_rx)) == 1;
}

void Driver::reconfigurePins() const {
    gpio_config_t const receiverConfiguration = {
        .pin_bit_mask = (1ULL << m_rx),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&receiverConfiguration);

    gpio_config_t const transmitterConfiguration = {
        .pin_bit_mask = (1ULL << m_tx),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&transmitterConfiguration);

    gpio_config_t const enableConfiguration = {
        .pin_bit_mask = (1ULL << m_tx),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&enableConfiguration);
}
