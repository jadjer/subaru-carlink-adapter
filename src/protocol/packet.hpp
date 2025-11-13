//
// Created by jadjer on 13.11.25.
//

#ifndef AVC_PROTOCOL_PACKET_HPP
#define AVC_PROTOCOL_PACKET_HPP

#include <cstdint>

struct Packet {
    std::uint8_t startBit;
    std::uint8_t broadcast;

    std::uint16_t masterAddress;
    std::uint8_t masterParity;

    std::uint16_t slaveAddress;
    std::uint8_t slaveParity;
    std::uint8_t slaveAck;

    std::uint8_t control;
    std::uint8_t controlParity;
    std::uint8_t controlAck;

    std::uint8_t dataLength;
    std::uint8_t dataLengthParity;
    std::uint8_t dataLengthAck;

    std::vector<std::uint8_t> data;
    std::vector<std::uint8_t> dataParity;
    std::vector<std::uint8_t> dataAck;
};

#endif //AVC_PROTOCOL_PACKET_HPP
