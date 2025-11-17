//
// Created by jadjer on 16.11.25.
//

#include "avc_line.hpp"

#include <hardware/gpio.h>

namespace {
    constexpr auto GPIO_RO_PIN = 13;
    constexpr auto GPIO_DI_PIN = 12;
    constexpr auto GPIO_DI_ENABLE = 11;
}

AVCLine::AVCLine() : m_inputLevel(true) {
    m_instance = this;

    gpio_init(GPIO_RO_PIN);
    gpio_set_dir(GPIO_RO_PIN, GPIO_IN);

    gpio_init(GPIO_DI_PIN);
    gpio_set_dir(GPIO_DI_PIN, GPIO_OUT);

    gpio_init(GPIO_DI_ENABLE);
    gpio_set_dir(GPIO_DI_ENABLE, GPIO_OUT);

//    gpio_set_irq_enabled_with_callback(GPIO_RO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &AVCLine::receiveCallback);
}

AVCLine::~AVCLine() {
//    gpio_set_irq_enabled(GPIO_RO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);

    m_instance = nullptr;
}

bool AVCLine::isInputSet() const {
    gpio_put(GPIO_DI_ENABLE, false);

    return not gpio_get(GPIO_RO_PIN);
}

bool AVCLine::isInputClear() const {
    gpio_put(GPIO_DI_ENABLE, false);

    return gpio_get(GPIO_RO_PIN);
}

void AVCLine::outputSet() {
    gpio_put(GPIO_DI_ENABLE, true);

    gpio_put(GPIO_DI_PIN, false);
}

void AVCLine::outputReset() {
    gpio_put(GPIO_DI_ENABLE, true);

    gpio_put(GPIO_DI_PIN, true);
}

void AVCLine::receiveCallback(uint gpio, std::uint32_t events) {
    if (not m_instance or gpio != GPIO_RO_PIN) {
        return;
    }

    if (events & GPIO_IRQ_EDGE_RISE) {
        m_instance->m_inputLevel = true;
    }

    if (events & GPIO_IRQ_EDGE_FALL) {
        m_instance->m_inputLevel = false;
    }
}

AVCLine *AVCLine::m_instance = nullptr;
