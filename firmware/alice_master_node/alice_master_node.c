#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "hardware/spi.h"

// Define the data structure to send
typedef struct
{
    bool joint_1_dir;
    bool joint_2_dir;
    bool joint_3_dir;
    bool joint_4_dir;
    bool joint_5_dir;
    uint16_t joint_1_speed;
    uint16_t joint_2_speed;
    uint16_t joint_3_speed;
    bool stop;
    bool gripper_open;
} RobotState;

typedef struct
{
    bool active;
    bool direction;
    bool direction2;
    uint16_t speed;

} JointPacket;

// spi0 pins
#define SPI0_SCK 18  // GPIO 10
#define SPI0_MOSI 19 // GPIO 11
#define SPI0_MISO 16 // GPIO 12
#define SPI0_CS 17   // GPIO 13

// SPI settings
#define SPI_FREQ 1000000 // 1 MHz
#define LED_PIN 25

// counter control pins
#define clk_pin 22
#define rst_pin 21

// Frame format definitions (since they're not in the SDK)
#define FRAME_FORMAT_MOTOROLA 0x0
#define FRAME_FORMAT_TI 0x1
#define FRAME_FORMAT_MICROWIRE 0x2

#define Something "Something"

// Configuration commands
const char *start_cmd = "start";
const char *cli_opt = "cli";
const char *gui_opt = "gui";

// Movement commands
const char *cmd_update_state = "us";
const char *cmd_move_cw = "mcw";
const char *cmd_move_ccw = "mccw";
const char *cmd_update_speed = "update-speed";
const char *stop_cmd = "stop";

static char input_buffer[256];
static int buffer_index = 0;
static bool led_state = false;
bool running = false;
bool serial_data_available = false;
bool move_motor = false;
int movement_speed = 800;
RobotState state;

void configure_pins();
void configure_spi();
void configure_serial();
void print_spi_config();
void configure_robot();

void process_command(const char *command);
void process_start_cmd(char *command);
void process_update_state(const char *args_token);

void move_cw(char *args_token);
void move_ccw(char *args_token);
void update_speed(char *args_token);
bool get_pi_input();
bool stringAsBool(const char *str);

