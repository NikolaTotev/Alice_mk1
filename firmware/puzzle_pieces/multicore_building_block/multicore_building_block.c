#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

// Define the operational modes as an enum
// This makes the code more readable and maintainable
typedef enum {
    MODE_IDLE = 0,
    MODE_RUNNING,
    MODE_CALIBRATING,
    MODE_ERROR,
    MODE_SHUTDOWN
} OperationalMode;

// Define the operational state structure
// This struct will be passed between cores via the queue
typedef struct {
    OperationalMode mode;
    uint32_t timestamp;      // Adding timestamp to track when state changed
    uint16_t error_code;     // Optional: error code if in ERROR mode
} OperationalState;

// Define execution status that Core 1 reports back to Core 0
typedef enum {
    EXEC_SUCCESS = 0,
    EXEC_IN_PROGRESS,
    EXEC_FAILED,
    EXEC_TIMEOUT,
    EXEC_INVALID_STATE
} ExecutionStatus;

// Structure for Core 1 to report execution results back to Core 0
typedef struct {
    ExecutionStatus status;
    OperationalMode mode_executed;  // Which mode was being executed
    uint32_t execution_time_ms;     // How long the execution took
    uint16_t details;               // Additional details/error codes
} ExecutionReport;

// Global queues for bidirectional inter-core communication
// Core 0 -> Core 1: State changes
queue_t state_queue;
// Core 1 -> Core 0: Execution status reports
queue_t report_queue;

// Current operational state on Core 0
// This is the "master" copy that Core 0 maintains
OperationalState current_state;

// Function to convert enum to string for printing
// This makes the serial output human-readable
const char* modeToString(OperationalMode mode) {
    switch(mode) {
        case MODE_IDLE:        return "IDLE";
        case MODE_RUNNING:     return "RUNNING";
        case MODE_CALIBRATING: return "CALIBRATING";
        case MODE_ERROR:       return "ERROR";
        case MODE_SHUTDOWN:    return "SHUTDOWN";
        default:               return "UNKNOWN";
    }
}

// Function to convert execution status to string
const char* statusToString(ExecutionStatus status) {
    switch(status) {
        case EXEC_SUCCESS:       return "SUCCESS";
        case EXEC_IN_PROGRESS:   return "IN_PROGRESS";
        case EXEC_FAILED:        return "FAILED";
        case EXEC_TIMEOUT:       return "TIMEOUT";
        case EXEC_INVALID_STATE: return "INVALID_STATE";
        default:                 return "UNKNOWN";
    }
}

// Function to check for execution reports from Core 1 (non-blocking)
// Returns true if a report was received, false otherwise
bool checkExecutionReport(void) {
    ExecutionReport report;
    
    // Non-blocking check for reports from Core 1
    if (queue_try_remove(&report_queue, &report)) {
        // We received a report - process it
        printf("Core 0: Received execution report:\n");
        printf("  Mode executed: %s\n", modeToString(report.mode_executed));
        printf("  Status: %s\n", statusToString(report.status));
        printf("  Execution time: %lu ms\n", report.execution_time_ms);
        
        if (report.status == EXEC_FAILED || report.status == EXEC_TIMEOUT) {
            printf("  Error details: 0x%04X\n", report.details);
        }
        printf("\n");
        
        return true;
    }
    
    return false;
}

// Function to update the operational state on Core 0
// This is called whenever the state needs to change
void setOperationalState(OperationalMode new_mode, uint16_t error_code) {
    // First, check if there are any pending execution reports
    // This ensures we don't miss status updates from Core 1
    while (checkExecutionReport()) {
        // Process all pending reports before sending new state
    }
    
    // Update the current state structure
    current_state.mode = new_mode;
    current_state.timestamp = to_ms_since_boot(get_absolute_time());
    current_state.error_code = error_code;
    
    // Try to add the new state to the queue
    // Using try_add (non-blocking) to avoid Core 0 getting stuck
    if (!queue_try_add(&state_queue, &current_state)) {
        // Queue is full - in production code, you might want to handle this
        // For now, we'll just note it (Core 0 shouldn't block on queue operations)
        printf("Core 0: Warning - Queue full, state change not sent\n");
    } else {
        // Successfully queued the state change
        printf("Core 0: State changed to %s at %lu ms\n", 
               modeToString(new_mode), current_state.timestamp);
    }
}

