// Copyright 2026 Pavel Suprunov
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

#include "DisplayProcessor.hpp"

#include <regex>

namespace iebus {

namespace {

Address constexpr BROADCAST_ADDRESS = 0xFFF;

} // namespace

DisplayProcessor::DisplayProcessor(Address const address) noexcept : m_address(address) {
}

auto DisplayProcessor::processMessage(Message const& message) noexcept -> DisplayProcessor::MessageList {
  if (not checkMessageForMe(message)) return {};
  if (message.length < 1) return {};

  auto const command = static_cast<Command>(message.data[0]);
  if (command == Command::COMMAND_10) return handleCommand10(message);

  if (not m_isStarted) return createCommandRestart(message);
  if (not m_isConfigured) return createCommandConfiguration(message);

  return {};
}

auto DisplayProcessor::handleCommand10(Message const& message) noexcept -> DisplayProcessor::MessageList {
  if (message.length < 2) return {};

  m_isStarted    = true;
  m_isConfigured = false;

  return {createCommand(message.master, Command::COMMAND_11, 5, {message.data[1], 0x01, 0x02, 0x85, 0x93})};
}

auto DisplayProcessor::createCommandRestart(Message const& message) noexcept -> DisplayProcessor::MessageList {
  return {createBroadcastCommand(Command::COMMAND_12, 0, {})};
}

auto DisplayProcessor::createCommandConfiguration(const Message& message) noexcept -> DisplayProcessor::MessageList {
  m_isConfigured = true;

  return {
      createCommand(message.master, Command::COMMAND_40, 3, {0xC0, 0x20, 0x02}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x02, 0x10}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x06, 0x10}),
      createCommand(message.master, Command::COMMAND_40, 2, {0xC0, 0x10}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x83, 0x10}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x02, 0x00}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x06, 0x00}),
      createCommand(message.master, Command::COMMAND_40, 2, {0xC0, 0x00}),
      createCommand(message.master, Command::COMMAND_40, 2, {0x83, 0x00}),
      createCommand(message.master, Command::COMMAND_40, 3, {0x02, 0x02, 0x00}),
      createCommand(message.master, Command::COMMAND_40, 4, {0x06, 0x02, 0x00, 0x01}),
      createCommand(message.master, Command::COMMAND_40, 4, {0x06, 0x02, 0x00, 0x02}),
      createCommand(message.master, Command::COMMAND_40, 4, {0x06, 0x02, 0x00, 0x10}),
      createCommand(message.master, Command::COMMAND_13, 1, {0xFF}),
      createCommand(message.master, Command::COMMAND_D0, 3, {0x31, 0x0B, 0x00}),
  };
}

auto DisplayProcessor::checkMessageForMe(Message const& message) const -> bool {
  auto const isBroadcast = message.broadcast == BroadcastType::BROADCAST;
  auto const address     = message.slave;

  if (isBroadcast) return true;
  if (address == m_address) return true;

  return false;
}

auto DisplayProcessor::createCommand(Address const target, Command const command, Size const length, Bytes const payload) const noexcept -> Message {
  Message message = {
      .master    = m_address,
      .slave     = target,
      .broadcast = BroadcastType::DEVICE,
      .control   = ControlType::WRITE_COMMAND,
      .length    = (length + 1),
      .data      = {},
  };

  message.data[0] = static_cast<Bit>(command);

  for (Size i = 0; ((i < length) and (i < (MAX_MESSAGE_SIZE - 1))); ++i) {
    message.data[i + 1] = payload[i];
  }

  return message;
}

auto DisplayProcessor::createBroadcastCommand(Command const command, Size const length, Bytes const payload) const noexcept -> Message {
  auto message      = createCommand(BROADCAST_ADDRESS, command, length, payload);
  message.broadcast = BroadcastType::BROADCAST;

  return message;
}

} // namespace iebus

