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
bool move_motor = false;
int movement_speed = 800;

// Built-in LED pin
#define LED_PIN 25
#define step_pin 15
#define dir_pin 14
// #define driver_ena_pin 7
#define shifter_ena_pin 10

const char *start_cmd = "start";
const char *cli_opt = "cli";
const char *gui_opt = "gui";
const char *cmd_move_cw = "mcw";
const char *cmd_move_ccw = "mccw";
const char *cmd_update_speed = "update-speed";
const char *stop_cmd = "stop";

// Function declarations
void process_command(const char *command);
void move_cw(char *args_token);
void move_ccw(char *args_token);
void update_speed(char *args_token);
void process_start_cmd(char *command);
bool get_pi_input();

int main()
{
    // Initialize stdio for USB
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_init(step_pin);
    gpio_init(dir_pin);
    // gpio_init(driver_ena_pin);
    gpio_init(shifter_ena_pin);

    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(step_pin, GPIO_OUT);
    gpio_set_dir(dir_pin, GPIO_OUT);
    // gpio_set_dir(driver_ena_pin, GPIO_OUT);
    gpio_set_dir(shifter_ena_pin, GPIO_OUT);

    gpio_put(LED_PIN, false);
    gpio_put(step_pin, true);
    gpio_put(dir_pin, true);
    // gpio_put(driver_ena_pin, true);
    gpio_put(shifter_ena_pin, true);

    // Wait for USB connection
    while (!stdio_usb_connected())
    {
        gpio_put(LED_PIN, true);
        sleep_ms(100);
        gpio_put(LED_PIN, false);
        sleep_ms(50);
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
        printf("Starting...");
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
            // gpio_put(LED_PIN, true);
            // sleep_us(10);
            // gpio_put(LED_PIN, false);
            // sleep_us(10);
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
    printf("Raw Received cmd was: %s\n", command);

    char *token = strtok(command, "_");
    bool start_received = false;

    if (token != NULL)
    {
        if (strcmp(token, cmd_move_cw) == 0)
        {
            printf("Recieved move_cw...\n");
            token = strtok(NULL, "_");
            move_cw(token);
        }
        else if (strcmp(token, cmd_move_ccw) == 0)
        {
            printf("Recieved move_ccw...\n");
            token = strtok(NULL, "_");
            move_ccw(token);
        }
        else if (strcmp(token, stop_cmd) == 0)
        {
            printf("Recieved stop...");
            move_motor = false;
        }
        else if (strcmp(token, cmd_update_speed) == 0)
        {
            printf("Updating speed...");
            token = strtok(NULL, "_");
            update_speed(token);
        }
        else
        {
            printf("Invalid command ...\n");
            return;
        }
    }
}
void update_speed(char *args_token)
{
    if (args_token != NULL)
    {
        printf(args_token);
        int number = atoi(args_token); // +1 to skip the underscore
        movement_speed = number;
    }
}

void move_cw(char *args_token)
{
    printf("Executing move_cw...\n");
    move_motor = true;
    gpio_put(dir_pin, false);
    sleep_us(6);

    if (args_token != NULL)
    {
        printf(args_token);
        int number = atoi(args_token); // +1 to skip the underscore
        movement_speed = number;
    }

    while (move_motor)
    {
        serial_data_available = get_pi_input();

        if (serial_data_available)
        {
            process_command(input_buffer);
        }

        gpio_put(step_pin, false);
        sleep_us(movement_speed);
        gpio_put(step_pin, true);
        sleep_us(movement_speed);
    }
}

void move_ccw(char *args_token)
{
    printf("Executing move_ccw...\n");
    move_motor = true;
    gpio_put(dir_pin, true);
    sleep_us(6);

    
    if (args_token != NULL)
    {
        printf(args_token);
        int number = atoi(args_token); // +1 to skip the underscore
        movement_speed = number;
    }

    while (move_motor)
    {
        serial_data_available = get_pi_input();

        if (serial_data_available)
        {
            process_command(input_buffer);
        }

        gpio_put(step_pin, false);
        sleep_us(movement_speed);
        gpio_put(step_pin, true);
        sleep_us(movement_speed);
    }
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
