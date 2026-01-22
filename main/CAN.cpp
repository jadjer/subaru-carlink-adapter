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
// Created by jadjer on 22.01.2026.
//

#include "CAN.hpp"

#include <esp_twai.h>
#include <esp_twai_onchip.h>

CAN::CAN(CAN::Pin rx, CAN::Pin tx) noexcept : m_rxPin(rx), m_txPin(tx) {
  twai_onchip_node_config_t const nodeConfiguration = {
      .io_cfg         = {.tx = static_cast<gpio_num_t>(m_txPin), .rx = static_cast<gpio_num_t>(m_rxPin), .quanta_clk_out = GPIO_NUM_NC, .bus_off_indicator = GPIO_NUM_NC},
      .clk_src        = TWAI_CLK_SRC_DEFAULT,
      .bit_timing     = {.bitrate = 200000, .sp_permill = 0, .ssp_permill = 0},
      .data_timing    = {.bitrate = 200000, .sp_permill = 0, .ssp_permill = 0},
      .fail_retry_cnt = 10,
      .tx_queue_depth = 100,
      .intr_priority  = 0,
      .flags          = {.enable_self_test = true, .enable_loopback = true, .enable_listen_only = false, .no_receive_rtr = true}
  };
  ESP_ERROR_CHECK(twai_new_node_onchip(&nodeConfiguration, &m_node));
  ESP_ERROR_CHECK(twai_node_enable(m_node));
}

CAN::~CAN() noexcept {
  ESP_ERROR_CHECK(twai_node_disable(m_node));
  ESP_ERROR_CHECK(twai_node_delete(m_node));
}
