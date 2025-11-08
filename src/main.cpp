#include "pico/stdlib.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

#ifndef PICO_DEFAULT_LED_PIN
#warning blink_simple example requires a board with a regular LED
#endif

void led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void data_init() {

}

void pico_set_led(bool led_on) {
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

int main() {
    pico_led_init();
    while (true) {
        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
    }
}
