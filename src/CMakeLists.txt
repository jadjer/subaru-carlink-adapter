cmake_minimum_required(VERSION 3.22.1)

set(SOURCES
        main.cpp

        configuration.cpp

        avc.cpp
        avc_line.cpp
        avc_line.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
   PICO_DEFAULT_WS2812_PIN=23
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_link_libraries(${PROJECT_NAME} pico_stdlib)
target_link_libraries(${PROJECT_NAME} pico_status_led)
#target_link_libraries(${PROJECT_NAME} pico_multicore)
target_link_libraries(${PROJECT_NAME} hardware_gpio)
#target_link_libraries(${PROJECT_NAME} hardware_uart)
target_link_libraries(${PROJECT_NAME} hardware_adc)
#target_link_libraries(${PROJECT_NAME} tinyusb_board)
#target_link_libraries(${PROJECT_NAME} tinyusb_device)

pico_add_extra_outputs(${PROJECT_NAME})
