//
// Created by jadjer on 13.11.25.
//

#include "packet_builder.hpp"

#include <string>

namespace {
    constexpr std::size_t MIN_PACKET_BITS = 1 + 1 + 12 + 1 + 12 + 1 + 1 + 4 + 1 + 1 + 8 + 1 + 1;
}

void PacketBuilder::startPacket() {
    reset();
    m_bitBuffer.push_back(1);
}

void PacketBuilder::addBit(PacketBuilder::Bit bit) {
    m_bitBuffer.push_back(bit);
}

std::optional<Packet> PacketBuilder::getCompletePacket() {
    if (checkBitsForMinimalPacket()) {
        return std::nullopt;
    }

    std::size_t bitIndex = 0;

    Packet packet;

    // Broadcast
    packet.startBit = m_bitBuffer[bitIndex++];

    // Broadcast
    packet.broadcast = m_bitBuffer[bitIndex++];

    // Master address (12 bits)
    packet.masterAddress = 0;
    for (int i = 0; i < 12; i++) {
        packet.masterAddress |= (m_bitBuffer[bitIndex++] ? 1UL : 0UL) << (11 - i);
    }

    // Master parity
    packet.masterParity = m_bitBuffer[bitIndex++];

    // Slave address (12 bits)
    packet.slaveAddress = 0;
    for (int i = 0; i < 12; i++) {
        packet.slaveAddress |= (m_bitBuffer[bitIndex++] ? 1UL : 0UL) << (11 - i);
    }

    // Slave parity
    packet.slaveParity = m_bitBuffer[bitIndex++];

    // Slave ACK
    packet.slaveAck = m_bitBuffer[bitIndex++];

    // Control (4 bits)
    packet.control = 0;
    for (int i = 0; i < 4; i++) {
        packet.control |= (m_bitBuffer[bitIndex++] ? 1UL : 0UL) << (3 - i);
    }

    // Control parity
    packet.controlParity = m_bitBuffer[bitIndex++];

    // Control ACK
    packet.controlAck = m_bitBuffer[bitIndex++];

    // Data length (8 bits)
    packet.dataLength = 0;
    for (int i = 0; i < 8; i++) {
        packet.dataLength |= (m_bitBuffer[bitIndex++] ? 1UL : 0UL) << (7 - i);
    }

    // Data length parity
    packet.dataLengthParity = m_bitBuffer[bitIndex++];

    // Data length ACK
    packet.dataLengthAck = m_bitBuffer[bitIndex++];

    // Data bytes
    packet.data.clear();
    packet.dataParity.clear();
    packet.dataAck.clear();

    for (int byte_idx = 0; byte_idx < packet.dataLength; byte_idx++) {
        if (bitIndex + 10 > m_bitBuffer.size()) {
            return std::nullopt; // Not enough bits for complete data
        }

        // Data byte (8 bits)
        uint8_t data_byte = 0;
        for (int i = 0; i < 8; i++) {
            data_byte |= (m_bitBuffer[bitIndex++] ? 1UL : 0UL) << (7 - i);
        }
        packet.data.push_back(data_byte);

        // Data parity
        packet.dataParity.push_back(m_bitBuffer[bitIndex++]);

        // Data ACK
        packet.dataAck.push_back(m_bitBuffer[bitIndex++]);
    }

    return packet;
}

void PacketBuilder::reset() {
    m_bitBuffer.clear();
}

bool PacketBuilder::checkBitsForMinimalPacket() {
    if (m_bitBuffer.size() < MIN_PACKET_BITS) {
        return false;
    }
    return true;
}

bool PacketBuilder::calculateParity(std::uint16_t const data, std::size_t const bits) const {
    bool parity = false;
    for (size_t i = 0; i < bits; i++) {
        if (data & (1UL << i)) {
            parity = !parity;
        }
    }
    return parity;
}

bool PacketBuilder::calculateParity(std::uint8_t const data) const {
    bool parity = false;
    for (int i = 0; i < 8; i++) {
        if (data & (1UL << i)) {
            parity = !parity;
        }
    }
    return parity;
}

void PacketBuilder::printBits() {
    std::string result;
    for (auto const bit : m_bitBuffer) {
        result += (bit ? '1' : '0');
    }
    printf("%s\n", result.c_str());
}
