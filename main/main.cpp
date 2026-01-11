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

namespace {

auto constexpr TAG = "CarLink";
auto constexpr IE_BUS_RX = 8;
auto constexpr IE_BUS_TX = 3;
auto constexpr IE_BUS_ENABLE = 9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x140;

} // namespace

QueueHandle_t errorQueueHandle = nullptr;
QueueHandle_t messageQueueHandle = nullptr;

[[noreturn]] auto mediaWorker(void* pvParameters) -> void {
  (void)pvParameters;

  iebus::Controller mediaController(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE, IE_BUS_DEVICE_ADDR);
  mediaController.enable();

  while (true) {
    auto const expectedMessage = mediaController.readMessage();

    if (expectedMessage.has_value()) {
      auto const message = expectedMessage.value();
      xQueueSend(messageQueueHandle, &message, 1);

    } else {
      auto const error = expectedMessage.error();
      xQueueSend(errorQueueHandle, &error, 1);
    }
  }
}

[[noreturn]] auto messageWorker(void* pvParameters) -> void {
  (void)pvParameters;

  while (true) {
    iebus::Message message = {};

    if (xQueueReceive(messageQueueHandle, &message, portMAX_DELAY) == pdTRUE) {
      ESP_LOGI(TAG, "%s", message.toString().c_str());
    }
  }
}

[[noreturn]] auto errorWorker(void* pvParameters) -> void {
  (void)pvParameters;

  while (true) {
    iebus::MessageError error = {};

    if (xQueueReceive(errorQueueHandle, &error, portMAX_DELAY) == pdTRUE) {
      ESP_LOGE(TAG, "%u", error);
    }
  }
}

extern "C" void app_main() {
  errorQueueHandle = xQueueCreate(100, sizeof(iebus::MessageError));
  messageQueueHandle = xQueueCreate(100, sizeof(iebus::Message));

  xTaskCreate(messageWorker, "message_worker", 4096, nullptr, 1, nullptr);
  xTaskCreate(mediaWorker, "media_worker", 4096, nullptr, 5, nullptr);
  xTaskCreate(errorWorker, "error_worker", 4096, nullptr, 5, nullptr);
}
