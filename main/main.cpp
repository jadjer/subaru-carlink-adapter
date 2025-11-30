//
// Created by jadjer on 30.11.2025.
//

#include <esp_log.h>

#include "ie_bus/Controller.hpp"

auto constexpr TAG             = "CarLink";
auto constexpr BUS_RX          = 8;
auto constexpr BUS_TX          = 3;
auto constexpr BUS_ENABLE      = 9;
auto constexpr OUTPUT_ENABLE   = 6;
auto constexpr BUS_DEVICE_ADDR = 0x997;

extern "C" [[noreturn]] void app_main() {
    Controller controller(BUS_RX, BUS_TX, BUS_ENABLE, BUS_DEVICE_ADDR);
    controller.enable();

    while (true) {
        auto const message = controller.readMessage();
        if (message) {
            ESP_LOGI(TAG, "%s - %d - %d - %d - %d", message->isBroadcast ? "0" : "1", message->master, message->slave,
                message->control, message->dataLength);
            ESP_LOG_BUFFER_HEX(TAG, message->data, message->dataLength);
        } else {
            ESP_LOGE(TAG, "Read error");
        }
    }
}
