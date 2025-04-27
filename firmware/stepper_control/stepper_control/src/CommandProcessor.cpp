#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "CommandProcessor.h"

// Parse the command from serial and send to Core 1
void CommandProcessor::parseCommand(const char *cmd_str)
{
    // Create a local copy we can modify
    char cmd_buffer[MAX_CMD_SIZE];
    strncpy(cmd_buffer, cmd_str, MAX_CMD_SIZE - 1);
    cmd_buffer[MAX_CMD_SIZE - 1] = '\0'; // Ensure null termination

    // Create command structure
    static Command cmd; // Static to ensure it exists while being processed by Core 1
    memset(&cmd, 0, sizeof(cmd));

    // Convert to uppercase for case-insensitive comparison
    for (int i = 0; cmd_buffer[i]; i++)
    {
        if (cmd_buffer[i] >= 'a' && cmd_buffer[i] <= 'z')
        {
            cmd_buffer[i] = cmd_buffer[i] - 'a' + 'A';
        }
    }

    printf("Processing command: %s\n", cmd_buffer);

    // Check for JOG command ===========================================================================
    if (strncmp(cmd_buffer, CMD_JOG, 3) == 0)
    {
        cmd.cmd_type = CommandType::JOG;

        // Parse parameters: speed and direction
        char *token = strtok(cmd_buffer, ","); // Skip "JOG"
        printf(token);
        // Get speed
        token = strtok(nullptr, ",");
        if (token)
        {
            cmd.speed = atoi(token);

            // Get direction
            token = strtok(nullptr, ",");
            if (token)
            {

                int dir = atoi(token);

                if (dir == 0)
                {
                    cmd.direction = Direction::CW;
                }
                else
                {
                    cmd.direction = Direction::CCW;
                }

                // Send command to Core 1
                printf("Sending JOG command: speed=%d, direction=%d\n", cmd.speed, cmd.direction);
                multicore_fifo_push_blocking(reinterpret_cast<uint32_t>(&cmd));
            }
            else
            {
                printf("Error: Missing direction parameter\n");
            }
        }
        else
        {
            printf("Error: Missing speed parameter\n");
        }
    }
    // Check for JOG STOP command ===========================================================================
    else if (strncmp(cmd_buffer, CMD_JOG_STOP, 2) == 0)
    {
        cmd.cmd_type = CommandType::JOG_STOP;
        printf("Sending JOG STOP command\n");
        multicore_fifo_push_blocking(reinterpret_cast<uint32_t>(&cmd));
    }
    // Check for TRACK command ===========================================================================
    else if (strncmp(cmd_buffer, CMD_TRACK, 5) == 0)
    {
        cmd.cmd_type = CommandType::TRACK;

        // Send command to Core 1
        printf("Sending TRACK command\n");
        multicore_fifo_push_blocking(reinterpret_cast<uint32_t>(&cmd));
    }
    // Check for TRACK STOP command ===========================================================================
    else if (strncmp(cmd_buffer, CMD_TRACK, 5) == 0)
    {
        cmd.cmd_type = CommandType::TACK_STOP;

        // Send command to Core 1
        printf("Sending TRACK command\n");
        multicore_fifo_push_blocking(reinterpret_cast<uint32_t>(&cmd));
    }
    else
    {
        printf("Unknown command: %s\n", cmd_buffer);
    }
}