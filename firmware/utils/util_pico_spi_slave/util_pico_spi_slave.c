// /**
//  * Slave Pico SPI Example
//  * 
//  * Receives messages from master Pico via SPI
//  * Blinks LED when expected message is received and sends a response
//  */
// #include <stdio.h>
// #include <string.h>
// #include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "hardware/gpio.h"

// // SPI configurations - adjust for each slave as needed
// #define SPI_PORT spi0
// #define PIN_MISO 16  // Master in, slave out
// #define PIN_CS   20  // Chip select (this would be 17, 18, or 19 depending on which slave)
// #define PIN_SCK  18  // Clock
// #define PIN_MOSI 19  // Master out, slave in

// // LED pin
// #define LED_PIN PICO_DEFAULT_LED_PIN

// // Expected message from master (change for each slave)
// const uint8_t expected_message[] = "Hello Slave 1"; // Change to appropriate message for each slave

// // Response to send back (change for each slave)
// const uint8_t response[] = "Response from Slave 1"; // Change to appropriate response for each slave

// // Buffer for receiving messages
// uint8_t rx_buffer[64];

// void blink_led(int count, int delay_ms) {
//     for (int i = 0; i < count; i++) {
//         gpio_put(LED_PIN, 1);
//         sleep_ms(delay_ms);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(delay_ms);
//     }
// }

// // This function will be called when chip select goes low (slave selected)
// void cs_callback(uint gpio, uint32_t events) {
//     if (gpio == PIN_CS && (events & GPIO_IRQ_EDGE_FALL)) {
//         // CS went low, we are selected
        
//         // Read the incoming message
//         memset(rx_buffer, 0, sizeof(rx_buffer));
//         spi_read_blocking(SPI_PORT, 0, rx_buffer, sizeof(expected_message));
        
//         // Check if the message matches what we expect
//         bool match = true;
//         for (int i = 0; i < sizeof(expected_message); i++) {
//             if (rx_buffer[i] != expected_message[i]) {
//                 match = false;
//                 break;
//             }
//         }
        
//         // If message matches expected, blink LED and send response
//         if (match) {
//             blink_led(1, 100);
//             printf("Received expected message from master\n");
            
//             // Small delay before sending response
//             sleep_ms(50);
            
//             // Send response
//             spi_write_blocking(SPI_PORT, response, sizeof(response));
//         } else {
//             printf("Unexpected message received\n");
//         }
//     }
// }

// int main() {
//     // Initialize standard I/O
//     stdio_init_all();
    
//     // Initialize LED pin
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
    
//     // Initialize SPI in slave mode
//     spi_init(SPI_PORT, 1000 * 1000); // 1 MHz clock rate
//     spi_set_slave(SPI_PORT, true);
    
//     // Set SPI format
//     spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
//     // Set SPI pins
//     gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
//     gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
//     gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
//     // Configure CS pin with pull-up and interrupt
//     gpio_init(PIN_CS);
//     gpio_set_dir(PIN_CS, GPIO_IN);
    
//     // Set up interrupt for CS pin
//     gpio_set_irq_enabled_with_callback(PIN_CS, GPIO_IRQ_EDGE_FALL, true, &cs_callback);
    
//     printf("SPI Slave initialized. Waiting for master...\n");
    
//     // Main loop - just sleep, the interrupt will handle communication
//     while (true) {
//         sleep_ms(1000);
//     }
    
//     return 0;
// }


#include <stdio.h>
#include <string.h>
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
#warning spi/spi_slave example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI slave example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    spi_set_slave(spi_default, true);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    uint8_t out_buf[BUF_LEN], in_buf[BUF_LEN];

    // Initialize output buffer
    for (size_t i = 0; i < BUF_LEN; ++i) {
        // bit-inverted from i. The values should be: {0xff, 0xfe, 0xfd...}
        out_buf[i] = 24;
    }

    printf("SPI slave says: When reading from MOSI, the following buffer will be written to MISO:\n");
    printbuf(out_buf, BUF_LEN);
    
    for (size_t i = 0; ; ++i) {
        
        // Write the output buffer to MISO, and at the same time read from MOSI.
        spi_write_read_blocking(spi_default, out_buf, in_buf, BUF_LEN);

        // Write to stdio whatever came in on the MOSI line.
        printf("SPI slave says: read page %d from the MOSI line:\n", i);
        printbuf(in_buf, BUF_LEN);
        printf("\n");
    }
#endif
}