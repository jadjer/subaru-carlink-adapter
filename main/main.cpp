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
#include <print>

#include "MessageParser.hpp"
#include "USB.hpp"

namespace {

auto constexpr TAG = "Carlink";

auto constexpr IE_BUS_RX          = 8;
auto constexpr IE_BUS_TX          = 3;
auto constexpr IE_BUS_ENABLE      = 9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x540;

QueueHandle_t messageQueue = xQueueCreate(100, sizeof(iebus::Message));

} // namespace

auto busWorker(void*) -> void {
  auto const driver     = iebus::Driver(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE);
  auto const controller = iebus::Controller(driver, IE_BUS_DEVICE_ADDR);

  auto processor = iebus::Processor(IE_BUS_DEVICE_ADDR);

  driver.enable();

  while (driver.isEnabled()) {
    auto const optMessage = controller.readMessage();
    if (optMessage) {
      auto const message = *optMessage;
      xQueueSend(messageQueue, &message, 0);

      for (auto const& answerMessage : processor.processMessage(message)) {
        if (controller.writeMessage(answerMessage)) {
          xQueueSend(messageQueue, &answerMessage, 0);
        }
      }
    }
  }
}

[[noreturn]] auto messageProcessWorker(void*) -> void {
  iebus::Message message = {};

  while (true) {
    if (xQueueReceive(messageQueue, &message, portMAX_DELAY) == pdTRUE) {
      iebus::common::printMessage(message);
    }
  }
}

extern "C" void app_main() {
  xTaskCreatePinnedToCore(messageProcessWorker, "message_process", 4096, nullptr, 10, nullptr, 0);
  xTaskCreatePinnedToCore(busWorker, "ie_bus_worker", 16384, nullptr, 24, nullptr, 1);
}
