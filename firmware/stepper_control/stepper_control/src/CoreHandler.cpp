#include <cstdio>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "CoreHandler.h"
#include "SerialHandler.h"
#include "MotionControl.h"
#include "Stepper.h"

// Core 0 main loop function
void CoreHandler::core0Main()
{
    // Main loop for Core 0
    while (true)
    {
        // Process any serial data
        SerialHandler::process();

        // Add a small delay to prevent CPU hogging
        sleep_ms(10);
    }
}

// Core 1 entry function
void CoreHandler::core1Entry()
{
    printf("Core 1 started\n");

    Stepper stepper;

    Stepper::Config stepperConfig;
    stepperConfig.pio = pio0;
    stepperConfig.sm = 0;
    stepperConfig.step_pin = 4;
    stepperConfig.dir_pin = 5;
    stepperConfig.driver_ena_pin = 7;
    stepperConfig.shifter_ena_pin = 8;
    stepperConfig.clk_div = 1.0;

    stepper.init(stepperConfig);

    while (true)
    {
        // Wait for command from Core 0
        uint32_t cmd_ptr = multicore_fifo_pop_blocking();
        Command *cmd = reinterpret_cast<Command *>(cmd_ptr);

        // Process the command
        switch (cmd->cmd_type)
        {
        case CommandType::JOG:
            stepper.jogCW(cmd->speed/0.008);
            MotionControl::start_jog(cmd->speed, cmd->direction);
            break;

        case CommandType::TRACK:
            // stepper.jogCW(62500);
            MotionControl::track();
            break;
        case CommandType::JOG_STOP:{
            stepper.stopJog();
        }

        default:
            printf("Core 1: Unknown command type: %d\n", cmd->cmd_type);
            break;
        }
    }
}