//
// Created by jadjer on 13.11.25.
//

#ifndef AVC_PROTOCOL_BIT_DECODER_HPP
#define AVC_PROTOCOL_BIT_DECODER_HPP

#include <cstdint>
#include <vector>
#include <optional>

#include "signal_reader.hpp"

enum class Bit {
    UNKNOWN,
    START_BIT,
    BIT_0,
    BIT_1
};

using Time = std::uint64_t;

class BitDecoder {
    using Bits = std::vector<Bit>;

public:
    void processLevelChange(SignalLevel currentLevel, Time currentTime);

    std::optional<Bit> getDecodedBit();

    void reset();

private:
    Bits m_bits{};
    Time m_bitStartTime = 0;
    SignalLevel m_lastLevel = SignalLevel::UNKNOWN;
};

#endif //AVC_PROTOCOL_BIT_DECODER_HPP
