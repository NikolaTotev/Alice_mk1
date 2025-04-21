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
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(stepper_control C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Source files for your C++ project
set(CPP_SOURCES
    src/main.cpp
    src/CommandProcessor.cpp
    src/CoreHandler.cpp
    src/MotionControl.cpp
    src/SerialHandler.cpp
)

# Add only the C++ executable
add_executable(stepper_control ${CPP_SOURCES})

# Set program information
pico_set_program_name(stepper_control "stepper_control")
pico_set_program_version(stepper_control "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(stepper_control 0)
pico_enable_stdio_usb(stepper_control 1)

# Add the standard library and other needed libraries to the build
target_link_libraries(stepper_control
    pico_stdlib
    pico_multicore  # Support for dual-core programming
    hardware_gpio   # GPIO access for motor control
)

# Add include directories
target_include_directories(stepper_control PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include  # Add the include directory
)

# Generate additional output files (.uf2, etc.)
pico_add_extra_outputs(stepper_control)