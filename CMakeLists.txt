cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(UniversalButtonMatrix C CXX)

pico_sdk_init()

add_executable(UniversalButtonMatrix
    src/main.cpp
    src/usb_descriptors.cpp
)

target_link_libraries(UniversalButtonMatrix
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

target_include_directories(UniversalButtonMatrix PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_enable_stdio_usb(UniversalButtonMatrix 0)
pico_enable_stdio_uart(UniversalButtonMatrix 0)

pico_add_extra_outputs(UniversalButtonMatrix)

