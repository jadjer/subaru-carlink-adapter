cmake_minimum_required(VERSION 3.22.1)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(cmake/pico_sdk_import.cmake)

project(avc
        VERSION 0.0.1
        DESCRIPTION "IEBus to USB converter"
        HOMEPAGE_URL https://github.com/jadjer/avc
        LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

add_subdirectory(src)
