//
// Created by jadjer on 13.11.25.
//

#include "bit_decoder.hpp"

#include <cmath>

namespace {
    constexpr auto BIT_THRESHOLD = 27;
    constexpr auto START_BIT_THRESHOLD = 150;
}

void BitDecoder::processLevelChange(SignalLevel const currentLevel, Time const currentTime) {
    if (currentLevel == m_lastLevel) {
        return;
    }

    if (m_lastLevel == SignalLevel::LEVEL_0 and currentLevel == SignalLevel::LEVEL_1) {
        m_bitStartTime = currentTime;
    }

    if (m_lastLevel == SignalLevel::LEVEL_1 and currentLevel == SignalLevel::LEVEL_0) {
        Time const highDuration = currentTime - m_bitStartTime;

        if (highDuration > START_BIT_THRESHOLD) {
            m_bits.push_back(Bit::START_BIT);
        }
        if (highDuration > BIT_THRESHOLD) {
            m_bits.push_back(Bit::BIT_0);
        }
        if (highDuration < BIT_THRESHOLD) {
            m_bits.push_back(Bit::BIT_1);
        }
    }

    m_lastLevel = currentLevel;
}

std::optional<Bit> BitDecoder::getDecodedBit() {
    if (m_bits.empty()) {
        return std::nullopt;
    }

    Bit bit = m_bits.front();
    m_bits.erase(m_bits.begin());

    return bit;
}

void BitDecoder::reset() {
    m_bits.clear();
    m_lastLevel = SignalLevel::UNKNOWN;
    m_bitStartTime = 0;
}
