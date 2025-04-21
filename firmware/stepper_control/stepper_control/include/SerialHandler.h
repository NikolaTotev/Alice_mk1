#include "CommandProcessor.h"
#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

class SerialHandler {
public:
    // Initialize the serial communication system
    static void init();
    
    // Process any incoming serial data
    static void process();
    
    // Print the welcome message
    static void printWelcomeMessage();
    
private:
    // Private constructor - this is a static utility class
    SerialHandler() {}
    
    // Buffer for command input
    static char cmd_buffer[MAX_CMD_SIZE];
    static int idx;
};

#endif // SERIAL_HANDLER_H