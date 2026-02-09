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

namespace {

iebus::Address constexpr DEVICE_ADDRESS = 0x150;

} // namespace

DisplayProcessor::DisplayProcessor() noexcept : iebus::Device(DEVICE_ADDRESS) {
}

iebus::Device::MessageList DisplayProcessor::processMessage(iebus::Message const& message) {
  if (not checkMessageForMe(message)) return {};

  if (message.length < 1) return {};

  auto const command = message.data[0];

  if (command == 0x10) return handleCommand10(message);
  if (not m_isInitialized) return createCommandInit();

  if (command == 0x20) return handleCommand20(message);
  if (not m_isRegistered) return {};

  if (not m_isConfigured) return createCommandConfiguration(message);

  if (command == 0x1F) return update();

  return {};
}

auto DisplayProcessor::update() noexcept -> DisplayProcessor::MessageList {
  return {};
}

auto DisplayProcessor::handleCommand10(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  if (message.length < 2) return {};

  m_isInitialized = true;

  m_isRegistered = false;
  m_isConfigured = false;

  auto const sessionIndex = message.data[1];

  return {
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 6, {0x11, sessionIndex, 0x01, 0x02, 0x85, 0x93}},
  };
}

auto DisplayProcessor::handleCommand20(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  if (message.length < 2) return {};

  m_isRegistered = false;
  m_isConfigured = false;

  auto const registerIndex = message.data[1];

//  if (registerIndex == 1)
  //    return {
  //        iebus::Message{
  //                       m_address, message.master,
  //                       iebus::BroadcastType::DEVICE,
  //                       iebus::ControlType::WRITE_COMMAND,
  //                       4, {0x40, 0xC0, 0x20, m_isInitialized},
  //                       },
  //    };
  if (registerIndex == 2) m_isRegistered = true;

  return {};
}

auto DisplayProcessor::createCommandInit() noexcept -> iebus::Device::MessageList {
  return {
      iebus::Message{m_address, 0xFFF, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_COMMAND, 1, {0x12}},
  };
}

auto DisplayProcessor::createCommandConfiguration(iebus::Message const& message) noexcept -> DisplayProcessor::MessageList {
  m_isConfigured = true;

  return {
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x02, 0x10}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x06, 0x10}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0xC0, 0x10}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x83, 0x10}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x02, 0x00}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x06, 0x00}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0xC0, 0x00}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 3, {0x40, 0x83, 0x00}            },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 4, {0x40, 0x02, 0x02, 0x00}      },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 5, {0x40, 0x06, 0x02, 0x00, 0x01}},
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 5, {0x40, 0x06, 0x02, 0x00, 0x02}},
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 5, {0x40, 0x06, 0x02, 0x00, 0x10}},
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 2, {0x13, 0xFF}                  },
      iebus::Message{m_address, message.master, iebus::BroadcastType::DEVICE, iebus::ControlType::WRITE_COMMAND, 4, {0xD0, 0x31, 0x0B, 0x00}      },
  };
}
