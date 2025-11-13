//
// Created by jadjer on 13.11.25.
//

#include "protocol_analyzer.hpp"

#include <pico/time.h>

void ProtocolAnalyzer::process() {
    auto const busLevel = m_adcReader.getBusLevel();
    auto const currentTime = get_absolute_time();

    m_bitDecoder.processLevelChange(busLevel, currentTime);

    auto const decodedBit = m_bitDecoder.getDecodedBit();
    if (not decodedBit.has_value()) {
        return;
    }

    switch (decodedBit.value()) {
        case Bit::UNKNOWN:
            break;
        case Bit::START_BIT:
            m_packetBuilder.printBits();
            m_packetBuilder.startPacket();
            break;
        case Bit::BIT_1:
            m_packetBuilder.addBit(1);
            break;
        case Bit::BIT_0:
            m_packetBuilder.addBit(0);
            break;
    }
}

std::optional<Packet> ProtocolAnalyzer::getDecodedPacket() {
    return m_packetBuilder.getCompletePacket();
}
