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

auto constexpr IE_BUS_RX          = GPIO_NUM_8;
auto constexpr IE_BUS_TX          = GPIO_NUM_3;
auto constexpr IE_BUS_ENABLE      = GPIO_NUM_9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x540;

auto constexpr QUEUE_MAX_SIZE = 100;

struct Context {
  QueueHandle_t errorQueue;
  QueueHandle_t writeQueue;
  QueueHandle_t messageQueue;

  Context() {
    errorQueue   = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::MessageError));
    writeQueue   = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::Message));
    messageQueue = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::Message));

    if (not errorQueue or not messageQueue) {
      ESP_LOGE(TAG, "Failed to create queues!");
    }
  }
};

Context* ctx = nullptr;

} // namespace

[[noreturn]] auto busWorker(void* pvParameters) -> void {
  auto const& context = *static_cast<Context*>(pvParameters);

  iebus::Driver driver         = {IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE};
  iebus::Controller controller = {driver, IE_BUS_DEVICE_ADDR};
  iebus::Processor processor   = {IE_BUS_DEVICE_ADDR};

  driver.enable();

  while (true) {
    auto const readResult = controller.readMessage();

    if (readResult.has_value()) {
      auto const requestMessage = readResult.value();
      xQueueSend(context.messageQueue, &requestMessage, 0);

      auto const responseMessages = processor.processMessage(requestMessage);
      for (auto const& responseMessage : responseMessages) {
        xQueueSend(context.writeQueue, &responseMessage, 0);
      }

    } else {
      auto const error = readResult.error();
      if (error >= iebus::MessageError::BROADCAST_BIT_READ_ERROR) {
        xQueueSend(context.errorQueue, &error, 0);
      }
    }

    iebus::Message message = {};

    if (xQueueReceive(context.writeQueue, &message, 0)) {
      auto const writeResult = controller.writeMessage(message);

      if (writeResult.has_value()) {
        xQueueSend(context.messageQueue, &message, 0);

      } else {
        xQueueSendToFront(context.writeQueue, &message, 0);

        auto const error = writeResult.error();
        if (error >= iebus::MessageError::BROADCAST_BIT_READ_ERROR) {
          xQueueSend(context.errorQueue, &error, 0);
        }
      }
    }
  }
}

[[noreturn]] auto messageErrorPrintWorker(void* pvParameters) -> void {
  auto const& context = *static_cast<Context*>(pvParameters);

  iebus::MessageError messageError = {};

  while (true) {
    if (xQueueReceive(context.errorQueue, &messageError, portMAX_DELAY) == pdTRUE) {
      iebus::common::printMessageError(messageError);
    }
  }
}

[[noreturn]] auto messageProcessWorker(void* pvParameters) -> void {
  auto const& context = *static_cast<Context*>(pvParameters);

//  USB usb                = {0};
  iebus::Message message = {};

  while (true) {
    if (xQueueReceive(context.messageQueue, &message, portMAX_DELAY) == pdTRUE) {
      iebus::common::printMessage(message);

//      usb.write({0x00, 0x12, 0x32, 0x44});

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
  ctx = new Context();

  xTaskCreatePinnedToCore(messageErrorPrintWorker, "message_error", 4096, ctx, 1, nullptr, 0);
  xTaskCreatePinnedToCore(messageProcessWorker, "message_process", 4096, ctx, 2, nullptr, 0);

  xTaskCreatePinnedToCore(busWorker, "ie_bus_worker", 4096, ctx, 24, nullptr, 1);
}
