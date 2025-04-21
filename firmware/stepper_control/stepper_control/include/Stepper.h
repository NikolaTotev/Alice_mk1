#ifndef STEPPER_CONTROLLER_H
#define STEPPER_CONTROLLER_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

class Stepper {
public:
    // Configuration structure
    struct Config {
        PIO pio;                 // PIO instance to use
        uint sm;                 // State machine number
        uint step_pin;           // GPIO pin for step signal
        uint dir_pin;            // GPIO pin for direction signal
        float clk_div;           // PIO clock divider
    };

    // Constructor
    Stepper();

    // Initialize the stepper controller with configuration
    void init(const Config& config);

    // Update speed (steps/second)
    void setSpeed(float speed);

    // Set direction (true = forward, false = backward)
    void setDirection(bool direction);

    // Process function to be called regularly to maintain motion
    void process();

    // Get current step count
    uint32_t getStepCount() const { return step_count; }

    // Get current direction
    bool getDirection() const { return direction; }

    // Get current speed
    float getSpeed() const { return speed; }

    // Static utility function to convert speed to delay
    static uint32_t speedToDelay(float speed);

private:
    // Hardware configuration
    Config config;
    uint pio_offset;         // Offset of program in PIO memory
    int dma_channel;         // DMA channel used
    
    // DMA buffers for ping-pong operation
    uint32_t buffer_a[32];
    uint32_t buffer_b[32];
    bool use_buffer_a;       // Which buffer to use next
    volatile bool dma_ready; // Flag set by IRQ when DMA is ready
    
    // Motion state
    volatile uint32_t step_count;   // Total steps taken
    bool direction;          // Current direction
    float speed;             // Current speed in steps/second
    uint32_t step_delay;     // Current delay value

    // Helper method to setup DMA transfer
    void setupDmaTransfer(uint32_t* buffer);

    // Static DMA interrupt handler
    static void dmaIrqHandler();
};

#endif // STEPPER_CONTROLLER_H