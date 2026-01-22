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
#include <vector>

class USB {
public:
  using Bit  = std::uint8_t;
  using Port = std::uint8_t;
  using Size = std::size_t;
  using Bits = std::vector<Bit>;

public:
  USB(Port port) noexcept;
  ~USB() noexcept;

public:
  [[nodiscard]] auto isConnected() const noexcept -> bool;

public:
  auto read() const noexcept -> void;
  auto write(Bits bits) const noexcept -> void;

private:
  Port m_port = 0;
};