int main()
{
    configure_pins();
    configure_serial();
    configure_spi();
    configure_robot();

    while (running)
    {
        int counter = 1;
        serial_data_available = get_pi_input();

        if (serial_data_available)
        {
            process_command(input_buffer);

            JointPacket tx_data;

            for (size_t i = 0; i < 4; i++)
            {
                if (i == 0)
                {
                    tx_data.active = state.stop;
                    tx_data.direction = state.joint_1_dir;
                    tx_data.direction2 = state.joint_1_dir;
                    tx_data.speed = state.joint_1_speed;
                }

                if (i == 1)
                {

                    tx_data.active = state.stop;
                    tx_data.direction = state.joint_3_dir;
                    tx_data.direction2 = state.joint_3_dir;
                    tx_data.speed = state.joint_3_speed;
                }

                if (i == 2)
                {
                    tx_data.active = state.stop;
                    tx_data.direction = state.joint_2_dir;
                    tx_data.direction2 = state.joint_2_dir;
                    tx_data.speed = state.joint_2_speed;
                }

                if (i == 3)
                {
                    tx_data.active = state.stop;
                    tx_data.direction = state.joint_4_dir;
                    tx_data.direction2 = state.joint_5_dir;
                    tx_data.speed = 42;
                }

                // gpio_put(SPI0_CS, 0);
                sleep_us(10); // Small delay for slave to detect CS

                // Send data first
                spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(JointPacket));

                // Small delay between write and read (optional)
                sleep_us(10);
                printf("CLK %lu", i);
                gpio_put(clk_pin, true);
                gpio_put(LED_PIN, true);
                sleep_ms(1);
                gpio_put(clk_pin, false);
                gpio_put(LED_PIN, false);
                sleep_ms(1);
            }
        }

        gpio_put(rst_pin, true);
        gpio_put(LED_PIN, true);
        sleep_ms(1);
        gpio_put(rst_pin, false);
        gpio_put(LED_PIN, false);
        sleep_ms(1);

        // if (counter < 4)
        // {
        //     if (counter == 1)
        //     {
        //         tx_data.active = state.stop;
        //         tx_data.direction = state.joint_1_dir;
        //         tx_data.direction2 = state.joint_1_dir;
        //         tx_data.speed = state.joint_1_speed;
        //     }

        //     if (counter == 2)
        //     {
        //         tx_data.active = state.stop;
        //         tx_data.direction = state.joint_2_dir;
        //         tx_data.direction2 = state.joint_2_dir;
        //         tx_data.speed = state.joint_2_speed;
        //     }

        //     if (counter == 3)
        //     {
        //         tx_data.active = state.stop;
        //         tx_data.direction = state.joint_3_dir;
        //         tx_data.direction2 = state.joint_3_dir;
        //         tx_data.speed = state.joint_3_speed;
        //     }

        //     // printf("Sending to pico %lu: num1=%lu, num2=%lu\n", counter, tx_data.num1, tx_data.num2);

        //     // Pull CS low to start transmission
        //     // gpio_put(SPI0_CS, 0);
        //     sleep_us(10); // Small delay for slave to detect CS

        //     // Send data first
        //     spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(JointPacket));

        //     // Small delay between write and read (optional)
        //     sleep_us(10);

        //     // Read response data
        //     // spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_data, sizeof(JointPacket));

        //     // spi_write_read_blocking(spi0, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

        //     // Pull CS high to end transmission
        //     // gpio_put(SPI0_CS, 1);

        //     // Print received data
        //     // printf("Received from pico %lu: num1=%lu, num2=%lu\n", counter, rx_data.num1, rx_data.num2);

        //     // printf("Incrementing, %d\n\n", counter);
        //     gpio_put(clk_pin, true);
        //     gpio_put(LED_PIN, true);
        //     sleep_ms(1);
        //     gpio_put(clk_pin, false);
        //     gpio_put(LED_PIN, false);
        //     sleep_ms(1);
        //     counter++;
        // }
        // else
        // {
        //     tx_data.active = state.stop;
        //     tx_data.direction = state.joint_4_dir;
        //     tx_data.direction2 = state.joint_5_dir;
        //     tx_data.speed = 0;

        //     // printf("Sending to pico %lu: num1=%lu, num2=%lu\n", counter, tx_data.num1, tx_data.num2);

        //     // Pull CS low to start transmission
        //     // gpio_put(SPI0_CS, 0);
        //     sleep_us(10); // Small delay for slave to detect CS

        //     // Send data first
        //     spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(JointPacket));

        //     // Small delay between write and read (optional)
        //     sleep_us(10);

        //     // Read response data
        //     // spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_data, sizeof(JointPacket));

        //     // spi_write_read_blocking(spi0, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

        //     // Pull CS high to end transmission
        //     // gpio_put(SPI0_CS, 1);

        //     // Print received data
        //     // printf("Received from pico %lu: num1=%lu, num2=%lu\n", counter, rx_data.num1, rx_data.num2);

        //     printf("Resetting counter \n\n");
        //     gpio_put(rst_pin, true);
        //     gpio_put(LED_PIN, true);
        //     sleep_ms(1);
        //     gpio_put(rst_pin, false);
        //     gpio_put(LED_PIN, false);
        //     sleep_ms(1);
        //     counter = 1;
        // }
    }
}

void configure_pins()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    gpio_init(clk_pin);
    gpio_init(rst_pin);

    gpio_set_dir(clk_pin, GPIO_OUT);
    gpio_set_dir(rst_pin, GPIO_OUT);

    gpio_put(clk_pin, false);
    gpio_put(rst_pin, false);
}

void configure_serial()
{
    stdio_init_all();

    // Wait for USB connection
    while (!stdio_usb_connected())
    {
        gpio_put(LED_PIN, true);
        sleep_ms(100);
        gpio_put(LED_PIN, false);
        sleep_ms(100);
        gpio_put(LED_PIN, true);
        sleep_ms(100);
        gpio_put(LED_PIN, false);
        sleep_ms(300);
    }
}

void configure_spi()
{
    printf("Initializing SPI...\n");

    // Initialize spi0
    spi_init(spi0, SPI_FREQ);

    // Set SPI format
    // spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);

    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Get hardware register access
    spi_hw_t *hw = spi_get_hw(spi0);

    // Set TI frame format (bits 5:4 = 01)
    uint32_t cr0 = hw->cr0;
    cr0 &= ~(0x3 << 4);            // Clear FRF field
    cr0 |= (FRAME_FORMAT_TI << 4); // Set TI format
    hw->cr0 = cr0;

    // Initialize SPI pins
    gpio_init(SPI0_SCK);
    gpio_init(SPI0_MOSI);
    gpio_init(SPI0_MISO);
    gpio_init(SPI0_CS);

    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);

    gpio_set_drive_strength(SPI0_SCK, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MOSI, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MISO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_CS, GPIO_DRIVE_STRENGTH_2MA);
}

