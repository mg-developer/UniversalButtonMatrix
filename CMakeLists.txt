cmake_minimum_required(VERSION 3.13)


option(HOST_BUILD "Build for host/x86 for testing" OFF)
option(VARIANT_32_BUTTONS_ENABLED "Enable the 32-button variant configuration" OFF)
option(VARIANT_64_BUTTONS_ENABLED "Enable the 32-button variant configuration" OFF)
option(VARIANT_128_BUTTONS_ENABLED "Enable the 32-button variant configuration" OFF)
set(CONSOLE_DEBUG "MEDIUM" CACHE STRING "Console debug level")
set_property(CACHE CONSOLE_DEBUG PROPERTY STRINGS "OFF" "LOW" "MEDIUM" "HIGH")


include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

if(HOST_BUILD)
    project(UniversalButtonMatrix_host C CXX)

    file(GLOB SRC_HEADERS "${CMAKE_CURRENT_LIST_DIR}/include/*.h")

    add_executable(${PROJECT_NAME}
        src/main_host.cpp
        src/shiftregister.cpp
        ${SRC_HEADERS}
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE HOST_BUILD=1)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/include
    )

    # Host build: nothing else required, use system toolchain
else()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

    project(UniversalButtonMatrix C CXX)
  
    pico_sdk_init()

    add_executable(${PROJECT_NAME}
        src/main.cpp
        src/usb_descriptors.cpp
        src/shiftregister.cpp
        ${SRC_HEADERS}
    )

    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        tinyusb_device
        tinyusb_board
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
    )
  
    target_compile_definitions(${PROJECT_NAME} PRIVATE HOST_BUILD=0)
  
    pico_enable_stdio_usb(${PROJECT_NAME} 0)
    pico_add_extra_outputs(${PROJECT_NAME})
endif()

if(VARIANT_32_BUTTONS_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_32_BUTTONS_ENABLED=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_32_BUTTONS_ENABLED=0)
endif()
if(VARIANT_64_BUTTONS_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_64_BUTTONS_ENABLED=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_64_BUTTONS_ENABLED=0)
endif()
if(VARIANT_128_BUTTONS_ENABLED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_128_BUTTONS_ENABLED=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE VARIANT_128_BUTTONS_ENABLED=0)
endif()


# Map text/booleans to numeric levels
if(CONSOLE_DEBUG STREQUAL "HIGH")
    set(NUM_LEVEL 3)
elseif(CONSOLE_DEBUG STREQUAL "MEDIUM")
    set(NUM_LEVEL 2)
elseif(CONSOLE_DEBUG STREQUAL "LOW")
    set(NUM_LEVEL 1)
else()
    set(NUM_LEVEL 0)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE CONSOLE_DEBUG_VERBOSITY_LEVEL=${NUM_LEVEL})


