
#include <cstdio>
#include <pico/stdlib.h>
#include <pico/status_led.h>

#include "avc.hpp"


[[noreturn]] int main() {
    stdio_init_all();

    status_led_init();

    AVC avc;

    status_led_set_state(true);

    std::uint64_t ledDelayStart = 0;

    while (true) {
        auto const message = avc.readMessage();
        if (message.has_value()) {
            status_led_set_state(false);
            ledDelayStart = time_us_64();

            printf("B: 0x%01X, M: 0x%03X, S: 0x%03X, C: 0x%03X, L: 0x%03X\n",
                   message->broadcast,
                   message->master,
                   message->slave,
                   message->control,
                   message->length);
        }

        if (not status_led_get_state()) {
            auto const ledDelay = time_us_64() - ledDelayStart;
            if (ledDelay >= 500000) {
                status_led_set_state(true);
            }
        }
    }
}
