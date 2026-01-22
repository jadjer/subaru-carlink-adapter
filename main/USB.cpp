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
#include <tinyusb_cdc_acm.h>

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

USB::USB(Port port) noexcept : m_port(port) {
  tinyusb_config_t const usbConfiguration = {
      .port       = TINYUSB_PORT_FULL_SPEED_0,
      .phy        = {.skip_setup = false, .self_powered = false, .vbus_monitor_io = 0},
      .task       = {.size = 4096, .priority = 10, .xCoreID = 0},
      .descriptor = {.device            = nullptr,
                     .qualifier         = nullptr,
                     .string            = HID_STRING_DESCRIPTION.data(),
                     .string_count      = HID_STRING_DESCRIPTION.size(),
                     .full_speed_config = nullptr,
                     .high_speed_config = nullptr},
      .event_cb   = nullptr,
      .event_arg  = this
  };
  ESP_ERROR_CHECK(tinyusb_driver_install(&usbConfiguration));

  tinyusb_config_cdcacm_t const acmConfiguration = {
      .cdc_port                     = static_cast<tinyusb_cdcacm_itf_t>(m_port),
      .callback_rx                  = nullptr,
      .callback_rx_wanted_char      = nullptr,
      .callback_line_state_changed  = nullptr,
      .callback_line_coding_changed = nullptr,
  };
  ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acmConfiguration));
}

USB::~USB() noexcept {
  ESP_ERROR_CHECK(tinyusb_cdcacm_deinit(TINYUSB_CDC_ACM_0));
  ESP_ERROR_CHECK(tinyusb_driver_uninstall());
}

auto USB::isConnected() const noexcept -> bool {
  return tud_mounted();
}

auto USB::read() noexcept -> void {
  Bits buffer;
  buffer.reserve(100);

  Size dataSize = 0;

  tinyusb_cdcacm_read(static_cast<tinyusb_cdcacm_itf_t>(m_port), buffer.data(), buffer.size(), &dataSize);

  ESP_LOGI(TAG, "Read %d bits", dataSize);
}

auto USB::write(Bits bits) noexcept -> void {
  auto const writtenBits = tinyusb_cdcacm_write_queue(static_cast<tinyusb_cdcacm_itf_t>(m_port), bits.data(), bits.size());
  if (writtenBits != bits.size()) {
    ESP_LOGE(TAG, "Not all the bits were written");
  }
  tinyusb_cdcacm_write_flush(static_cast<tinyusb_cdcacm_itf_t>(m_port), 0);
}
