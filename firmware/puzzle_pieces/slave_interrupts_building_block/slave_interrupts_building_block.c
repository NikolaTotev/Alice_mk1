#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include <stdio.h>

// GPIO pin definitions
#define CONTROL_PIN 2    // GPIO pin receiving control signal from master
#define LED_PIN 25       // Built-in LED for visual feedback (optional)

// Global variables
volatile bool stop_flag = false;  // Controls the print loop
volatile bool running = true;     // Overall program control

// Interrupt handler for control signal changes
void gpio_irq_handler(uint gpio, uint32_t events) {
    if (gpio == CONTROL_PIN) {
        // Read current state of control pin
        bool pin_state = gpio_get(CONTROL_PIN);
        
        if (pin_state == false) {
            // Signal went LOW - stop the loop
            stop_flag = true;
            gpio_put(LED_PIN, false);  // Turn off LED
            printf("Control signal LOW - stopping loop\n");
        } else {
            // Signal went HIGH - resume the loop
            stop_flag = false;
            gpio_put(LED_PIN, true);   // Turn on LED
            printf("Control signal HIGH - resuming loop\n");
        }
    }
}

int main() {
    // Initialize stdio for printf output
    stdio_init_all();
    
    // Wait a moment for USB connection
    sleep_ms(2000);
    printf("Slave Pico starting...\n");
    
    // Initialize control pin as input with pull-up
    gpio_init(CONTROL_PIN);
    gpio_set_dir(CONTROL_PIN, GPIO_IN);
    gpio_pull_up(CONTROL_PIN);  // Default high state
    
    // Initialize LED pin as output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);  // Start with LED on (running state)
    
    // Set up interrupt for control pin
    // Trigger on both rising and falling edges
    gpio_set_irq_enabled_with_callback(CONTROL_PIN, 
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                                       true, 
                                       &gpio_irq_handler);
    
    printf("Slave ready - waiting for control signals...\n");
    
    // Main loop counter for demonstration
    uint32_t loop_counter = 0;
    
    // Main infinite loop
    while (running) {
        // Only print when stop_flag is false
        if (!stop_flag) {
            printf("Loop iteration: %lu\n", loop_counter++);
            sleep_ms(1000);  // 1 second delay between prints
        } else {
            // When stopped, just yield CPU time
            gpio_put(LED_PIN, false);  // Start with LED on (running state)
            sleep_ms(100);   // Short sleep to prevent busy waiting
            gpio_put(LED_PIN, true);  // Start with LED on (running state)
            sleep_ms(100);   // Short sleep to prevent busy waiting
        }
    }
    
    return 0;
}