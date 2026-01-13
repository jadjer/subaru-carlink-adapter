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
auto constexpr IE_BUS_DEVICE_ADDR = 0x540;

auto constexpr QUEUE_MAX_SIZE = 100;

} // namespace

QueueHandle_t messageQueueHandle = nullptr;

[[noreturn]] auto mediaWorker(void*) -> void {
  iebus::Driver mediaDriver(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE);
  iebus::Controller mediaController(mediaDriver, IE_BUS_DEVICE_ADDR);

  mediaDriver.enable();

  vTaskDelay(pdMS_TO_TICKS(100));

  while (true) {
    auto const optionalMessage = mediaController.readMessage();
    if (optionalMessage.has_value()) {
      auto const message = optionalMessage.value();
      xQueueSend(messageQueueHandle, &message, 0);
    }
  }
}

[[noreturn]] auto messageWorker(void*) -> void {
  while (true) {
    iebus::Message message = {};

    if (xQueueReceive(messageQueueHandle, &message, portMAX_DELAY) == pdTRUE) {
      ESP_LOGI(TAG, "%s", message.toString().c_str());
    }
  }
}

extern "C" void app_main() {
  messageQueueHandle = xQueueCreate(QUEUE_MAX_SIZE, sizeof(iebus::Message));

  xTaskCreatePinnedToCore(messageWorker, "message_worker", 8096, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(mediaWorker, "media_worker", 8096, nullptr, 10, nullptr, 1);
}
