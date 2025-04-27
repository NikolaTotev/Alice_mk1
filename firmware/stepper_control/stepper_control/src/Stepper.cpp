#include "Stepper.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include <cmath>
#include <jog.pio.h>
#include <cstdio>

// Static instance pointer for interrupt handler to access the correct instance
static Stepper *active_instance = nullptr;

// Constructor
Stepper::Stepper() : pio_offset(0),
                     dma_channel(-1),
                     use_buffer_a(true),
                     dma_ready(true),
                     step_count(0),
                     direction(true),
                     speed(0.0f),
                     step_delay(0)
{
    // Initialize buffers to zero
    for (int i = 0; i < 32; i++)
    {
        buffer_a[i] = 0;
        buffer_b[i] = 0;
    }
}

// Initialize the stepper controller with configuration
void Stepper::init(const Config &config)
{
    this->config = config;

    // Store the instance for the IRQ handler
    active_instance = this;

    // Configure GPIO pin for PIO
    gpio_set_function(config.step_pin, GPIO_FUNC_PIO0);

    gpio_init(config.shifter_ena_pin);
    gpio_set_dir(config.shifter_ena_pin, GPIO_OUT);
    gpio_put(config.shifter_ena_pin, 1);

    sleep_ms(5);

    gpio_init(config.driver_ena_pin);
    gpio_set_dir(config.driver_ena_pin, GPIO_OUT);
    gpio_put(config.driver_ena_pin, 1);

    sleep_ms(5);

    // Set initial direction
    gpio_init(config.dir_pin);
    gpio_set_dir(config.dir_pin, GPIO_OUT);
    gpio_put(config.dir_pin, 1);

    sleep_ms(100);

    // Load PIO program
    pio_offset = pio_add_program(config.pio, &jog_program);

    // Initialize and configure PIO state machine
    // pio_sm_config sm_config = jog_program_get_default_config(pio_offset);
    // sm_config_set_clkdiv(&sm_config, config.clk_div);
    // sm_config_set_out_pins(&sm_config, config.step_pin, 1);
    // pio_sm_init(config.pio, config.sm, pio_offset, &sm_config);

    jog_program_init(this->config.pio, this->config.sm, pio_offset, this->config.step_pin, this->config.clk_div);
    // Start PIO state machine
    pio_sm_set_enabled(this->config.pio, this->config.sm, true);
    // this->config.pio->txf[this->config.sm] = 30000;

    // Claim DMA channel
    dma_channel = dma_claim_unused_channel(true);

    // Configure DMA interrupt
    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dmaIrqHandler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Initialize with stopped motion
    setSpeed(0);
}

void Stepper::jogCW(uint32_t speed)
{
    printf("Starting jogging!");
    gpio_put(config.dir_pin, 1);
    pio_sm_put_blocking(this->config.pio, this->config.sm, speed - 3);
}

void Stepper::jogCCW(uint32_t speed)
{
    gpio_put(config.dir_pin, 1);
    pio_sm_put_blocking(this->config.pio, this->config.sm, speed - 3);
}

void Stepper::stopJog()
{
    pio_sm_put_blocking(this->config.pio, this->config.sm, 0);
}

// Update speed (steps/second)
void Stepper::setSpeed(float new_speed)
{
    speed = fabsf(new_speed); // Speed is always positive

    // Update direction if needed
    if (new_speed < 0 && direction)
    {
        setDirection(false);
    }
    else if (new_speed > 0 && !direction)
    {
        setDirection(true);
    }

    // Convert speed to delay value
    step_delay = speedToDelay(speed);

    // // Fill buffers with new delay value
    // for (int i = 0; i < 32; i++)
    // {
    //     buffer_a[i] = step_delay;
    //     buffer_b[i] = step_delay;
    // }
}

// Set direction (true = forward, false = backward)
void Stepper::setDirection(bool new_direction)
{
    direction = new_direction;
    gpio_put(config.dir_pin, direction);
}

// Process function to be called regularly to maintain motion
void Stepper::process()
{
    // Skip if DMA is still busy
    if (!dma_ready)
    {
        return;
    }

    // Skip if speed is zero (no motion)
    if (speed == 0.0f)
    {
        return;
    }

    // Setup and start next DMA transfer
    dma_ready = false;

    if (use_buffer_a)
    {
        setupDmaTransfer(buffer_a);
    }
    else
    {
        setupDmaTransfer(buffer_b);
    }

    use_buffer_a = !use_buffer_a; // Swap buffers for next time
}

// Helper method to setup DMA transfer
void Stepper::setupDmaTransfer(uint32_t *buffer)
{
    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, pio_get_dreq(config.pio, config.sm, true));

    dma_channel_configure(
        dma_channel,
        &dma_config,
        &config.pio->txf[config.sm], // Destination is PIO FIFO
        buffer,                      // Source is our buffer
        32,                          // Transfer count
        true                         // Start immediately
    );
}

// Convert speed (steps/second) to delay value
uint32_t Stepper::speedToDelay(float speed)
{
    if (speed <= 0.0f)
    {
        return 0; // Stopped
    }

    // Calculate delay based on clock frequency and desired speed
    float system_clock = clock_get_hz(clk_sys);
    return static_cast<uint32_t>((system_clock / speed) - 20); // Adjust for overhead
}

// Static DMA interrupt handler
void Stepper::dmaIrqHandler()
{
    // Clear the interrupt
    dma_hw->ints0 = 1u << active_instance->dma_channel;

    // Update step count (32 steps per DMA transfer)
    active_instance->step_count += 32;

    // Signal that DMA is ready for next transfer
    active_instance->dma_ready = true;
}