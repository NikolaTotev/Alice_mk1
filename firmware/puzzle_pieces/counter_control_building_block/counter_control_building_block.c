#include <stdio.h>
#include "pico/stdlib.h"

#define clk_pin 16
#define rst_pin 0
#define led_pin 25

int main()
{
    int counter = 0;
    stdio_init_all();

    stdio_init_all();

    gpio_init(clk_pin);
    gpio_init(rst_pin);
    gpio_init(led_pin);

    gpio_set_dir(clk_pin, GPIO_OUT);
    gpio_set_dir(rst_pin, GPIO_OUT);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_put(clk_pin, false);
    gpio_put(rst_pin, false);
    gpio_put(led_pin, false);

    // Wait for USB connection
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    gpio_put(rst_pin, true);
    gpio_put(led_pin, true);
    sleep_ms(1);
    gpio_put(rst_pin, false);
    gpio_put(led_pin, false);
    sleep_ms(1);

    while (true)
    {
        if (counter < 3)
        {
            printf("Incrementing, %d\n", counter);
            gpio_put(clk_pin, true);
            gpio_put(led_pin, true);
            sleep_ms(1);
            gpio_put(clk_pin, false);
            gpio_put(led_pin, false);
            sleep_ms(1);
            counter++;
        }
        else
        {
            printf("Counter is zero");
            gpio_put(rst_pin, true);
            gpio_put(led_pin, true);
            sleep_ms(1);
            gpio_put(rst_pin, false);
            sleep_ms(1);
            counter = 0;
        }
        sleep_ms(5);
    }
}
