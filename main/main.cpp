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

#include <freertos/FreeRTOS.h>
#include <iebus/Controller.hpp>
#include <iebus/Driver.hpp>
#include <iebus/Processor.hpp>
#include <iebus/common.hpp>

#include "MessageParser.hpp"
#include "USB.hpp"

namespace {

auto constexpr TAG = "Carlink";

auto constexpr IE_BUS_RX          = 8;
auto constexpr IE_BUS_TX          = 3;
auto constexpr IE_BUS_ENABLE      = 9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x150;

struct Context {
  QueueHandle_t messageErrorQueue = nullptr;
  QueueHandle_t messagePrintQueue = nullptr;
};

} // namespace

auto busWorker(void* arg) -> void {
  auto const context = static_cast<Context*>(arg);

  auto queue      = xQueueCreate(100, sizeof(iebus::Message));
  auto driver     = iebus::Driver(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE);
  auto processor  = iebus::Processor(IE_BUS_DEVICE_ADDR);
  auto controller = iebus::Controller(driver, IE_BUS_DEVICE_ADDR);

  driver.enable();

  while (driver.isEnabled()) {
    auto const readResult = controller.readMessage();

    if (readResult) {
      auto const message = (*readResult);

      xQueueSend(context->messagePrintQueue, &message, 0);

      for (auto const& answerMessage : processor.processMessage(message)) {
        xQueueSend(queue, &answerMessage, 0);
      }

    } else {
      auto const error = readResult.error();
      if (error >= iebus::MessageError::BROADCAST_BIT_READ_ERROR) {
        xQueueSend(context->messageErrorQueue, &error, 0);
      }
    }

    if (driver.isBusFree()) {
      iebus::Message message = {};

      if (xQueueReceive(queue, &message, 0) == pdTRUE) {
        auto const writeResult = controller.writeMessage(message);
        if (writeResult) {
          xQueueSend(context->messagePrintQueue, &message, 0);
        } else {
          xQueueSend(context->messageErrorQueue, &writeResult.error(), 0);
        }
      }
    }
  }
}

[[noreturn]] auto messageProcess(void* arg) -> void {
  auto const context = static_cast<Context*>(arg);

  iebus::Message message = {};

  while (true) {
    if (xQueueReceive(context->messagePrintQueue, &message, portMAX_DELAY) == pdTRUE) {
      iebus::common::printMessage(message);
    }
  }
}

[[noreturn]] auto messageErrorProcess(void* arg) -> void {
  auto const context = static_cast<Context*>(arg);

  iebus::MessageError messageError = {};

  while (true) {
    if (xQueueReceive(context->messageErrorQueue, &messageError, portMAX_DELAY) == pdTRUE) {
      iebus::common::printMessageError(messageError);
    }
  }
}

extern "C" void app_main() {
  Context static context = {
      .messageErrorQueue = xQueueCreate(10000, sizeof(iebus::MessageError)),
      .messagePrintQueue = xQueueCreate(100, sizeof(iebus::Message)),
  };

  xTaskCreatePinnedToCore(messageProcess, "message_process", 8192, &context, 10, nullptr, 0);
  xTaskCreatePinnedToCore(messageErrorProcess, "message_error_process", 8192, &context, 10, nullptr, 0);

  xTaskCreatePinnedToCore(busWorker, "ie_bus_worker", 8192, &context, 24, nullptr, 1);
}
