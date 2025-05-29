#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// Operational modes matching C# enum
typedef enum
{
    MODE_IDLE = 0,
    MODE_DATA_ACQUISITION = 1,
    MODE_CONTROL = 2,
    MODE_DEBUG = 3
} operational_mode_t;

// Global state
static operational_mode_t current_mode = MODE_IDLE;
static char input_buffer[256];
static int buffer_index = 0;
static bool led_state = false;
bool running = false;
bool serial_data_available = false;

// Built-in LED pin
#define LED_PIN 25

const char *start_cmd = "start";
const char *cli_opt = "cli";
const char *gui_opt = "gui";

// Function declarations
void process_command(const char *command);
void process_start_cmd(char *command);
bool get_pi_input();

int main()
{
    // Initialize stdio for USB
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // Wait for USB connection
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    // Before continuing execution/starting the rest of the system wait for start command from GUI/PI
    while (!running)
    {
        while (!get_pi_input())
        {
            gpio_put(LED_PIN, true);
            sleep_ms(100);
            gpio_put(LED_PIN, false);
            sleep_ms(100);
        };

        process_start_cmd(input_buffer);

        if (!running)
        {
            printf("Invalid start command...");
        }
    }

    printf("Alice Mk1 is running!");

    while (running)
    {
        serial_data_available = get_pi_input();

        if (serial_data_available)
        {
            process_command(input_buffer);
        }
        else
        {
            gpio_put(LED_PIN, true);
            sleep_ms(100);
            gpio_put(LED_PIN, false);
            sleep_ms(300);
            sleep_ms(200);
        }
    }

    return 0;
}

void process_start_cmd(char *command)
{
    char *token = strtok(command, "_");
    bool start_received = false;

    if (token != NULL)
    {
        if (strcmp(token, start_cmd) == 0)
        {
            printf("Initializing robot...\n");
            printf("Reading configuration...\n");
        }
        else
        {
            printf("Please use start cmd to start initialization...\n");
            return;
        }
    }

    token = strtok(NULL, "_");

    if (token != NULL)
    {
        if (strcmp(token, cli_opt) == 0)
        {
            printf("Configuring for cli operation...\n");
        }
        else if (strcmp(token, gui_opt) == 0)
        {
            printf("Configuring for gui operation...\n");
        }
        else
        {
            printf("Invalid option ...\n");
            return;
        }
    }
    else
    {
        printf("Please add configuration options!\n");
        return;
    }

    running = true;
}

void process_command(const char *command)
{
    printf(">>> You said: %s\n", command);
}

bool get_pi_input()
{
    // Main loop
    while (true)
    {
        // Check for incoming characters
        int c = getchar_timeout_us(0); // 1ms timeout

        if (c != PICO_ERROR_TIMEOUT)
        {
            // Handle received character
            if (c == '>')
            {
                // End of command - process it
                if (buffer_index > 0)
                {
                    input_buffer[buffer_index] = '\0'; // Null terminate
                    buffer_index = 0;                  // Reset buffer
                    return true;
                }
            }
            else if (buffer_index < sizeof(input_buffer) - 1)
            {
                // Add character to buffer
                input_buffer[buffer_index++] = (char)c;
            }
            else
            {
                // Buffer overflow - reset
                // send_error("Command too long");
                buffer_index = 0;
            }
        }
        return false;

        // Update behavior based on current mode
        // update_mode_behavior();

        // Small delay to prevent overwhelming the system
        sleep_ms(1);
    }
}
