#ifndef HID_INTERFACE_H
#define HID_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// HID Interface API
//--------------------------------------------------------------------+

/**
 * Change the device operating mode
 * @param mode New mode value (0-255)
 * @return true if successful, false otherwise
 */
bool change_mode(uint8_t mode);

/**
 * Send data to the host
 * @param data Pointer to data buffer
 * @param length Length of data (max 64 bytes)
 * @return true if successful, false otherwise
 */
bool send_data(const uint8_t* data, uint16_t length);

/**
 * Get data from internal buffer (received from host)
 * @param buffer Pointer to destination buffer
 * @param max_length Maximum bytes to copy
 * @return Number of bytes copied, 0 if no data available
 */
uint16_t get_data(uint8_t* buffer, uint16_t max_length);

#ifdef __cplusplus
}
#endif

#endif /* HID_INTERFACE_H */