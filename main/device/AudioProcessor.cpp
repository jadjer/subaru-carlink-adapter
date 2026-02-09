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

iebus::Address constexpr DEVICE_ADDRESS              = 0x230;
iebus::Timer::Time constexpr MICROSECONDS_PER_SECOND = 1'000'000;

} // namespace

AudioProcessor::AudioProcessor() noexcept : iebus::Device(DEVICE_ADDRESS) {
}

auto AudioProcessor::processMessage(iebus::Message const& message) noexcept -> AudioProcessor::MessageList {
  if (not checkMessageForMe(message)) return {};

  if (message.length < 1) return {};

  auto const command = message.data[0];

  if (command == 0x10) return handleCommand10(message);

  if (m_isInitialized) {
    if (command == 0x20) return handleCommand20(message);

    if (m_isRegistered) {
      if (command == 0x40) return handleCommand40(message);
      if (command == 0x60) return handleCommand60(message);
      if (command == 0x1F) return update();
    }
  }

  return {};
}

auto AudioProcessor::handleCommand10(iebus::Message const& message) noexcept -> AudioProcessor::MessageList {
  if (message.length < 2) return {};

  m_playTime      = 0;
  m_isEnabled     = false;
  m_isRegistered  = false;
  m_isInitialized = true;

  auto const sessionIndex = message.data[1];

  return {
      iebus::Message{
                     m_address,                       message.master,
                     iebus::BroadcastType::DEVICE,
                     iebus::ControlType::WRITE_COMMAND,
                     5,                               {0x11, sessionIndex, 0x00, 0x01, 0x02},
                     },
      iebus::Message{
                     m_address,                               0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     9, {0x60, 0x22, 0x02, 0x11, 0x13, 0x00, 0x00, 0x00, 0x00},
                     },
      iebus::Message{
                     m_address,                               0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     22,                       {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0xBB, 0x01, 0x01, 0x01, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00},
                     },
      iebus::Message{
                     m_address, 0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     22,                               {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0xBB, 0x01, 0x58, 0x06, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00},
                     },
  };
}

auto AudioProcessor::handleCommand20(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  if (message.length < 2) return {};

  auto const registerIndex = message.data[1];

  if (registerIndex == 0x01) m_isRegistered = true;

  return {};
}

auto AudioProcessor::handleCommand40(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  return {
      iebus::Message{
                     m_address,                       0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     22,                              {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0xBB, 0x01, 0x58, 0x06, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00},
                     },
      iebus::Message{
                     m_address,                               0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     13, {0x60, 0x22, 0x01, 0x06, 0x03, 0x63, 0x73, 0x59, 0x02, 0xFF, 0xFF, 0xFF, 0xFF},
                     },
      iebus::Message{
                     m_address,                              0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     9,                       {0x60, 0x22, 0x02, 0x11, 0x13, 0x00, 0x00, 0x00, 0x00},
                     },
      iebus::Message{
                     m_address, 0x100,
                     iebus::BroadcastType::BROADCAST,
                     iebus::ControlType::WRITE_DATA,
                     7,                               {0x70, 0x22, 0x0A, 0x3F, 0x03, 0x2C, 0x02},
                     },
  };
}

auto AudioProcessor::handleCommand60(iebus::Message const& message) noexcept -> iebus::Device::MessageList {
  if (message.length < 2) return {};

  auto const subCommand = message.data[1];

  if (subCommand == 0xC0) {
    if (message.length < 10) return {};

    auto const enableKey = message.data[9];

    if (enableKey == 0x00) {
      m_playTime = 0;
      m_isEnabled = false;
    }

    if (enableKey == 0x22) {
      m_playTime = 0;
      m_isEnabled = true;
    }
  }

  return update();
}

auto AudioProcessor::update() noexcept -> iebus::Device::MessageList {
  auto const currentTime = m_timer.getTime();
  auto const diffTime    = currentTime - m_lastUpdateTime;

  if (not m_isInitialized) {
    auto constexpr INIT_TIMEOUT = (1 * MICROSECONDS_PER_SECOND);

    if (diffTime < INIT_TIMEOUT) return {};

    m_lastUpdateTime = currentTime;

    return {
        iebus::Message{
                       m_address, 0xFFF,
                       iebus::BroadcastType::BROADCAST,
                       iebus::ControlType::WRITE_COMMAND,
                       1, {0x12},
                       },
    };
  }

  if (m_isEnabled) {
    auto constexpr UPDATE_TIME = (1 * MICROSECONDS_PER_SECOND);

    if (diffTime < UPDATE_TIME) return {};

    m_lastUpdateTime = currentTime;

    if (m_playTime > 500) m_playTime = 0;

    iebus::Bit const trackNumber = 0x34;
    iebus::Bit const minutes = (m_playTime / 60);
    iebus::Bit const seconds = (m_playTime % 60);

    m_playTime++;

    iebus::Bit const highNibble = (0xB0 | (minutes % 10));
    iebus::Bit const lowNibble = (((seconds / 10) << 4) | (seconds % 10));

    return {
        iebus::Message{
                       m_address, 0x100,
                       iebus::BroadcastType::BROADCAST,
                       iebus::ControlType::WRITE_DATA,
                       22, {0x60, 0x22, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x04, 0xBB, 0x01, trackNumber, 0x02, highNibble, lowNibble, 0x30, 0x30, 0x30, 0x30, 0x00},
                       },
    };
  }

  return {};
}
