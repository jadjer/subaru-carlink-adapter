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
// Created by jadjer on 17.01.2026.
//

#pragma once

#include <iebus/Message.hpp>
#include <vector>

namespace iebus {

enum class Command : Bit {
  COMMAND_10 = 0x10,
  COMMAND_11 = 0x11,
  COMMAND_12 = 0x12,
  COMMAND_13 = 0x13,
  COMMAND_20 = 0x20,
  COMMAND_40 = 0x40,
  COMMAND_60 = 0x60,
  COMMAND_70 = 0x70,
  COMMAND_D0 = 0xD0,
};

class DisplayProcessor {
public:
  using MessageList = std::vector<Message>;

public:
  DisplayProcessor(Address address) noexcept;

public:
  [[nodiscard]] auto processMessage(Message const& message) noexcept -> MessageList;

private:
  [[nodiscard]] auto handleCommand10(Message const& message) noexcept -> MessageList;

private:
  auto createCommandRestart(Message const& message) noexcept -> MessageList;
  auto createCommandConfiguration(Message const& message) noexcept -> MessageList ;

private:
  [[nodiscard]] auto checkMessageForMe(Message const& message) const -> bool;

private:
  [[nodiscard]] auto createCommand(Address target, Command command, Size length, Bytes payload) const noexcept -> Message;
  [[nodiscard]] auto createBroadcastCommand(Command command, Size length, Bytes payload) const noexcept -> Message;

private:
  Address const m_address;

private:
  bool m_isStarted = false;
  bool m_isConfigured = false;
};

} // namespace iebus
