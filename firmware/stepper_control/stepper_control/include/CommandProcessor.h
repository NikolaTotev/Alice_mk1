#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <cstdint>
#include <cstring>

// Command buffer size
#define MAX_CMD_SIZE 32

enum class CommandType : uint8_t {
    JOG = 1,
    TRACK = 2,
    // Add more commands here as needed
};

// Command structure to pass between cores
struct Command {
    CommandType cmd_type;    // 1 for JOG, 2 for TRACK
    int speed;         // Used for JOG command
    int direction;     // Used for JOG command (1 or -1)
};

class CommandProcessor {
public:
    // Parse a command string and send to Core 1
    static void parseCommand(const char* cmd_str);
    
private:
    // Private constructor - this is a static utility class
    CommandProcessor() {}
};

#endif // COMMAND_PROCESSOR_H