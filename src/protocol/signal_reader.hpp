//
// Created by jadjer on 13.11.25.
//

#ifndef AVC_PROTOCOL_SIGNAL_READER_HPP
#define AVC_PROTOCOL_SIGNAL_READER_HPP

enum class SignalLevel {
    UNKNOWN = -1,
    LEVEL_0 = 0,
    LEVEL_1 = 1,
};

class SignalReader {
public:
    SignalReader();

public:
    [[nodiscard]] SignalLevel getBusLevel();
};

#endif //AVC_PROTOCOL_SIGNAL_READER_HPP
