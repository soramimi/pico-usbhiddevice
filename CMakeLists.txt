cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

#add_subdirectory(usb)

#if (TARGET tinyusb_device)
#    add_subdirectory(usb/device)
#else ()
#    message("Skipping TinyUSB device examples as TinyUSB is unavailable")
#endif ()

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
#add_subdirectory(${PICO_TINYUSB_PATH}/examples/device tinyusb_device_examples)

#add_subdirectory($ENV{NAME})
#add_subdirectory(dev_lowlevel)

#if (TARGET tinyusb_host)
#    add_subdirectory(usb/host)
#else ()
#    message("Skipping TinyUSB host examples as TinyUSB is unavailable")
#endif ()

add_executable($ENV{NAME})

target_sources($ENV{NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories($ENV{NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries($ENV{NAME} PUBLIC pico_stdlib tinyusb_device tinyusb_board hardware_i2c hardware_spi)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions($ENV{NAME} PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs($ENV{NAME})

# add url via pico_set_program_url
example_auto_set_url($ENV{NAME})