void print_spi_config()
{
    spi_hw_t *hw = spi_get_hw(spi0);
    uint32_t frame_format = (hw->cr0 >> 4) & 0x3;

    printf("Frame format: ");
    switch (frame_format)
    {
    case FRAME_FORMAT_MOTOROLA:
        printf("Motorola SPI\n");
        break;
    case FRAME_FORMAT_TI:
        printf("TI Synchronous Serial\n");
        break;
    case FRAME_FORMAT_MICROWIRE:
        printf("Microwire\n");
        break;
    default:
        printf("Unknown (%lu)\n", frame_format);
    }
}

void configure_robot()
{
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
    printf("Processing: %s\n", command);

    char *token = strtok(command, "_");
    bool start_received = false;

    if (token != NULL)
    {
        if (strcmp(token, cmd_update_state) == 0)
        {
            printf("Recieved %s...\n", cmd_update_state);
            token = strtok(NULL, "_");
            process_update_state(token);
        }
        else
        {
            printf("Invalid command ...\n");
            return;
        }
    }
}

void process_update_state(const char *args_token)
{
    RobotState new_state;
    char *args = args_token;

    // Parse stop flag
    if (args != NULL)
    {
        printf("Recieved args %s\n", args);
        new_state.stop = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint direction handling ====================================
    // Joint 1 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 1 direction %s\n", args_token);
        new_state.joint_1_dir = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint 2 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 2 direction %s\n", args_token);
        new_state.joint_2_dir = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint 3 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 3 direction %s\n", args_token);
        new_state.joint_3_dir = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint 4 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 4 direction %s\n", args_token);
        new_state.joint_4_dir = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint 5 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 5 direction %s\n", args_token);
        new_state.joint_5_dir = stringAsBool(args); // +1 to skip the underscore
    }

    // Joint speed handling ====================================

    // Joint 1 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 1 speed %s\n", args_token);
        new_state.joint_1_speed = atoi(args); // +1 to skip the underscore
    }

    // Joint 2 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 2 speed %s\n", args_token);
        new_state.joint_2_speed = atoi(args); // +1 to skip the underscore
    }

    // Joint 3 direction handling
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 3 speed %s\n", args_token);
        new_state.joint_3_speed = atoi(args); // +1 to skip the underscore
    }

    // Gripper state  ====================================
    args = strtok(NULL, "_");

    if (args != NULL)
    {
        // printf("Joint 5 direction %s\n", args_token);
        new_state.gripper_open = stringAsBool(args); // +1 to skip the underscore
    }

    printf("New state Stop Flag: %d\n Directions: J1 %d, J2 %d, J3 %d, J4 %d, J5 %d\n Speeds: J1 %lu, J2 %lu, J3 %lu,\n Gripper: %d",
           new_state.stop,
           new_state.joint_1_dir,
           new_state.joint_2_dir,
           new_state.joint_3_dir,
           new_state.joint_4_dir,
           new_state.joint_5_dir,

           new_state.joint_1_speed,
           new_state.joint_2_speed,
           new_state.joint_3_speed,

           new_state.gripper_open);

    state = new_state;
}

// void update_speed(char *args_token)
// {
//     if (args_token != NULL)
//     {
//         printf(args_token);
//         bool stop = stringAsBool(args_token); // +1 to skip the underscore
//         movement_speed = number;
//     }
// }

// void move_cw(char *args_token)
// {
//     printf("Executing move_cw...\n");
//     move_motor = true;
//     gpio_put(dir_pin, false);
//     sleep_us(6);

//     if (args_token != NULL)
//     {
//         printf(args_token);
//         int number = atoi(args_token); // +1 to skip the underscore
//         movement_speed = number;
//     }

//     while (move_motor)
//     {
//         serial_data_available = get_pi_input();

//         if (serial_data_available)
//         {
//             process_command(input_buffer);
//         }

//         gpio_put(step_pin, false);
//         sleep_us(movement_speed);
//         gpio_put(step_pin, true);
//         sleep_us(movement_speed);
//     }
// }

// void move_ccw(char *args_token)
// {
//     printf("Executing move_ccw...\n");
//     move_motor = true;
//     gpio_put(dir_pin, true);
//     sleep_us(6);

//     if (args_token != NULL)
//     {
//         printf(args_token);
//         int number = atoi(args_token); // +1 to skip the underscore
//         movement_speed = number;
//     }

//     while (move_motor)
//     {
//         serial_data_available = get_pi_input();

//         if (serial_data_available)
//         {
//             process_command(input_buffer);
//         }

//         gpio_put(step_pin, false);
//         sleep_us(movement_speed);
//         gpio_put(step_pin, true);
//         sleep_us(movement_speed);
//     }
// }

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

bool stringAsBool(const char *str)
{
    // if (strcmp(str, "1") == 0)
    // {
    //     printf("Got %s\n", str);
    // }

    return strcmp(str, "1") == 0;
}