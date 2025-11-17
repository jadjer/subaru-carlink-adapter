//
// Created by jadjer on 16.11.25.
//

#include "avc.hpp"

#include <hardware/gpio.h>
#include <hardware/timer.h>

namespace {
    // commands
    constexpr AVC::Byte stat1[] = {0x4, 0x00, 0x00, 0x01, 0x0A};
    constexpr AVC::Byte stat2[] = {0x4, 0x00, 0x00, 0x01, 0x08};
    constexpr AVC::Byte stat3[] = {0x4, 0x00, 0x00, 0x01, 0x0D};
    constexpr AVC::Byte stat4[] = {0x4, 0x00, 0x00, 0x01, 0x0C};
    constexpr AVC::Byte play_req1[] = {0x4, 0x00, 0x25, 0x63, 0x80};
    constexpr AVC::Byte play_req2[] = {0x6, 0x00, 0x11, 0x63, 0x42, 0x00, 0x00};
    constexpr AVC::Byte play_req3[] = {0x6, 0x00, 0x11, 0x63, 0x42, 0x41, 0x00};
    constexpr AVC::Byte stop_req[] = {0x5, 0x00, 0x11, 0x63, 0x43, 0x00};
    constexpr AVC::Byte stop_req2[] = {0x6, 0x00, 0x11, 0x63, 0x43, 0x00, 0x00};

    constexpr AVC::Byte next_track[] = {0x4, 0x00, 0x25, 0x63, 0x94};
    constexpr AVC::Byte prev_track[] = {0x4, 0x00, 0x25, 0x63, 0x95};

    constexpr AVC::Byte next_cd[] = {0x4, 0x00, 0x25, 0x63, 0x90};
    constexpr AVC::Byte prev_cd[] = {0x4, 0x00, 0x25, 0x63, 0x91};

    constexpr AVC::Byte fast_forward[] = {0x4, 0x00, 0x25, 0x63, 0x98};
    constexpr AVC::Byte fast_back[] = {0x4, 0x00, 0x25, 0x63, 0x99};

    constexpr AVC::Byte scan_on[] = {0x4, 0x00, 0x25, 0x63, 0xA6};
    constexpr AVC::Byte scan_off[] = {0x4, 0x00, 0x25, 0x63, 0xA7};
    constexpr AVC::Byte scan_d_on[] = {0x4, 0x00, 0x25, 0x63, 0xA9};
    constexpr AVC::Byte scan_d_off[] = {0x4, 0x00, 0x25, 0x63, 0xAA};

    constexpr AVC::Byte repeat_on[] = {0x4, 0x00, 0x25, 0x63, 0xA0};
    constexpr AVC::Byte repeat_off[] = {0x4, 0x00, 0x25, 0x63, 0xA1};
    constexpr AVC::Byte repeat_d_on[] = {0x4, 0x00, 0x25, 0x63, 0xA3};
    constexpr AVC::Byte repeat_d_off[] = {0x4, 0x00, 0x25, 0x63, 0xA4};

    constexpr AVC::Byte random_on[] = {0x4, 0x00, 0x25, 0x63, 0xB0};
    constexpr AVC::Byte random_off[] = {0x4, 0x00, 0x25, 0x63, 0xB1};
    constexpr AVC::Byte random_d_on[] = {0x4, 0x00, 0x25, 0x63, 0xB3};
    constexpr AVC::Byte random_d_off[] = {0x4, 0x00, 0x25, 0x63, 0xB4};

    // broadcast
    constexpr AVC::Byte lan_stat1[] = {0x3, 0x00, 0x01, 0x0A};
    constexpr AVC::Byte lan_reg[] = {0x3, 0x11, 0x01, 0x00};
    constexpr AVC::Byte lan_init[] = {0x3, 0x11, 0x01, 0x01};
    constexpr AVC::Byte lan_check[] = {0x4, 0x11, 0x01, 0x20, 0x00};
    constexpr AVC::Byte lan_check_CD_CH[] = {0x5, 0x00, 0x11, 0x01, 0x20, 0x00};
    constexpr AVC::Byte lan_playit[] = {0x5, 0x11, 0x01, 0x45, 0x63, 0x00};

