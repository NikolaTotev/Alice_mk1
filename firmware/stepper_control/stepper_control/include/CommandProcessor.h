#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <cstdint>
#include <cstring>

// Command buffer size
#define MAX_CMD_SIZE 32

#define CMD_JOG "JOG"
#define CMD_JOG_STOP "JS"
#define CMD_TRACK "TRACK"
#define CMD_TRACK_STOP "TRACK_STOP"

enum class CommandType : uint8_t
{
    JOG = 1,
    JOG_STOP = 2,
    TRACK = 3,
    TACK_STOP = 4
    // Add more commands here as needed
};

enum class Direction : uint8_t {
    CW = 0,
    CCW = 1,
    // Add more commands here as needed
};


// Command structure to pass between cores
struct Command
{
    CommandType cmd_type; // 1 for JOG, 2 for TRACK
    int speed;            // Used for JOG command
    Direction direction;        // Used for JOG command (1 or -1)
};

class CommandProcessor
{
public:
    // Parse a command string and send to Core 1
    static void parseCommand(const char *cmd_str);

private:
    // Private constructor - this is a static utility class
    CommandProcessor() {}
};

#endif // COMMAND_PROCESSOR_H