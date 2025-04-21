#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

enum class Direction : uint8_t {
    CW = 0,
    CCW = 1,
    // Add more commands here as needed
};

class MotionControl {
public:
    // Initialize the motion control system
    static void init();
    
    // Jog with the specified speed and direction
    static void start_jog(int speed, Direction direction);

    static void stop_jog(bool immediate);
    
    // Track (follow a target)
    static void track();
        
private:
    // Private constructor - this is a static utility class
    MotionControl() {}
    
    // Internal motor control methods could go here
    static void setMotorSpeed(int motor_id, int speed);
};

#endif // MOTION_CONTROL_H