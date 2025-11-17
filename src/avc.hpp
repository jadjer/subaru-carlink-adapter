//
// Created by jadjer on 16.11.25.
//

#ifndef AVC_AVC_HPP
#define AVC_AVC_HPP

#include <array>
#include <cstdint>
#include <optional>

#include "avc_line.hpp"

constexpr auto MAX_MESSAGE_LEN = 32;

struct Message {
    bool broadcast;
    std::uint16_t master;
    std::uint16_t slave;
    std::uint8_t control;
    std::uint8_t length;
    std::uint8_t data[MAX_MESSAGE_LEN];
};

class AVC {
public:
    using Byte = std::uint8_t;
    using BitCount = std::size_t;

public:
    AVC();

public:
    std::optional<Message> readMessage();

private:
    Byte readByte(BitCount bitCount);
    [[nodiscard]] bool readACK() const;

private:
    [[nodiscard]] bool isStartBit() const;
    void sendStartBit() const;
    void sendBit0() const;
    void sendBit1() const;
    void sendACK() const;

private:
    AVCLine *m_avcLine = nullptr;

private:
    std::uint8_t m_parityBit = 0;
};


#endif //AVC_AVC_HPP
