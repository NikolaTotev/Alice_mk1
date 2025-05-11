// /**
//  * Master Pico SPI Example
//  * 
//  * Connects to 3 slave Picos via SPI and sends messages sequentially
//  * Blinks LED when message is sent and when correct response is received
//  */
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "hardware/gpio.h"

// // SPI configurations
// #define SPI_PORT spi0
// #define PIN_MISO 16
// #define PIN_CS1  20  // Chip select for slave 1
// #define PIN_CS2  21  // Chip select for slave 2
// #define PIN_CS3  17  // Chip select for slave 3
// #define PIN_SCK  18  // Clock
// #define PIN_MOSI 19  // Master out, slave in

// // LED pin
// #define LED_PIN PICO_DEFAULT_LED_PIN

// // Messages to send to each slave
// const uint8_t message_to_slave1[] = "Hello Slave 1";
// const uint8_t message_to_slave2[] = "Hello Slave 2";
// const uint8_t message_to_slave3[] = "Hello Slave 3";

// // Expected responses from each slave
// const uint8_t expected_response1[] = "Response from Slave 1";
// const uint8_t expected_response2[] = "Response from Slave 2";
// const uint8_t expected_response3[] = "Response from Slave 3";

// // Buffer for receiving responses
// uint8_t rx_buffer[64];

// void blink_led(int count, int delay_ms) {
//     for (int i = 0; i < count; i++) {
//         gpio_put(LED_PIN, 1);
//         sleep_ms(delay_ms);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(delay_ms);
//     }
// }

// bool send_message_to_slave(int cs_pin, const uint8_t* message, size_t message_len, 
//                           const uint8_t* expected_resp, size_t resp_len) {
//     // Select the slave by setting its CS pin low
//     gpio_put(cs_pin, 0);
//     sleep_ms(10); // Small delay to ensure CS is registered

//     // Send the message
//     spi_write_blocking(SPI_PORT, message, message_len);
    
//     // Blink LED once - message sent
//     blink_led(1, 100);
    
//     // Small delay before reading response
//     sleep_ms(100);
    
//     // Read response
//     for (int i = 0; i < resp_len; i++) {
//         rx_buffer[i] = 0;
//     }
//     spi_read_blocking(SPI_PORT, 0, rx_buffer, resp_len);
    
//     // Deselect the slave
//     gpio_put(cs_pin, 1);
    
//     // Check if response matches expected
//     bool match = true;
//     for (int i = 0; i < resp_len; i++) {
//         if (rx_buffer[i] != expected_resp[i]) {
//             match = false;
//             break;
//         }
//     }
    
//     // If response matches expected, blink LED again
//     if (match) {
//         blink_led(1, 100);
//         printf("Received correct response from slave\n");
//     } else {
//         printf("Incorrect response received\n");
//     }
    
//     return match;
// }

// int main() {
//     // Initialize standard I/O
//     stdio_init_all();
    
//     // Initialize LED pin
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
    
//     // Initialize chip select pins
//     gpio_init(PIN_CS1);
//     gpio_init(PIN_CS2);
//     gpio_init(PIN_CS3);
//     gpio_set_dir(PIN_CS1, GPIO_OUT);
//     gpio_set_dir(PIN_CS2, GPIO_OUT);
//     gpio_set_dir(PIN_CS3, GPIO_OUT);
    
//     // Set CS pins high (inactive) initially
//     gpio_put(PIN_CS1, 1);
//     gpio_put(PIN_CS2, 1);
//     gpio_put(PIN_CS3, 1);
    
//     // Initialize SPI
//     spi_init(SPI_PORT, 1000 * 1000); // 1 MHz clock rate
    
//     // Set SPI format
//     spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
//     // Set SPI pins
//     gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
//     gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
//     gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
//     printf("SPI Master initialized. Starting communication with slaves...\n");
    
//     // Main loop
//     while (true) {
//         // Send message to Slave 1
//         printf("Sending message to Slave 1...\n");
//         send_message_to_slave(PIN_CS1, message_to_slave1, sizeof(message_to_slave1), 
//                              expected_response1, sizeof(expected_response1));
//         sleep_ms(1000);
        
//         // // Send message to Slave 2
//         // printf("Sending message to Slave 2...\n");
//         // send_message_to_slave(PIN_CS2, message_to_slave2, sizeof(message_to_slave2), 
//         //                      expected_response2, sizeof(expected_response2));
//         // sleep_ms(1000);
        
//         // // Send message to Slave 3
//         // printf("Sending message to Slave 3...\n");
//         // send_message_to_slave(PIN_CS3, message_to_slave3, sizeof(message_to_slave3), 
//         //                      expected_response3, sizeof(expected_response3));
//         // sleep_ms(1000);
//     }
    
//     return 0;
// }

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define BUF_LEN         0x6

void printbuf(uint8_t buf[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

int main() {
    // Enable UART so we can print
    stdio_init_all();
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_master example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI master example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    uint8_t out_buf[BUF_LEN], in_buf[BUF_LEN];

    // Initialize output buffer
    for (size_t i = 0; i < BUF_LEN; ++i) {
        out_buf[i] = 42;
    }

    printf("SPI master says: The following buffer will be written to MOSI endlessly:\n");
    printbuf(out_buf, BUF_LEN);

    for (size_t i = 0; ; ++i) {
        // Write the output buffer to MOSI, and at the same time read from MISO.
        spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);

        // Write to stdio whatever came in on the MISO line.
        printf("SPI master says: read page %d from the MISO line:\n", i);
        printbuf(in_buf, BUF_LEN);
        printf("\n");

        // Sleep for ten seconds so you get a chance to read the output.
        sleep_ms(10 * 1000);
    }
#endif
}