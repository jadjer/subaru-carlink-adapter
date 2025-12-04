cmake_minimum_required(VERSION 3.22.1)

set(INCLUDES
        "."
)

set(SOURCES
        main.cpp
)

set(PRIV_REQUIRES
        esp32-iebus
)

idf_component_register(INCLUDE_DIRS ${INCLUDES} SRCS ${SOURCES}  PRIV_REQUIRES ${PRIV_REQUIRES})
