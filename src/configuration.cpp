//
// Created by jadjer on 9.11.25.
//

#include "configuration.hpp"

Pin Configuration::getBusReceiverPin() {
    return 12;
}

Pin Configuration::getBusTransmitterPin() {
    return 10;
}

Pin Configuration::getBusEnablePin() {
    return 11;
}

Pin Configuration::getLedPin() {
    return 23;
}
