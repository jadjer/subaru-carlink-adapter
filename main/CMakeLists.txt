cmake_minimum_required(VERSION 3.22.1)

set(INCLUDES
        "."
)

set(SOURCES
        main.cpp

        USB.cpp
        MessageParser.cpp
)

set(PRIV_REQUIRES
        esp_driver_gpio
        esp-iebus
)

idf_component_register(INCLUDE_DIRS ${INCLUDES} SRCS ${SOURCES} PRIV_REQUIRES ${PRIV_REQUIRES})
