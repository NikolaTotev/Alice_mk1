#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

// GPIO pin definitions
#define CONTROL_OUTPUT_PIN 2  // GPIO pin sending control signal to slave
#define LED_PIN 25           // Built-in LED for status indication

// Global variables
volatile bool control_state = true;   // Current state of control signal (default HIGH)

void toggle_control_signal() {
    // Toggle the control state
    control_state = !control_state;
    
    // Set the GPIO output
    gpio_put(CONTROL_OUTPUT_PIN, control_state);
    
    // Update LED to show current state
    gpio_put(LED_PIN, control_state);
    
    // Print status
    printf("Control signal set to: %s\n", control_state ? "HIGH" : "LOW");
}

void set_control_signal(bool state) {
    // Set specific control state
    control_state = state;
    
    // Set the GPIO output
    gpio_put(CONTROL_OUTPUT_PIN, control_state);
    
    // Update LED to show current state
    gpio_put(LED_PIN, control_state);
    
    // Print status
    printf("Control signal set to: %s\n", control_state ? "HIGH" : "LOW");
}

int main() {
    // Initialize stdio for printf output
    stdio_init_all();
    
    // Wait a moment for USB connection
        // Initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);  // Start with LED on (HIGH state)

    sleep_ms(5000);
    gpio_put(LED_PIN, false);  // Start with LED on (HIGH state)
    printf("Master Pico starting...\n");
    
    // Initialize control output pin
    gpio_init(CONTROL_OUTPUT_PIN);
    gpio_set_dir(CONTROL_OUTPUT_PIN, GPIO_OUT);
    gpio_put(CONTROL_OUTPUT_PIN, true);  // Start HIGH (default state)
    

    
    printf("Master ready - Console control enabled\n");
    printf("Commands:\n");
    printf("  'stop' - Set control signal LOW (stop slave)\n");
    printf("  'start' - Set control signal HIGH (start slave)\n");
    printf("  'toggle' - Toggle control signal\n");
    printf("  'status' - Show current signal state\n");
    printf("Control signal starts HIGH\n\n");
    
    char input_buffer[32];
    int buffer_index = 0;
    
    // Main loop
    while (true) {
        // Check for console input
        int c = getchar_timeout_us(1000);  // 1ms timeout
        
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '>') {
                // End of command - process it
                input_buffer[buffer_index] = '\0';  // Null terminate
                
                if (strlen(input_buffer) > 0) {
                    // Process the command
                    if (strcmp(input_buffer, "stop") == 0) {
                        set_control_signal(false);
                        printf("Slave stopped.\n");
                    }
                    else if (strcmp(input_buffer, "start") == 0) {
                        set_control_signal(true);
                        printf("Slave started.\n");
                    }
                    else if (strcmp(input_buffer, "toggle") == 0) {
                        toggle_control_signal();
                        printf("Signal toggled.\n");
                    }
                    else if (strcmp(input_buffer, "status") == 0) {
                        printf("Current control signal: %s\n", control_state ? "HIGH" : "LOW");
                    }
                    else {
                        printf("Unknown command: '%s'\n", input_buffer);
                        printf("Valid commands: stop, start, toggle, status\n");
                    }
                }
                
                // Reset buffer
                buffer_index = 0;
                printf("> ");  // Prompt for next command
            }
            else if (c == 8 || c == 127) {  // Backspace or DEL
                if (buffer_index > 0) {
                    buffer_index--;
                    printf("\b \b");  // Erase character from terminal
                }
            }
            else if (buffer_index < sizeof(input_buffer) - 1) {
                // Add character to buffer
                input_buffer[buffer_index++] = (char)c;
                printf("%c", c);  // Echo character
            }
        }
        
        // Small delay to prevent busy waiting
        sleep_ms(10);
    }
    
    return 0;
}