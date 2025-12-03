//
// Created by jadjer on 30.11.2025.
//

#include <esp_log.h>

#include "ie_bus/Controller.hpp"
#include "ie_bus/Driver.hpp"

auto constexpr TAG             = "CarLink";
auto constexpr BUS_RX          = 8;
auto constexpr BUS_TX          = 3;
auto constexpr BUS_ENABLE      = 9;
auto constexpr OUTPUT_ENABLE   = 6;
// auto constexpr BUS_DEVICE_ADDR = 0x140;
auto constexpr BUS_DEVICE_ADDR = 0x940;

auto errorToString(MessageError const error) {
    switch (error) {
    case MessageError::BUS_READ_ERROR:
        return "BUS_READ_ERROR";
    case MessageError::BIT_SEQUENCE_ERROR:
        return "BIT_SEQUENCE_ERROR";
    case MessageError::BIT_PARITY_ERROR:
        return "BIT_PARITY_ERROR";
    case MessageError::BITS_COUNT_ERROR:
        return "BITS_COUNT_ERROR";
    case MessageError::START_BIT_READ_ERROR:
        return "START_BIT_READ_ERROR";
    case MessageError::BROADCAST_BIT_READ_ERROR:
        return "BROADCAST_BIT_READ_ERROR";
    case MessageError::MASTER_ADDRESS_READ_ERROR:
        return "MASTER_ADDRESS_READ_ERROR";
    case MessageError::SLAVE_ADDRESS_READ_ERROR:
        return "SLAVE_ADDRESS_READ_ERROR";
    case MessageError::CONTROL_DATA_READ_ERROR:
        return "CONTROL_DATA_READ_ERROR";
    case MessageError::DATA_LENGTH_READ_ERROR:
        return "DATA_LENGTH_READ_ERROR";
    case MessageError::DATA_READ_ERROR:
        return "DATA_READ_ERROR";
    case MessageError::ACKNOWLEDGE_ERROR:
        return "ACKNOWLEDGE_ERROR";
    default:
        return "UNKNOWN_ERROR";
    }
}

extern "C" [[noreturn]] void app_main() {
    Controller controller(BUS_RX, BUS_TX, BUS_ENABLE, BUS_DEVICE_ADDR);
    controller.enable();

    while (true) {
        auto const optionalMessage = controller.readMessage();
        if (optionalMessage.has_value()) {
            auto const message = optionalMessage.value();

            ESP_LOGI(TAG, "%01X %03X %03X %01X %d", message.broadcast, message.master, message.slave, message.control,
                message.dataLength);
            ESP_LOG_BUFFER_HEX(TAG, message.data, message.dataLength);
            ESP_LOGI(TAG, "--------------");
        }
    }
}
