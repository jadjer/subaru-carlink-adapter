//
// Created by jadjer on 13.11.25.
//

#ifndef AVC_PROTOCOL_PROTOCOL_ANALYZER_HPP
#define AVC_PROTOCOL_PROTOCOL_ANALYZER_HPP

#include <optional>

#include "protocol/signal_reader.hpp"
#include "protocol/bit_decoder.hpp"
#include "protocol/packet_builder.hpp"
#include "protocol/packet.hpp"

class ProtocolAnalyzer {
public:
    void process();

public:
    std::optional<Packet> getDecodedPacket();

private:
    BitDecoder m_bitDecoder;
    SignalReader m_adcReader;
    PacketBuilder m_packetBuilder;
};


#endif //AVC_PROTOCOL_PROTOCOL_ANALYZER_HPP
