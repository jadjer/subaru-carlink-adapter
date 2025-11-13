
#include <cstdio>
#include <pico/stdlib.h>

#include "protocol_analyzer.hpp"


[[noreturn]] int main() {
    stdio_init_all();

    ProtocolAnalyzer analyzer;

    while (true) {
        analyzer.process();

        auto packet = analyzer.getDecodedPacket();
        if (packet.has_value()) {
            printf("0x%01X, 0x%03X, 0x%03X, 0x%03X, 0x%03X\n",
                   packet->broadcast,
                   packet->masterAddress,
                   packet->slaveAddress,
                   packet->control,
                   packet->dataLength);
        }

        busy_wait_us(1);
    }
}
