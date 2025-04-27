#include <cstdio>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "SerialHandler.h"
#include "CoreHandler.h"
#include "MotionControl.h"

int main() {
    // Initialize stdio (USB serial)
    stdio_init_all();
    
    // Small delay to allow connections to establish
    sleep_ms(500);

    // Initialize subsystems
    SerialHandler::init();
    MotionControl::init();

 
    
    // Print welcome message
    SerialHandler::printWelcomeMessage();
    
    // Launch Core 1
    multicore_launch_core1(CoreHandler::core1Entry);
    
    // Run Core 0 main function (never returns)
    CoreHandler::core0Main();
    printf("s");
    
    return 0;
}