// Function running on Core 1 to receive and print operational states
// This function will block waiting for new states to arrive
void sendOperationalState(void) {
    OperationalState received_state;
    uint32_t start_time, execution_time;
    ExecutionReport report;
    
    // Blocking remove - Core 1 will wait here until data is available
    // This is appropriate since Core 1's main job is to handle state updates
    queue_remove_blocking(&state_queue, &received_state);
    
    // Record when we started processing
    start_time = to_ms_since_boot(get_absolute_time());
    
    // Print the received state information
    printf("Core 1: Received state update:\n");
    printf("  Mode: %s\n", modeToString(received_state.mode));
    printf("  Timestamp: %lu ms\n", received_state.timestamp);
    
    // If in error mode, also print the error code
    if (received_state.mode == MODE_ERROR) {
        printf("  Error Code: 0x%04X\n", received_state.error_code);
    }
    
    printf("  Queue remaining capacity: %d\n\n", 
           queue_get_level(&state_queue));
    
    // Simulate executing the state change
    // In a real application, this is where you would perform the actual work
    ExecutionStatus exec_status = EXEC_SUCCESS;
    uint16_t exec_details = 0;
    
    // Simulate different execution scenarios based on mode
    switch (received_state.mode) {
        case MODE_RUNNING:
            // Simulate some work
            sleep_ms(100);
            exec_status = EXEC_SUCCESS;
            break;
            
        case MODE_CALIBRATING:
            // Calibration takes longer
            sleep_ms(500);
            // Simulate occasional calibration failure
            if (to_ms_since_boot(get_absolute_time()) % 7 == 0) {
                exec_status = EXEC_FAILED;
                exec_details = 0x0001;  // Calibration failed
            } else {
                exec_status = EXEC_SUCCESS;
            }
            break;
            
        case MODE_ERROR:
            // Can't execute in error mode
            exec_status = EXEC_INVALID_STATE;
            exec_details = received_state.error_code;
            break;
            
        case MODE_SHUTDOWN:
            // Shutdown sequence
            sleep_ms(200);
            exec_status = EXEC_SUCCESS;
            break;
            
        default:
            // Default handling
            sleep_ms(50);
            exec_status = EXEC_SUCCESS;
            break;
    }
    
    // Calculate execution time
    execution_time = to_ms_since_boot(get_absolute_time()) - start_time;
    
    // Prepare execution report
    report.status = exec_status;
    report.mode_executed = received_state.mode;
    report.execution_time_ms = execution_time;
    report.details = exec_details;
    
    //TODO Maybe add IDs to the commands and the reports so we can track which report corresponds to which ID
    // Send report back to Core 0 (non-blocking to avoid Core 1 getting stuck)
    if (!queue_try_add(&report_queue, &report)) {
        printf("Core 1: Warning - Report queue full, status not sent\n");
    } else {
        printf("Core 1: Execution complete, report sent\n\n");
    }
}

// Core 1 entry point - this runs the entire time on Core 1
void core1_main(void) {
    printf("Core 1: Started and waiting for state updates...\n\n");
    
    // Infinite loop on Core 1
    // Continuously wait for and process state updates
    while (true) {
        sendOperationalState();
    }
}

// Example function to simulate state changes
// In a real application, these would be triggered by actual events
void simulateStateChanges(void) {
    // Wait a bit before starting
    sleep_ms(2000);
    
    // Simulate various state transitions
    setOperationalState(MODE_RUNNING, 0);
    sleep_ms(1500);
    
    setOperationalState(MODE_CALIBRATING, 0);
    sleep_ms(2000);
    
    setOperationalState(MODE_RUNNING, 0);
    sleep_ms(1000);
    
    // Simulate an error
    setOperationalState(MODE_ERROR, 0x1234);
    sleep_ms(3000);
    
    // Recover from error
    setOperationalState(MODE_IDLE, 0);
    sleep_ms(1000);
    
    // Shutdown
    setOperationalState(MODE_SHUTDOWN, 0);
}

int main() {
    // Initialize stdio for USB/UART output
    stdio_init_all();
    
    // Wait for USB serial connection (optional, remove for standalone operation)
    sleep_ms(3000);
    
    printf("RP2040 Dual Core Queue Example Starting...\n");
    printf("Core 0: Initializing...\n");
    
    // Initialize the queues
    // Parameters: queue pointer, element size, max elements
    // State queue: Core 0 -> Core 1 (8 state updates)
    queue_init(&state_queue, sizeof(OperationalState), 8);
    // Report queue: Core 1 -> Core 0 (8 execution reports)
    queue_init(&report_queue, sizeof(ExecutionReport), 8);
    
    // Initialize the current state to IDLE
    current_state.mode = MODE_IDLE;
    current_state.timestamp = to_ms_since_boot(get_absolute_time());
    current_state.error_code = 0;
    
    // Launch Core 1 with its main function
    // Core 1 will start executing core1_main() independently
    multicore_launch_core1(core1_main);
    
    // Give Core 1 time to start up
    sleep_ms(100);
    
    printf("Core 0: Running main loop...\n\n");
    
    // Main loop on Core 0
    while (true) {
        // Check for any execution reports from Core 1
        // This is non-blocking, so Core 0 continues if no reports
        checkExecutionReport();
        
        // In a real application, this is where you would:
        // - Read sensors
        // - Check for user input
        // - Monitor system health
        // - Make decisions about state changes
        
        // For this demo, we'll simulate state changes
        simulateStateChanges();
        
        // Check for reports one more time before idling
        checkExecutionReport();
        
        // After simulation, just idle
        sleep_ms(1000);
    }
    
    return 0;
}