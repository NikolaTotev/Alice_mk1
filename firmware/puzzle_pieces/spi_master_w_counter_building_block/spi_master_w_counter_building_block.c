#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// Define the data structure to send
typedef struct
{
    uint32_t num1;
    uint32_t num2;
} DataPacket;

// SPI1 pins
#define SPI1_SCK 10  // GPIO 10
#define SPI1_MOSI 11 // GPIO 11
#define SPI1_MISO 12 // GPIO 12
#define SPI1_CS 13   // GPIO 13

// SPI settings
#define SPI_FREQ 1000000 // 1 MHz
#define LED_PIN 25

#define clk_pin 0
#define rst_pin 1

int main()
{
    int counter = 0;
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);


    
    gpio_init(clk_pin);
    gpio_init(rst_pin);


    gpio_set_dir(clk_pin, GPIO_OUT);
    gpio_set_dir(rst_pin, GPIO_OUT);


    gpio_put(clk_pin, false);
    gpio_put(rst_pin, false);

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

    printf("SPI Master (SPI1) initialized\n");

    // Initialize SPI1
    spi_init(spi1, SPI_FREQ);

    // Set SPI format
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Initialize SPI pins
    gpio_set_function(SPI1_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_CS, GPIO_FUNC_SPI);

    // Create data packet
    DataPacket tx_data;
    DataPacket rx_data;

    gpio_put(rst_pin, true);
    gpio_put(LED_PIN, true);
    sleep_ms(1);
    gpio_put(rst_pin, false);
    gpio_put(LED_PIN, false);
    sleep_ms(1);

    while (true)
    {
        if (counter < 1)
        {
            // Prepare data to send
            tx_data.num1 = 111;
            tx_data.num2 = 111;

            printf("Sending to pico 1: num1=%lu, num2=%lu\n", tx_data.num1, tx_data.num2);

            // Pull CS low to start transmission
            // gpio_put(SPI1_CS, 0);
            sleep_us(10); // Small delay for slave to detect CS

            // Send data first
            spi_write_blocking(spi1, (uint8_t *)&tx_data, sizeof(DataPacket));

            // Small delay between write and read (optional)
            sleep_us(10);

            // Read response data
            spi_read_blocking(spi1, 0xFF, (uint8_t *)&rx_data, sizeof(DataPacket));

            // spi_write_read_blocking(spi1, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

            // Pull CS high to end transmission
            // gpio_put(SPI1_CS, 1);

            // Print received data
            printf("Received from pico 1: num1=%lu, num2=%lu\n\n", rx_data.num1, rx_data.num2);

            printf("Incrementing, %d\n", counter);
            gpio_put(clk_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(1);
            gpio_put(clk_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(1);
            counter++;
        }
        else
        {
            // Prepare data to send
            tx_data.num1 = 222;
            tx_data.num2 = 222;

            printf("Sending to pico 2: num1=%lu, num2=%lu\n", tx_data.num1, tx_data.num2);

            // Pull CS low to start transmission
            // gpio_put(SPI1_CS, 0);
            sleep_us(10); // Small delay for slave to detect CS

            // Send data first
            spi_write_blocking(spi1, (uint8_t *)&tx_data, sizeof(DataPacket));

            // Small delay between write and read (optional)
            sleep_us(10);

            // Read response data
            spi_read_blocking(spi1, 0xFF, (uint8_t *)&rx_data, sizeof(DataPacket));

            // spi_write_read_blocking(spi1, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

            // Pull CS high to end transmission
            // gpio_put(SPI1_CS, 1);

            // Print received data
            printf("Received from pico 2: num1=%lu, num2=%lu\n\n", rx_data.num1, rx_data.num2);

            printf("Counter is zero");
            gpio_put(rst_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(1);
            gpio_put(rst_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(1);
            counter = 0;
        }
        sleep_ms(1000);
    }

    return 0;
}