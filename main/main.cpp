// Copyright 2026 Pavel Suprunov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//
// Created by jadjer on 30.11.2025.
//

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <iebus/Controller.hpp>
#include <iebus/Driver.hpp>
#include <iebus/Processor.hpp>
#include <iebus/common.hpp>

#include "MessageParser.hpp"
#include "USB.hpp"

namespace {

auto constexpr TAG = "Carlink";

auto constexpr IE_BUS_RX = GPIO_NUM_8;
auto constexpr IE_BUS_TX = GPIO_NUM_3;
auto constexpr IE_BUS_ENABLE = GPIO_NUM_9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x540;

auto constexpr QUEUE_MAX_SIZE = 100;

struct Context {
  QueueHandle_t errorQueue = nullptr;
  QueueHandle_t messageQueue = nullptr;

  bool init() {
    errorQueue = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::MessageError));
    messageQueue = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::Message));

    return (errorQueue and messageQueue);
  }
};

Context ctx;

} // namespace

[[noreturn]] auto busWorker(void* pvParameters) -> void {
  auto& context = *static_cast<Context*>(pvParameters);

  iebus::Driver driver{IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE};
  iebus::Controller controller{driver, IE_BUS_DEVICE_ADDR};
  iebus::Processor processor{IE_BUS_DEVICE_ADDR};

  driver.enable();

  while (true) {
    auto const result = controller.readMessage();
    if (result) {
      auto const value = result.value();
      if (xQueueSend(context.messageQueue, &value, 0) != pdPASS) {
        ESP_LOGW(TAG, "Message queue is full");
      }

      auto const messages = processor.processMessage(value);
      for (auto const& message : messages) {
        if (xQueueSend(context.messageQueue, &message, 0) != pdPASS) {
          ESP_LOGW(TAG, "Message queue is full");
        }

        auto const isWritten = controller.writeMessage(message);
        if (not isWritten) {
          ESP_LOGE(TAG, "Couldn't write a message to the bus");
        }
      }
    } else {
      auto const error = result.error();
      if (error >= iebus::MessageError::BROADCAST_BIT_READ_ERROR) {
        if (xQueueSend(context.errorQueue, &error, 0) != pdPASS) {
          ESP_LOGW(TAG, "Error queue is full");
        }
      }
    }
  }
}

[[noreturn]] auto messageError(void* pvParameters) -> void {
  auto& context = *static_cast<Context*>(pvParameters);

  iebus::MessageError messageError = {};

  while (true) {
    if (xQueueReceive(context.errorQueue, &messageError, portMAX_DELAY) == pdTRUE) {
      iebus::printMessageError(messageError);
    }
  }
}

[[noreturn]] auto messageProcess(void* pvParameters) -> void {
  auto& context = *static_cast<Context*>(pvParameters);

  USB usb;
  iebus::Message message = {};

  while (true) {
    if (xQueueReceive(context.messageQueue, &message, portMAX_DELAY) == pdTRUE) {
      iebus::printMessage(message);

//      auto const action = parseMessageToUsbAction(message);
//      switch (action) {
//      case USB_ACTION_TRACK_PREV:
//        usb.trackPrev();
//        break;
//      case USB_ACTION_TRACK_NEXT:
//        usb.trackNext();
//        break;
//      case USB_ACTION_PLAY:
//        usb.play();
//        break;
//      case USB_ACTION_MUTE:
//        usb.mute();
//      }
    }
  }
}

extern "C" void app_main() {
  if (not ctx.init()) {
    ESP_LOGE(TAG, "Failed to create queues");
    return;
  }

  xTaskCreatePinnedToCore(messageError, "message_error", 4096, &ctx, 1, nullptr, 0);
  xTaskCreatePinnedToCore(messageProcess, "message_process", 4096, &ctx, 1, nullptr, 0);

  xTaskCreatePinnedToCore(busWorker, "bus_worker", 4096, &ctx, 10, nullptr, 1);
}
