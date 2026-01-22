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

#pragma once

#include <cstdint>
#include <esp_twai_types.h>

class CAN {
public:
  using Pin = std::uint8_t;
  using Node = twai_node_handle_t;

public:
  CAN(Pin rx, Pin tx) noexcept;
  ~CAN() noexcept;

private:
  Pin const m_rxPin;
  Pin const m_txPin;

private:
  Node m_node = nullptr;
};
