#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/vreg.h"

// // Function to set system clock to a specific frequency
// bool set_sys_clock_khz(uint32_t freq_khz, bool required) {
//     uint32_t freq_hz = freq_khz * 1000;
    
//     // Calculate PLL settings
//     // VCO frequency should be between 400MHz and 1600MHz
//     // We'll use a simple approach: find a suitable multiplier
    
//     uint32_t ref_freq = 12000000; // 12MHz crystal
//     uint32_t vco_freq;
//     uint32_t post_div1, post_div2;
    
//     // Try different post dividers to find valid configuration
//     for (post_div1 = 7; post_div1 >= 1; post_div1--) {
//         for (post_div2 = 7; post_div2 >= 1; post_div2--) {
//             vco_freq = freq_hz * post_div1 * post_div2;
//             if (vco_freq >= 400000000 && vco_freq <= 1600000000) {
//                 uint32_t feedback_div = vco_freq / ref_freq;
//                 if (feedback_div >= 16 && feedback_div <= 320) {
//                     // Valid configuration found
//                     pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
//                     clock_configure(clk_sys,
//                                   CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
//                                   CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
//                                   freq_hz, freq_hz);
//                     return true;
//                 }
//             }
//         }
//     }
//     return false;
// }

// Predefined overclock configurations
void overclock_250mhz() {
    // Increase voltage for stability at higher frequencies
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10); // Allow voltage to settle
    
    // Set to 250MHz
    set_sys_clock_khz(250000, true);
    
    // Update clock-dependent peripherals
    clock_configure(clk_peri, 0,
                   CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                   250 * MHZ, 250 * MHZ);
}

void overclock_200mhz() {
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(10);
    
    set_sys_clock_khz(200000, true);
    
    clock_configure(clk_peri, 0,
                   CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                   200 * MHZ, 200 * MHZ);
}

void overclock_300mhz() {
    // Higher voltage needed for 300MHz
    vreg_set_voltage(VREG_VOLTAGE_1_25);
    sleep_ms(10);
    
    set_sys_clock_khz(300000, true);
    
    clock_configure(clk_peri, 0,
                   CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                   300 * MHZ, 300 * MHZ);
}

void overclock_400mhz() {
    // Maximum voltage for extreme overclocking
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    sleep_ms(15); // Longer settle time for max voltage
    
    set_sys_clock_khz(400000, true);
    
    clock_configure(clk_peri, 0,
                   CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                   400 * MHZ, 400 * MHZ);
}

// Alternative using Pico SDK's built-in function
void simple_overclock() {
    // Boost voltage first
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    
    // Use SDK function (available in newer SDK versions)
    set_sys_clock_khz(250000, true);
    
    // Reconfigure peripheral clock
    clock_configure(clk_peri, 0,
                   CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                   250000000, 250000000);
}

int main() {
    // Initialize standard I/O (uses default 125MHz initially)
    stdio_init_all();
    
    printf("Current frequency: %lu Hz\n", clock_get_hz(clk_sys));
    
    // Choose your overclock method:
    
    // Method 1: Use predefined configurations
    overclock_300mhz();
    
    // Method 2: Use simple SDK approach
    // simple_overclock();
    
    // Method 3: Custom frequency
    // vreg_set_voltage(VREG_VOLTAGE_1_15);
    // sleep_ms(10);
    // set_sys_clock_khz(200000, true);
    
    printf("New frequency: %lu Hz\n", clock_get_hz(clk_sys));
    
    // Your main program loop
    while (true) {
        printf("New frequency: %lu Hz\n", clock_get_hz(clk_sys));
        printf("Running at overclocked speed!\n");
        sleep_ms(1000);
    }
    
    return 0;
}