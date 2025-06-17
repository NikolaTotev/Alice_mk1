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

// spi0 pins
#define SPI0_SCK 18  // GPIO 10
#define SPI0_MOSI 19 // GPIO 11
#define SPI0_MISO 16 // GPIO 12
#define SPI0_CS 17   // GPIO 13

// SPI settings
#define SPI_FREQ 1000000 // 1 MHz
#define LED_PIN 25

#define clk_pin 22
#define rst_pin 21

// Frame format definitions (since they're not in the SDK)
#define FRAME_FORMAT_MOTOROLA 0x0
#define FRAME_FORMAT_TI 0x1
#define FRAME_FORMAT_MICROWIRE 0x2

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

int main()
{
    int counter = 1;
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

    printf("SPI Master (spi0) initialized\n");

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

    print_spi_config();

    // Initialize SPI pins
    gpio_set_function(SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CS, GPIO_FUNC_SPI);

    gpio_set_drive_strength(SPI0_SCK, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MOSI, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_MISO, GPIO_DRIVE_STRENGTH_2MA);
    gpio_set_drive_strength(SPI0_CS, GPIO_DRIVE_STRENGTH_2MA);

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
        if (counter <4)
        {
            // Prepare data to send
            tx_data.num1 = 111*counter;
            tx_data.num2 = 111*counter;

            printf("Sending to pico %lu: num1=%lu, num2=%lu\n", counter, tx_data.num1, tx_data.num2);

            // Pull CS low to start transmission
            // gpio_put(SPI0_CS, 0);
            sleep_us(10); // Small delay for slave to detect CS

            // Send data first
            spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(DataPacket));

            // Small delay between write and read (optional)
            sleep_us(10);

            // Read response data
            spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_data, sizeof(DataPacket));

            // spi_write_read_blocking(spi0, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

            // Pull CS high to end transmission
            // gpio_put(SPI0_CS, 1);

            // Print received data
            printf("Received from pico %lu: num1=%lu, num2=%lu\n", counter, rx_data.num1, rx_data.num2);

            printf("Incrementing, %d\n\n", counter);
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
            tx_data.num1 = 111*counter;
            tx_data.num2 = 111*counter;

            printf("Sending to pico %lu: num1=%lu, num2=%lu\n",counter, tx_data.num1, tx_data.num2);

            // Pull CS low to start transmission
            // gpio_put(SPI0_CS, 0);
            sleep_us(10); // Small delay for slave to detect CS

            // Send data first
            spi_write_blocking(spi0, (uint8_t *)&tx_data, sizeof(DataPacket));

            // Small delay between write and read (optional)
            sleep_us(10);

            // Read response data
            spi_read_blocking(spi0, 0xFF, (uint8_t *)&rx_data, sizeof(DataPacket));

            // spi_write_read_blocking(spi0, (uint8_t *)&tx_data, (uint8_t *)&rx_data, sizeof(DataPacket));

            // Pull CS high to end transmission
            // gpio_put(SPI0_CS, 1);

            // Print received data
            printf("Received from pico %lu: num1=%lu, num2=%lu\n", counter, rx_data.num1, rx_data.num2);

            printf("Resetting counter \n\n");
            gpio_put(rst_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(1);
            gpio_put(rst_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(1);
            counter = 1;
        }
        sleep_ms(1000);
    }

    return 0;
}
