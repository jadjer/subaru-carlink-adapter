cmake_minimum_required(VERSION 4.0)

set(SOURCES
        main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} pico_stdlib)

pico_add_extra_outputs(${PROJECT_NAME})
