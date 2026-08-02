cmake_minimum_required(VERSION 3.13)

option(HOST_BUILD "Build for host/x86 for testing" OFF)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

if(HOST_BUILD)
    project(UniversalButtonMatrix_host C CXX)

    file(GLOB SRC_HEADERS "${CMAKE_CURRENT_LIST_DIR}/include/*.h")

    add_executable(UniversalButtonMatrix_host
        src/main_host.cpp
        src/shiftregister.cpp
        ${SRC_HEADERS}
    )

    target_compile_definitions(UniversalButtonMatrix_host PRIVATE HOST_BUILD=1)
    target_include_directories(UniversalButtonMatrix_host PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/include
    )

    # Host build: nothing else required, use system toolchain
else()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

    project(UniversalButtonMatrix C CXX)

    pico_sdk_init()

    add_executable(UniversalButtonMatrix
        src/main.cpp
        src/usb_descriptors.cpp
        src/shiftregister.cpp
        ${SRC_HEADERS}
    )

    target_link_libraries(UniversalButtonMatrix
        pico_stdlib
        tinyusb_device
        tinyusb_board
    )

    target_include_directories(UniversalButtonMatrix PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
    )

    file(READ "${CMAKE_CURRENT_LIST_DIR}/src/board_config.h" BOARD_CONFIG_H_CONTENTS)
    if(BOARD_CONFIG_H_CONTENTS MATCHES "#define[ \t]+CONSOLE_DEBUG[ \t]+1")
        pico_enable_stdio_uart(UniversalButtonMatrix 1)
    else()
        pico_enable_stdio_uart(UniversalButtonMatrix 0)
    endif()
    pico_enable_stdio_usb(UniversalButtonMatrix 0)

    pico_add_extra_outputs(UniversalButtonMatrix)
endif()