    // answers
    constexpr AVC::Byte CMD_REGISTER[] = {0x1, 0x05, 0x00, 0x01, 0x11, 0x10, 0x63};
    constexpr AVC::Byte CMD_STATUS1[] = {0x1, 0x04, 0x00, 0x01, 0x00, 0x1A};
    constexpr AVC::Byte CMD_STATUS2[] = {0x1, 0x04, 0x00, 0x01, 0x00, 0x18};
    constexpr AVC::Byte CMD_STATUS3[] = {0x1, 0x04, 0x00, 0x01, 0x00, 0x1D};
    constexpr AVC::Byte CMD_STATUS4[] = {0x1, 0x05, 0x00, 0x01, 0x00, 0x1C, 0x00};
    constexpr AVC::Byte CMD_CHECK[] = {0x1, 0x06, 0x00, 0x01, 0x11, 0x30, 0x00, 0x00};
    constexpr AVC::Byte CMD_PLAY_OK1[] = {0x1, 0x05, 0x00, 0x63, 0x11, 0x50, 0x01};
    constexpr AVC::Byte CMD_PLAY_OK2[] = {0x1, 0x05, 0x00, 0x63, 0x11, 0x52, 0x01};
    constexpr AVC::Byte CMD_PLAY_OK3[] = {0x0, 0x0B, 0x63, 0x31, 0xF1, 0x01, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x80};
    constexpr AVC::Byte CMD_PLAY_OK4[] = {0x0, 0x0B, 0x63, 0x31, 0xF1, 0x01, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    constexpr AVC::Byte CMD_STOP1[] = {0x1, 0x05, 0x00, 0x63, 0x11, 0x53, 0x01};
    constexpr AVC::Byte CMD_STOP2[] = {0x0, 0x0B, 0x63, 0x31, 0xF1, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    constexpr AVC::Byte CMD_BEEP[] = {0x1, 0x05, 0x00, 0x63, 0x29, 0x60, 0x02};

    // Head Unid ID
    constexpr auto HU_ID_1 = 0x01;
    constexpr auto HU_ID_2 = 0x90;

    // CD Changer ID
    constexpr auto CD_ID_1 = 0x02;
    constexpr auto CD_ID_2 = 0x40;

    constexpr auto cmNull = 0;
    constexpr auto cmStatus1 = 1;
    constexpr auto cmStatus2 = 2;
    constexpr auto cmStatus3 = 3;
    constexpr auto cmStatus4 = 4;

    constexpr auto cmPlayReq1 = 5;
    constexpr auto cmPlayReq2 = 6;
    constexpr auto cmPlayReq3 = 7;
    constexpr auto cmStopReq = 8;
    constexpr auto cmStopReq2 = 9;

    constexpr auto cmRegister = 100;
    constexpr auto cmInit = 101;
    constexpr auto cmCheck = 102;
    constexpr auto cmPlayIt = 103;
    constexpr auto cmBeep = 110;
}

AVC::AVC() : m_avcLine(new AVCLine()) {

}

std::optional<Message> AVC::readMessage() {
    bool forMe = false;

    if (not isStartBit()) {
        return std::nullopt;
    }

    Message message{};

    message.broadcast = readByte(1);

    {
        m_parityBit = 0;

        auto const master1 = readByte(4);
        auto const master2 = readByte(8);

        message.master = (master2 << 8) | master1;

        if ((m_parityBit & 1) != readByte(1)) {
            return std::nullopt;
        }
    }

    {
        m_parityBit = 0;

        auto const slave1 = readByte(4);
        auto const slave2 = readByte(8);

        if ((m_parityBit & 1) != readByte(1)) {
            return std::nullopt;
        }

        message.slave = (slave2 << 8) | slave1;

        if ((slave1 == CD_ID_1) && (slave2 == CD_ID_2)) {
            m_forMe = true;
        }

        if (m_forMe) {
            sendACK();
        } else {
            auto const ackBit = readByte(1);
        }
    }

    {
        m_parityBit = 0;

        message.control = readByte(4);    // control - always 0xF

        if ((m_parityBit & 1) != readByte(1)) {
            return std::nullopt;
        }

        if (m_forMe) {
            sendACK();
        } else {
            auto const ackBit = readByte(1);
        }
    }

    {
        m_parityBit = 0;

        message.length = readByte(8);

        if ((m_parityBit & 1) != readByte(1)) {
            return std::nullopt;
        }

        if (m_forMe) {
            sendACK();
        } else {
            auto const ackBit = readByte(1);
        }

        if (message.length > MAX_MESSAGE_LEN) {
            return std::nullopt;
        }
    }

    for (auto i = 0; i < message.length; i++) {
        m_parityBit = 0;

        message.data[i] = readByte(8);

        if ((m_parityBit & 1) != readByte(1)) {
            return std::nullopt;
        }

        if (m_forMe) {
            sendACK();
        } else {
            auto const ackBit = readByte(1);
        }
    }

    return message;
}

AVC::Byte AVC::readByte(BitCount const bitCount) {
    AVC::Byte byte = 0;

    for (BitCount bitsRemaining = bitCount; bitsRemaining > 0; --bitsRemaining) {
        while (m_avcLine->isInputClear());

        std::uint64_t const pulseStart = time_us_64();

        while (m_avcLine->isInputSet());

        std::uint64_t const pulseWidth = time_us_64() - pulseStart;

        bool bitValue = false;

        if (pulseWidth < 8) {
            bitValue = true;
            m_parityBit++;
        }

        byte = (byte << 1) | (bitValue ? 0x01 : 0x00);
    }

    return byte;
}

bool AVC::readACK() const {
    std::uint64_t pulseWidth = 0;

    m_avcLine->outputSet();
    busy_wait_us(19);

    m_avcLine->outputReset();

    std::uint64_t const pulseStart = time_us_64();

    while (true) {
        pulseWidth = time_us_64() - pulseStart;

        if (m_avcLine->isInputSet() and (pulseWidth > 1)) {
            break;
        }

        if (pulseWidth > 5) {
            return true;
        }
    }

    while (m_avcLine->isInputSet());

    return false;
}

bool AVC::isStartBit() const {
    std::uint64_t pulseWidth = 0;

    std::uint64_t const pulseStart = time_us_64();

    while (m_avcLine->isInputSet()) {
        pulseWidth = time_us_64() - pulseStart;

        if (pulseWidth > 400) {
            return false;
        }
    }

    if (pulseWidth < 20) {
        return false;
    }

    return true;
}

void AVC::sendStartBit() const {
    m_avcLine->outputSet();
    busy_wait_us(166); // 141-192

    m_avcLine->outputReset();
    busy_wait_us(30); // 02-57
}

void AVC::sendBit0() const {
    m_avcLine->outputSet();
    busy_wait_us(32); // 28-37

    m_avcLine->outputReset();
    busy_wait_us(4); // 00-09
}

void AVC::sendBit1() const {
    m_avcLine->outputSet();
    busy_wait_us(20); // 17-23

    m_avcLine->outputReset();
    busy_wait_us(16); // 12-21
}

void AVC::sendACK() const {
    std::uint64_t const pulseStart = time_us_64();

    while (m_avcLine->isInputClear()) {
        std::uint64_t const pulseWidth = time_us_64() - pulseStart;
        if (pulseWidth >= 25) {
            return;    // max wait time
        }
    }

    m_avcLine->outputSet();
    busy_wait_us(32); //28-37

    m_avcLine->outputReset();
    busy_wait_us(4); //00-09
}
