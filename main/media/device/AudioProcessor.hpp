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

#pragma once

#include <iebus/Device.hpp>
#include <iebus/types.hpp>

class AudioProcessor : public iebus::Device {
public:
  AudioProcessor() noexcept;
  ~AudioProcessor() noexcept override = default;

public:
  [[nodiscard]] auto processMessage(iebus::Message const& message) noexcept -> iebus::Device::MessageList override;

private:
  [[nodiscard]] auto handleCommand10(iebus::Message const& message) noexcept -> iebus::Device::MessageList;
  [[nodiscard]] auto handleCommand20(iebus::Message const& message) noexcept -> iebus::Device::MessageList;

private:
  [[nodiscard]] auto update() noexcept -> iebus::Device::MessageList override;

private:
  [[nodiscard]] auto createCommandInit() noexcept -> iebus::Device::MessageList;
  [[nodiscard]] auto createCommandConfiguration() noexcept -> iebus::Device::MessageList;

private:
  bool m_isConfigured  = false;
  bool m_isRegistered  = false;
  bool m_isInitialized = false;

private:
  iebus::Data m_playTime = 0;
};
