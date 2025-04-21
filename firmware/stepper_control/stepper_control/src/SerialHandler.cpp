#include <cstdio>
#include "pico/stdlib.h"

#include "SerialHandler.h"
#include "CommandProcessor.h"

// Initialize static members
char SerialHandler::cmd_buffer[MAX_CMD_SIZE];
int SerialHandler::idx = 0;

// Initialize serial communication
void SerialHandler::init() {
    // Already initialized in main with stdio_init_all()
}

// Print welcome message
void SerialHandler::printWelcomeMessage() {
    printf("RP2040 Dual Core Demo started\n");
    printf("Available commands:\n");
    printf("  - JOG,speed,direction>");
    printf("  - TRACK>");
}

// Process incoming serial data
void SerialHandler::process() {
    // Check for data from serial
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) {
        // Echo character back
        putchar(c);
        
        // Store in buffer if there's space and it's not a newline
        if (idx < MAX_CMD_SIZE - 1 && c != '>') {
            cmd_buffer[idx++] = static_cast<char>(c);
        } 
        // Process command when newline is received
        else if (c == '>') {
            cmd_buffer[idx] = '\0';  // Null terminate
            
            // Process the command
            if (idx > 0) {
                CommandProcessor::parseCommand(cmd_buffer);
            }
            
            // Reset buffer index
            idx = 0;
        }
    }
}