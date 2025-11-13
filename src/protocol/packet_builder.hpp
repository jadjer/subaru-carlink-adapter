//
// Created by jadjer on 13.11.25.
//

#ifndef AVC_PROTOCOL_PACKET_BUILDER_HPP
#define AVC_PROTOCOL_PACKET_BUILDER_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "packet.hpp"

class PacketBuilder {
public:
    using Bit = std::uint8_t;
    using Bits = std::vector<Bit>;

public:
    void startPacket();
    void addBit(Bit bit);

public:
    std::optional<Packet> getCompletePacket();

public:
    void reset();
    void printBits();

private:
    [[nodiscard]] bool checkBitsForMinimalPacket();
    [[nodiscard]] bool calculateParity(std::uint16_t data, std::size_t bits) const;
    [[nodiscard]] bool calculateParity(std::uint8_t data) const;

private:
    Bits m_bitBuffer{};
};


#endif //AVC_PROTOCOL_PACKET_BUILDER_HPP
