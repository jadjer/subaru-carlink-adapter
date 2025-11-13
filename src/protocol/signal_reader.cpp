//
// Created by jadjer on 13.11.25.
//

#include "signal_reader.hpp"

#include <cmath>
#include <cstdint>
#include <hardware/adc.h>

namespace {
    constexpr std::uint8_t BUS_A_PIN = 28;
    constexpr std::uint8_t BUS_B_PIN = 29;
    constexpr std::uint8_t BUS_A_CHANNEL = 2;
    constexpr std::uint8_t BUS_B_CHANNEL = 3;

    constexpr std::uint16_t DIFF_THRESHOLD_0 = 150;
    constexpr std::uint16_t DIFF_THRESHOLD_1 = 25;
}

SignalReader::SignalReader() {
    adc_init();
    adc_gpio_init(BUS_A_PIN);
    adc_gpio_init(BUS_B_PIN);
}

SignalLevel SignalReader::getBusLevel() {
    adc_select_input(BUS_A_CHANNEL);
    std::uint16_t const valueBusA = adc_read();

    adc_select_input(BUS_B_CHANNEL);
    std::uint16_t const valueBusB = adc_read();

    std::uint16_t const difference = std::abs(valueBusA - valueBusB);

    if (difference >= DIFF_THRESHOLD_0) {
        return SignalLevel::LEVEL_0;
    }
    if (difference <= DIFF_THRESHOLD_1) {
        return SignalLevel::LEVEL_1;
    }

    return SignalLevel::UNKNOWN;
}
