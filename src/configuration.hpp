//
// Created by jadjer on 9.11.25.
//

#ifndef AVC_CONFIGURATION_HPP
#define AVC_CONFIGURATION_HPP

#include <cstdint>

using Pin = std::uint8_t;

class Configuration {
public:
    /**
     * Get IEBus receiver pin number
     * @return pin number
     */
    [[nodiscard]] static Pin getBusReceiverPin();

    /**
     * Get IEBus transmitter pin
     * @return pin number
     */
    [[nodiscard]] static Pin getBusTransmitterPin();

    /**
     * Get IEBus enable transmitter (disable receiver) pin
     * @return pin number
     */
    [[nodiscard]] static Pin getBusEnablePin();

    /**
     * Get WS2812 led pin number (build-in)
     * @return pin number
     */
    [[nodiscard]] static Pin getLedPin();
};


#endif //AVC_CONFIGURATION_HPP
