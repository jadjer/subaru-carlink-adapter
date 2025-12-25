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
#include <freertos/task.h>
#include <iebus/Controller.hpp>

#include "MessageParser.hpp"
#include "USB.hpp"

namespace {

auto constexpr TAG = "CarLink";
auto constexpr IE_BUS_RX = 8;
auto constexpr IE_BUS_TX = 3;
auto constexpr IE_BUS_ENABLE = 9;
auto constexpr IE_BUS_DEVICE_ADDR = 0x140;

} // namespace

extern "C" [[noreturn]] void app_main() {
  USB usb;
  iebus::Controller mediaController(IE_BUS_RX, IE_BUS_TX, IE_BUS_ENABLE, IE_BUS_DEVICE_ADDR);

  while (true) {
    bool const isUsbConnected = usb.isConnected();

    if (isUsbConnected != mediaController.isEnabled()) {
      isUsbConnected ? mediaController.enable() : mediaController.disable();
    }

    if (not isUsbConnected) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (auto const msg = mediaController.readMessage()) {
      ESP_LOGI(TAG, "%s", msg->toString().c_str());

      switch (messageParse(*msg)) {
      case Command::PLAY:
        usb.play();
        break;
      case Command::MUTE:
        usb.mute();
        break;
      case Command::VOLUME_UP:
        usb.volumeUp();
        break;
      case Command::VOLUME_DOWN:
        usb.volumeDown();
        break;
      case Command::TRACK_NEXT:
        usb.trackNext();
        break;
      case Command::TRACK_PREV:
        usb.trackPrev();
        break;
      default:
        break;
      }
    }
  }
}
