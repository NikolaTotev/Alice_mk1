#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp/board.h"
#include "tusb.h"
#include "device/usbd.h"
#include "class/hid/hid_device.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hid_interface.h"

// HID Report IDs
#define REPORT_ID_CHANGE_MODE  0x01
#define REPORT_ID_SEND_DATA    0x02
#define REPORT_ID_GET_DATA     0x03

// HID Report sizes
#define REPORT_SIZE_CHANGE_MODE 2   // 1 byte report ID + 1 byte mode
#define REPORT_SIZE_SEND_DATA   65  // 1 byte report ID + 64 bytes data
#define REPORT_SIZE_GET_DATA    65  // 1 byte report ID + 64 bytes data

// Device state
static uint8_t current_mode = 0;
static uint8_t data_buffer[64];
static bool data_ready = false;

// Remove this entire section - it's already in usb_descriptors.c
// //--------------------------------------------------------------------+
// // HID Report Descriptor  
// //--------------------------------------------------------------------+
// uint8_t const desc_hid_report[] = {
//     ... (REMOVE THIS ENTIRE ARRAY)
// };

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                               hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    (void) instance;
    (void) report_type;
    
    switch (report_id) {
        case REPORT_ID_GET_DATA:
            if (reqlen >= REPORT_SIZE_GET_DATA) {
                buffer[0] = REPORT_ID_GET_DATA;
                memcpy(&buffer[1], data_buffer, 64);
                return REPORT_SIZE_GET_DATA;
            }
            break;
            
        case REPORT_ID_CHANGE_MODE:
            if (reqlen >= REPORT_SIZE_CHANGE_MODE) {
                buffer[0] = REPORT_ID_CHANGE_MODE;
                buffer[1] = current_mode;
                return REPORT_SIZE_CHANGE_MODE;
            }
            break;
    }
    
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, 
                           hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;
    (void) report_type;
    
    switch (report_id) {
        case REPORT_ID_CHANGE_MODE:
            if (bufsize >= 2) {
                current_mode = buffer[1];
                printf("Mode changed to: %d\n", current_mode);
            }
            break;
            
        case REPORT_ID_SEND_DATA:
            if (bufsize >= 2) {
                memcpy(data_buffer, &buffer[1], (bufsize - 1 > 64) ? 64 : bufsize - 1);
                data_ready = true;
                printf("Data received: %d bytes\n", bufsize - 1);
            }
            break;
    }
}

//--------------------------------------------------------------------+
// User API Functions
//--------------------------------------------------------------------+

/**
 * Change the device operating mode
 * @param mode New mode value (0-255)
 * @return true if successful, false otherwise
 */
bool change_mode(uint8_t mode) {
    if (!tud_mounted()) {
        printf("USB not mounted\n");
        return false;
    }
    
    current_mode = mode;
    
    // Send mode change report to host
    uint8_t report[REPORT_SIZE_CHANGE_MODE];
    report[0] = REPORT_ID_CHANGE_MODE;
    report[1] = mode;
    
    if (tud_hid_ready()) {
        bool result = tud_hid_report(REPORT_ID_CHANGE_MODE, report, REPORT_SIZE_CHANGE_MODE);
        if (result) {
            printf("Mode changed to: %d\n", mode);
        }
        return result;
    }
    
    return false;
}

/**
 * Send data to the host
 * @param data Pointer to data buffer
 * @param length Length of data (max 64 bytes)
 * @return true if successful, false otherwise
 */
bool send_data(const uint8_t* data, uint16_t length) {
    if (!tud_mounted()) {
        printf("USB not mounted\n");
        return false;
    }
    
    if (length > 64) {
        printf("Data too large (max 64 bytes)\n");
        return false;
    }
    
    if (!data) {
        printf("Invalid data pointer\n");
        return false;
    }
    
    uint8_t report[REPORT_SIZE_SEND_DATA];
    report[0] = REPORT_ID_SEND_DATA;
    memcpy(&report[1], data, length);
    
    // Pad with zeros if needed
    if (length < 64) {
        memset(&report[1 + length], 0, 64 - length);
    }
    
    if (tud_hid_ready()) {
        bool result = tud_hid_report(REPORT_ID_SEND_DATA, report, REPORT_SIZE_SEND_DATA);
        if (result) {
            printf("Data sent: %d bytes\n", length);
        }
        return result;
    }
    
    return false;
}

/**
 * Get data from internal buffer (received from host)
 * @param buffer Pointer to destination buffer
 * @param max_length Maximum bytes to copy
 * @return Number of bytes copied, 0 if no data available
 */
uint16_t get_data(uint8_t* buffer, uint16_t max_length) {
    if (!buffer || max_length == 0) {
        return 0;
    }
    
    if (!data_ready) {
        return 0;
    }
    
    uint16_t copy_length = (max_length > 64) ? 64 : max_length;
    memcpy(buffer, data_buffer, copy_length);
    
    // Clear the data ready flag
    data_ready = false;
    
    printf("Data retrieved: %d bytes\n", copy_length);
    return copy_length;
}

//--------------------------------------------------------------------+
// Device Callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
    printf("USB HID Device mounted\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    printf("USB HID Device unmounted\n");
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    printf("USB suspended\n");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    printf("USB resumed\n");
}

//--------------------------------------------------------------------+
// Main Application
//--------------------------------------------------------------------+

int main(void) {
    // Initialize board
    board_init();
    
    // Initialize USB
    tusb_init();
    
    printf("RP2040 HID Interface Started\n");
    printf("Available functions:\n");
    printf("- change_mode(mode)\n");
    printf("- send_data(data, length)\n");
    printf("- get_data(buffer, max_length)\n");
    
    // Example usage
    uint8_t test_data[] = "Hello from RP2040!";
    uint8_t receive_buffer[64];
    
    while (1) {
        // Process USB tasks
        tud_task();
        
        // Example: Change mode every 5 seconds
        static uint32_t mode_timer = 0;
        static uint8_t test_mode = 0;
        
        if (board_millis() - mode_timer > 5000) {
            mode_timer = board_millis();
            change_mode(test_mode++);
        }
        
        // Example: Send test data every 3 seconds
        static uint32_t send_timer = 0;
        
        if (board_millis() - send_timer > 3000) {
            send_timer = board_millis();
            send_data(test_data, strlen((char*)test_data));
        }
        
        // Example: Check for received data
        uint16_t received = get_data(receive_buffer, sizeof(receive_buffer));
        if (received > 0) {
            printf("Received data: ");
            for (int i = 0; i < received; i++) {
                printf("%02X ", receive_buffer[i]);
            }
            printf("\n");
        }
        
        // Small delay to prevent overwhelming the system
        sleep_ms(10);
    }
    
    return 0;
}