#include <stdio.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define LED_PIN 25
#define clk_pin 22
#define rst_pin 21

#define sck 10  // GPIO 10
#define mosi 11 // GPIO 11
#define miso 12 // GPIO 12
#define cs 13   // GPIO 13

int main()
{
    int counter = 0;

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_init(clk_pin);
    gpio_init(rst_pin);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(clk_pin, GPIO_OUT);
    gpio_set_dir(rst_pin, GPIO_OUT);
    gpio_put(LED_PIN, true);
    gpio_put(clk_pin, false);
    gpio_put(rst_pin, false);

    gpio_init(sck);
    gpio_init(mosi);
    // gpio_init(miso);
    gpio_init(cs);

    gpio_set_dir(sck, GPIO_OUT);
    gpio_set_dir(mosi, GPIO_OUT);
    // gpio_set_dir(miso, GPIO_OUT);
    gpio_set_dir(cs, GPIO_OUT);

    gpio_put(sck, true);
    gpio_put(mosi, true);
    // gpio_put(miso, true);
    gpio_put(cs, true);

    while (true)
    {
        if (counter == 0)
        {
            gpio_put(LED_PIN, true);

            for (size_t i = 0; i < 1000; i++)
            {
                gpio_put(cs, true);
                sleep_ms(2);
                gpio_put(cs, false);
                sleep_ms(2);
            }
            gpio_put(LED_PIN, false);

            gpio_put(clk_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(10);
            gpio_put(clk_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(10);
            counter++;
        }
        else if (counter == 1)
        {

            gpio_put(LED_PIN, true);

            for (size_t i = 0; i < 1000; i++)
            {
                gpio_put(cs, true);
                sleep_ms(15);
                gpio_put(cs, false);
                sleep_ms(15);
            }
            gpio_put(LED_PIN, false);

            gpio_put(clk_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(10);
            gpio_put(clk_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(10);
            counter++;
        }
        else if (counter == 2)
        {

            gpio_put(LED_PIN, true);

            for (size_t i = 0; i < 1000; i++)
            {
                gpio_put(cs, true);
                sleep_ms(20);
                gpio_put(cs, false);
                sleep_ms(20);
            }
            gpio_put(LED_PIN, false);

            gpio_put(clk_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(10);
            gpio_put(clk_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(10);
            counter++;
        }
        else if (counter == 3)
        {

            gpio_put(LED_PIN, true);

            for (size_t i = 0; i < 1000; i++)
            {
                gpio_put(cs, true);
                sleep_ms(25);
                gpio_put(cs, false);
                sleep_ms(25);
            }
            gpio_put(LED_PIN, false);

            gpio_put(clk_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(10);
            gpio_put(clk_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(10);
            counter++;
        }
        else
        {

            gpio_put(rst_pin, true);
            gpio_put(LED_PIN, true);
            sleep_ms(10);
            gpio_put(rst_pin, false);
            gpio_put(LED_PIN, false);
            sleep_ms(100);
            counter = 0;
        }
        sleep_ms(25);
    }

    // while (!stdio_usb_connected())
    // {
    //     gpio_put(LED_PIN, true);
    //     sleep_ms(10);
    //     gpio_put(LED_PIN, false);
    //     sleep_ms(10);
    // }

    // while (true)
    // {
    //     gpio_put(LED_PIN, true);
    //     printf("PCB seems to be working!!\n");
    //     sleep_ms(1000);
    //     gpio_put(LED_PIN, false);
    //     sleep_ms(1000);
    // }
}
