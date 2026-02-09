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
// Created by jadjer on 01.02.2026.
//

#include "AudioProcessor.hpp"

namespace {

iebus::Address constexpr DEVICE_ADDRESS = 0x250;

} // namespace

AudioProcessor::AudioProcessor() noexcept : iebus::Device(DEVICE_ADDRESS) {
  m_playTime = 0;
}

auto AudioProcessor::processMessage(iebus::Message const& message) noexcept -> AudioProcessor::MessageList {
  if (not checkMessageForMe(message)) return {};

  if (message.length < 1) return {};

  auto const command = message.data[0];

  if (command == 0x10) return handleCommand10(message);
  if (not m_isInitialized) return createCommandInit();

  if (command == 0x20) return handleCommand20(message);
  if (not m_isRegistered) return {};

  if (not m_isConfigured) return createCommandConfiguration();

  if (command == 0x1F) return update();

  return {};
}

auto AudioProcessor::handleCommand10(iebus::Message const& message) noexcept -> AudioProcessor::MessageList {
  if (message.length < 2) return {};

  m_isInitialized = true;

  m_isRegistered = false;
  m_isConfigured = false;

  return {
      iebus::Message{
                     m_address, message.master,
                     iebus::BroadcastType::DEVICE,
                     iebus::ControlType::WRITE_COMMAND,
                     5, {0x11, message.data[1], 0x00, 0x01, 0x02},
                     },
  };
}

auto AudioProcessor::handleCommand20(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  if (message.length < 2) return {};

  m_isRegistered = false;
  m_isConfigured = false;

  auto const registerIndex = message.data[1];

  if (registerIndex == 1) m_isRegistered = true;

  return {};
}

auto AudioProcessor::createCommandInit() noexcept -> AudioProcessor::MessageList {
  return {
      iebus::Message{m_address, 0xFFF, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_COMMAND, 1, {0x12}},
  };
}

auto AudioProcessor::createCommandConfiguration() noexcept -> AudioProcessor::MessageList {
  m_isConfigured = true;

  return {
      iebus::Message{m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 9,  {0x60, 0x22, 0x02, 0x11, 0x13, 0x00, 0x00, 0x00, 0x00}                                                                                                                       },
      iebus::Message{m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 22, {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
                                                                                                             0xBB, 0x01, 0x01, 0x01, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00}},
      iebus::Message{m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 22, {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
                                                                                                             0xBB, 0x01, 0x58, 0x06, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00}},
      iebus::Message{
                     m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 13, {0x60, 0x22, 0x01, 0x06, 0x03, 0x63, 0x73, 0x59, 0x02, 0xFF, 0xFF, 0xFF, 0xFF}                                                                                               },
      iebus::Message{m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 9,  {0x60, 0x22, 0x02, 0x11, 0x13, 0x00, 0x00, 0x00, 0x00}                                                                                                                       },
      iebus::Message{m_address, 0x100, iebus::BroadcastType::BROADCAST, iebus::ControlType::WRITE_DATA, 7,  {0x70, 0x22, 0x0A, 0x3F, 0x03, 0x2C, 0x02}                                                                                                                                   },
  };
}

auto AudioProcessor::update() noexcept -> iebus::Device::MessageList {
  m_playTime++;

  if (m_playTime > 500) m_playTime = 0;

  iebus::Bit const trackNumber = 0x34;
  iebus::Bit const highNibble  = m_playTime / 60;
  iebus::Bit const lowNibble   = m_playTime % 60;

  return {
      iebus::Message{
                     m_address, 0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     21, {0x22, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x04, 0xBB, 0x01, trackNumber, 0x02, highNibble, lowNibble, 0x30, 0x30, 0x30, 0x30, 0x00},
                     },
  };
}
