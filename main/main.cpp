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

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <iebus/Controller.hpp>
#include <iebus/Driver.hpp>

#include "MessageParser.hpp"
#include "USB.hpp"

namespace {

auto constexpr IE_BUS_RX = GPIO_NUM_8;
auto constexpr IE_BUS_TX = GPIO_NUM_3;
auto constexpr IE_BUS_ENABLE = GPIO_NUM_9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x540;

auto constexpr QUEUE_MAX_SIZE = 100;

struct MediaContext {
  USB& usb;
  iebus::Driver& driver;
  iebus::Controller& controller;
  QueueHandle_t queue;
};

USB usb;
iebus::Driver driver(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE);
iebus::Controller controller(driver, IE_BUS_DEVICE_ADDR);
QueueHandle_t queue = nullptr;

} // namespace

[[noreturn]] auto mediaWorker(void* pvParameters) -> void {
  auto const context = static_cast<MediaContext*>(pvParameters);

  context->driver.enable();

  while (true) {
    auto const optionalMessage = context->controller.readMessage();
    if (optionalMessage.has_value()) {
      auto const message = optionalMessage.value();
      xQueueSend(context->queue, &message, 0);
    }
  }
}

[[noreturn]] auto messageWorker(void* pvParameters) -> void {
  auto const context = static_cast<MediaContext*>(pvParameters);

  iebus::Message message = {};

  while (true) {
    if (xQueueReceive(context->queue, &message, portMAX_DELAY) == pdTRUE) {
      iebus::printMessage(message);

//      messageParse(message);
    }
  }
}

extern "C" void app_main() {
  queue = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::Message));
  if (queue == nullptr) {
    return;
  }

  static MediaContext context{usb, driver, controller, queue};

  xTaskCreatePinnedToCore(messageWorker, "message_worker", 8096, &context, 1, nullptr, 0);
  xTaskCreatePinnedToCore(mediaWorker, "media_worker", 8096, &context, 10, nullptr, 1);
}
