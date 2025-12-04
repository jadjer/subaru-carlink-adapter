//
// Created by jadjer on 30.11.2025.
//

#include <esp_log.h>

#include <iebus/Controller.hpp>

auto constexpr TAG             = "CarLink";
auto constexpr BUS_RX          = 8;
auto constexpr BUS_TX          = 3;
auto constexpr BUS_ENABLE      = 9;
auto constexpr OUTPUT_ENABLE   = 6;
// auto constexpr BUS_DEVICE_ADDR = 0x140;
auto constexpr BUS_DEVICE_ADDR = 0x940;

extern "C" [[noreturn]] void app_main() {
    iebus::Controller controller(BUS_RX, BUS_TX, BUS_ENABLE, BUS_DEVICE_ADDR);
    controller.enable();

    while (true) {
        auto const optionalMessage = controller.readMessage();
        if (optionalMessage.has_value()) {
            auto const message = optionalMessage.value();

            ESP_LOGI(TAG, "%01X %03X %03X %01X %d", message.broadcast, message.master, message.slave, message.control,
                message.dataLength);
            // ESP_LOG_BUFFER_HEX(TAG, message.data., message.dataLength);
            ESP_LOGI(TAG, "--------------");
        }
    }
}
