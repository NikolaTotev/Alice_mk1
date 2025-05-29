# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hid_building_block C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(hid_building_block hid_building_block.c usb_descriptors.c)

pico_set_program_name(hid_building_block "hid_building_block")
pico_set_program_version(hid_building_block "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(hid_building_block 0)
pico_enable_stdio_usb(hid_building_block 1)

# Add the standard library to the build
target_link_libraries(hid_building_block
    tinyusb_device
    tinyusb_board
    pico_unique_id
    hardware_flash
    pico_stdlib)

# Explicitly compile TinyUSB source files that might be missing
target_sources(hid_building_block PRIVATE
    ${PICO_TINYUSB_PATH}/src/tusb.c
    ${PICO_TINYUSB_PATH}/src/device/usbd.c
    ${PICO_TINYUSB_PATH}/src/device/usbd_control.c
    ${PICO_TINYUSB_PATH}/src/class/hid/hid_device.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/rp2040/dcd_rp2040.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/rp2040/rp2040_usb.c
)

# # Add the standard include files to the build
# target_include_directories(hid_building_block PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}
# )



pico_add_extra_outputs(hid_building_block)

