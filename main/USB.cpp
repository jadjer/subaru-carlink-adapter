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
#include <esp_log.h>
#include <string_view>
#include <tinyusb.h>

#include <tinyusb_default_config.h>

namespace {

auto constexpr TAG = "USB";

auto HID_STRING_DESCRIPTION = std::to_array<char const*>({
    "\x09\x04",               // 0: Supported language is English (0x0409)
    "JADJER",                 // 1: Manufacturer
    "Subaru Carlink Adapter", // 2: Product
    "000001",                 // 3: Serials, should use chip ID
    "Media HID interface"     // 4: HID
});

} // namespace

USB::USB() : m_port(TINYUSB_CDC_ACM_0) {
  auto usbConfiguration                         = TINYUSB_DEFAULT_CONFIG();
  usbConfiguration.descriptor.device            = nullptr;
  usbConfiguration.descriptor.string            = HID_STRING_DESCRIPTION.data();
  usbConfiguration.descriptor.string_count      = HID_STRING_DESCRIPTION.size();
  usbConfiguration.descriptor.full_speed_config = nullptr;
  ESP_ERROR_CHECK(tinyusb_driver_install(&usbConfiguration));

  tinyusb_config_cdcacm_t const acmConfiguration = {
      .cdc_port                     = m_port,
      .callback_rx                  = nullptr,
      .callback_rx_wanted_char      = nullptr,
      .callback_line_state_changed  = nullptr,
      .callback_line_coding_changed = nullptr,
  };
  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acmConfiguration));
}

auto USB::isConnected() const -> bool {
  return tud_mounted();
}

auto USB::read() -> void {
  Bits buffer;
  buffer.reserve(100);

  Size dataSize = 0;

  tinyusb_cdcacm_read(m_port, buffer.data(), buffer.size(), &dataSize);

  ESP_LOGI(TAG, "Read %d bits", dataSize);
}

auto USB::write(Bits bits) -> void {
  auto const writtenBits = tinyusb_cdcacm_write_queue(m_port, bits.data(), bits.size());
  if (writtenBits != bits.size()) {
    ESP_LOGE(TAG, "Not all the bits were written");
  }
  tinyusb_cdcacm_write_flush(m_port, 0);
}
