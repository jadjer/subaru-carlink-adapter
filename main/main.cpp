// Copyright 2025 Pavel Suprunov
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
#include <iebus/Message.hpp>

namespace {

auto constexpr TAG = "CarLink";
auto constexpr IE_BUS_RX = 8;
auto constexpr IE_BUS_TX = 3;
auto constexpr IE_BUS_ENABLE = 9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x140;

iebus::Controller mediaController(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE, IE_BUS_DEVICE_ADDR);

QueueHandle_t messageQueue;

void busWorker(void* pvParameters) {
  while (true) {
    auto const optionalMessage = mediaController.readMessage();
    if (not optionalMessage) {
      continue;
    }

    auto const message = optionalMessage.value();

    auto const sendResult = xQueueSend(messageQueue, &message, pdMS_TO_TICKS(100));
    if (sendResult != pdPASS) {
      ESP_LOGE(TAG, "Queue is full");
    }
  }
}

void messageProcessWorker(void* pvParameters) {
  iebus::Message message;

  while (true) {
    if (xQueueReceive(messageQueue, &message, portMAX_DELAY)) {
      ESP_LOGI(TAG, "%s", message.toString().c_str());
    }
  }
}

} // namespace

extern "C" void app_main() {
  mediaController.enable();

  messageQueue = xQueueCreate(100, sizeof(iebus::Message));

  if (messageQueue) {
    xTaskCreatePinnedToCore(busWorker, "bus_worker", 2048, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(messageProcessWorker, "message_process_worker", 2048, nullptr, 5, nullptr, 1);
  }
}
