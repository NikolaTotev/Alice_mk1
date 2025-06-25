#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include <stdio.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// Your struct definition
typedef struct
{
    bool active;
    bool direction;
    bool direction2;
    uint16_t speed;
} JointPacket;

// SPI0 pins
#define SPI0_SCK 18  // GPIO 2
#define SPI0_MOSI 16 // GPIO 3
#define SPI0_MISO 19 // GPIO 4
#define SPI0_CS 17   // GPIO 5

// SPI settings
#define SPI_FREQ 1000000 // 1 MHz
#define LED_PIN 25

// Frame format definitions (since they're not in the SDK)
#define FRAME_FORMAT_MOTOROLA 0x0
#define FRAME_FORMAT_TI 0x1
#define FRAME_FORMAT_MICROWIRE 0x2

#define LED_PIN 25
#define step_pin 15
#define dir_pin 14
#define driver_ena_pin 13
#define shifter_ena_pin 10

#define servo_1 14
#define servo_2 15

// Queue to send data from core 0 to core 1
queue_t joint_queue;
#define pico_num 3
#define motor_type 1 // 0 - small motor, 1 - big motor, 2 - servo

// Core 1 main function
void core1_main()
{
    JointPacket received_packet;

    if (motor_type == 0 || motor_type == 1)
    {
        gpio_init(step_pin);
        gpio_init(dir_pin);
        gpio_init(shifter_ena_pin);

        gpio_set_dir(step_pin, GPIO_OUT);
        gpio_set_dir(dir_pin, GPIO_OUT);
        gpio_set_dir(shifter_ena_pin, GPIO_OUT);

        gpio_put(step_pin, true);
        gpio_put(dir_pin, true);

        gpio_put(shifter_ena_pin, true);
    }
    gpio_init(LED_PIN);

    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, false);

    if (motor_type == 0)
    {
    }

    if (motor_type == 1)
    {
        gpio_init(driver_ena_pin);
        gpio_set_dir(driver_ena_pin, GPIO_OUT);
        gpio_put(driver_ena_pin, true);
    }

    if (motor_type == 2)
    {
    }

    printf("Core 1 started, waiting for packets...\n");

    while (true)
    {
        // Wait for a packet from core 0
        if (queue_try_remove(&joint_queue, &received_packet))
        {
            printf("PICO %lu >> Core 1 received: active=%d, dir=%d, dir2=%d, speed=%d\n",
                   pico_num,
                   received_packet.active,
                   received_packet.direction,
                   received_packet.direction2,
                   received_packet.speed);
        }
        

        if (received_packet.active)
        {
            if (motor_type == 2)
            {
            }
            else
            {
                gpio_put(dir_pin, received_packet.direction);
                sleep_us(6);
                gpio_put(step_pin, false);
                sleep_us(received_packet.speed);
                gpio_put(step_pin, true);
                sleep_us(received_packet.speed);
            }
        }
    }
}

int main()
{
    // Initialize stdio
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

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

    printf("PICO %lu SPI Slave (SPI0) initialized\n", pico_num);

    // Initialize SPI0
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

    // Set SPI0 to slave mode
    spi_set_slave(spi0, true);

    // Initialize SPI pins
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);

    gpio_set_drive_strength(SPI0_SCK, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MOSI, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MISO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_CS, GPIO_DRIVE_STRENGTH_2MA);

    // Initialize the queue
    // Parameters: queue, element_size, element_count
    queue_init(&joint_queue, sizeof(JointPacket), 10);

    // Launch core 1
    multicore_launch_core1(core1_main);

    // Core 0 main loop
    JointPacket rx_packet;
    uint16_t counter = 0;

    printf("PICO %lu >> Core 0 started, sending packets...\n", pico_num);

    while (true)
    {
        printf("Tryint to read...");
        spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_packet, sizeof(JointPacket));

        // Send the packet to core 1
        if (queue_try_add(&joint_queue, &rx_packet))
        {
            printf("Core 0 sent packet");
        }
        else
        {
            printf("Core 0: Queue full, couldn't send packet\n");
        }
    }

    return 0;
}