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

#pragma once

#include <cstdint>

class USB {
private:
  using Port = uint8_t;

public:
  USB();

public:
  [[nodiscard]] auto isConnected() const -> bool;

public:
  [[nodiscard]] auto sendKey(std::uint16_t key) const -> bool;

public:
  auto play() -> bool;

public:
  auto mute() -> bool;
  auto volumeUp() -> bool;
  auto volumeDown() -> bool;

public:
  auto trackNext() -> bool;
  auto trackPrev() -> bool;
};
