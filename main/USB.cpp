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
// Created by jadjer on 18.12.2025.
//

#include "USB.hpp"

#include <array>
#include <class/hid/hid.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <string_view>
#include <tinyusb.h>
#include <tinyusb_default_config.h>

namespace {

auto constexpr TAG = "USB";
auto constexpr POLLING_INTERVAL_MS = 10;
auto constexpr USB_DESC_TOTAL_LEN = TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN;

auto constexpr HID_REPORT_DESCRIPTOR = std::to_array<std::uint8_t>({
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
});

auto constexpr HID_CONFIGURATION_DESCRIPTOR = std::to_array<uint8_t>({
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, USB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(HID_REPORT_DESCRIPTOR), 0x81, 16, POLLING_INTERVAL_MS),
});

auto HID_STRING_DESCRIPTION = std::to_array<char const*>({
    "\x09\x04",               // 0: Supported language is English (0x0409)
    "JADJER",                 // 1: Manufacturer
    "Subaru Carlink Adapter", // 2: Product
    "000001",                 // 3: Serials, should use chip ID
    "Media HID interface"     // 4: HID
});

} // namespace

auto tud_hid_descriptor_report_cb(uint8_t) -> uint8_t const* {
  return HID_REPORT_DESCRIPTOR.data();
}

auto tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t*, uint16_t) -> uint16_t {
  return 0;
}

auto tud_hid_set_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t const*, uint16_t) -> void {
}

USB::USB() {
  auto usbConfiguration = TINYUSB_DEFAULT_CONFIG();
  usbConfiguration.descriptor.device = nullptr;
  usbConfiguration.descriptor.string = HID_STRING_DESCRIPTION.data();
  usbConfiguration.descriptor.string_count = HID_STRING_DESCRIPTION.size();
  usbConfiguration.descriptor.full_speed_config = HID_CONFIGURATION_DESCRIPTOR.data();

  auto const isDriverInstalled = tinyusb_driver_install(&usbConfiguration);
  if (isDriverInstalled != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install USB driver");
    return;
  }
}

auto USB::isConnected() const -> bool {
  return tud_mounted();
}

auto USB::sendKey(std::uint16_t const key) const -> bool {
  if (not isConnected()) {
    ESP_LOGW(TAG, "USB not connected to host device");
    return false;
  }

  auto const isPressed = tud_hid_report(HID_ITF_PROTOCOL_KEYBOARD, &key, 2);
  if (not isPressed) {
    ESP_LOGE(TAG, "Error press key");
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL_MS));

  auto const emptyKey = HID_USAGE_CONSUMER_UNASSIGNED;

  auto const isReleased = tud_hid_report(HID_ITF_PROTOCOL_KEYBOARD, &emptyKey, 2);
  if (not isReleased) {
    ESP_LOGE(TAG, "Error release key");
    return false;
  }

  return true;
}

auto USB::play() -> bool {
  ESP_LOGI(TAG, "play");

  return sendKey(HID_USAGE_CONSUMER_PLAY_PAUSE);
}

auto USB::mute() -> bool {
  ESP_LOGI(TAG, "mute");

  return sendKey(HID_USAGE_CONSUMER_MUTE);
}

auto USB::volumeUp() -> bool {
  ESP_LOGI(TAG, "volumeUp");

  return sendKey(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
}

auto USB::volumeDown() -> bool {
  ESP_LOGI(TAG, "volumeDown");

  return sendKey(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
}

auto USB::trackNext() -> bool {
  ESP_LOGI(TAG, "trackNext");

  return sendKey(HID_USAGE_CONSUMER_SCAN_NEXT_TRACK);
}

auto USB::trackPrev() -> bool {
  ESP_LOGI(TAG, "trackPrev");

  return sendKey(HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK);
}
