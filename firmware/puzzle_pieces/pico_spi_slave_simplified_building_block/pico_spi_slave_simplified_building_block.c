#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// Define the data structure
typedef struct
{
    uint32_t num1;
    uint32_t num2;
} DataPacket;

// SPI0 pins
#define SPI0_SCK 18  // GPIO 2
#define SPI0_MOSI 16 // GPIO 3
#define SPI0_MISO 19 // GPIO 4
#define SPI0_CS 17   // GPIO 5

// SPI settings
#define SPI_FREQ 1000000 // 1 MHz
#define LED_PIN 25

// Global variables for interrupt handling
volatile bool data_ready = false;
DataPacket rx_data;
DataPacket tx_data;

// Interrupt handler for CS pin
// void cs_interrupt_handler(uint gpio, uint32_t events)
// {
//     if (gpio == SPI0_CS)
//     {
//         if (events & GPIO_IRQ_EDGE_FALL)
//         {
//             // CS went low - prepare to receive data
//             // The actual SPI transfer will happen in the main loop
//             data_ready = false;
//         }
//         else if (events & GPIO_IRQ_EDGE_RISE)
//         {
//             // CS went high - transmission complete
//             data_ready = true;
//         }
//     }
// }

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

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

    printf("SPI Slave (SPI0) initialized\n");

    // Initialize SPI0
    spi_init(spi0, SPI_FREQ);

    // Set SPI format
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Set SPI0 to slave mode
    spi_set_slave(spi0, true);

    // Initialize SPI pins
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);

    // Set up interrupt on CS pin
    // gpio_set_irq_enabled_with_callback(SPI0_CS, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
    //                                    true, &cs_interrupt_handler);

    uint32_t response_counter = 1000;

    while (true)
    {
        // Check if CS is low (active)
        // if (!gpio_get(SPI0_CS))
        // {
        printf("Waiting to read...");
        gpio_put(LED_PIN, true);
        // First, read incoming data from master
        spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_data, sizeof(DataPacket));

        // Prepare response data
        tx_data.num1 = 2222;
        tx_data.num2 = 2222;

        // // Then send response data back
        spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(DataPacket));

        //spi_write_read_blocking(spi0, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

        // Wait for CS to go high
        // while (!gpio_get(SPI0_CS))
        // {
        //     tight_loop_contents();
        // }
        gpio_put(LED_PIN, false);

        // Process received data
        printf("Received: num1=%lu, num2=%lu\n", rx_data.num1, rx_data.num2);
        printf("Sent back: num1=%lu, num2=%lu\n\n", tx_data.num1, tx_data.num2);

        response_counter++;
        //}

        // Small delay to prevent busy waiting
        sleep_us(10);
    }

    return 0;
}