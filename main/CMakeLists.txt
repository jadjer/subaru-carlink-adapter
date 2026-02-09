cmake_minimum_required(VERSION 3.22.1)

set(INCLUDES
        "."
)

set(SOURCES
        main.cpp

        CAN.cpp
        USB.cpp

        media/MessageParser.cpp
        media/device/AudioProcessor.cpp
        media/device/DisplayProcessor.cpp
)

set(PRIV_REQUIRES
        esp-iebus
        esp_driver_twai
)

idf_component_register(INCLUDE_DIRS ${INCLUDES} SRCS ${SOURCES} PRIV_REQUIRES ${PRIV_REQUIRES})
