/**
 * @file AS5048A.h
 * @brief Library for the AS5048A magnetic rotary encoder with SPI interface
 * @date April 2025
 *
 * This library provides an interface to the AS5048A 14-bit magnetic rotary encoder
 * for ESP32 C3 mini 1 module using SPI communication.
 */

#ifndef AS5048A_H
#define AS5048A_H

#include <Arduino.h>
#include <SPI.h>

class AS5048A {
public:
    // Register addresses
    static const uint16_t REG_NOP = 0x0000;
    static const uint16_t REG_CLEAR_ERROR_FLAG = 0x0001;
    static const uint16_t REG_PROG_CTRL = 0x0003;
    static const uint16_t REG_OTP_ZERO_POS_HI = 0x0016;
    static const uint16_t REG_OTP_ZERO_POS_LOW = 0x0017;
    static const uint16_t REG_DIAG_AGC = 0x3FFD;
    static const uint16_t REG_MAGNITUDE = 0x3FFE;
    static const uint16_t REG_ANGLE = 0x3FFF;

    // Command bits
    static const uint16_t CMD_READ = 0x4000;      // Read bit: 1 << 14
    static const uint16_t CMD_WRITE = 0x0000;     // Write bit: 0 << 14

    // Diagnostic flags (bits in REG_DIAG_AGC)
    static const uint8_t DIAG_COMP_HIGH = 0x08;   // Bit 11
    static const uint8_t DIAG_COMP_LOW = 0x04;    // Bit 10
    static const uint8_t DIAG_COF = 0x02;         // Bit 9
    static const uint8_t DIAG_OCF = 0x01;         // Bit 8

    // Programming control bits
    static const uint8_t PROG_VERIFY = 0x40;      // Bit 6
    static const uint8_t PROG_BURN = 0x08;        // Bit 3
    static const uint8_t PROG_ENABLE = 0x01;      // Bit 0

    /**
     * @brief Constructor for the AS5048A class
     * @param csPin The Chip Select pin to use for SPI communication
     * @param spi Pointer to the SPI object to use (optional)
     */
    AS5048A(uint8_t csPin, SPIClass* spi = &SPI);

    /**
     * @brief Initialize the sensor
     * @param speedMHz SPI clock speed in MHz (default: 1)
     * @return True if initialization succeeded, false otherwise
     */
    bool begin(float speedMHz = 1.0);

    /**
     * @brief Read the raw angle value (0-16383)
     * @return 14-bit raw angle value (0-16383)
     */
    uint16_t readRawAngle();

    /**
     * @brief Read the angle in degrees (0-359.99)
     * @return Angle in degrees
     */
    float readAngleDegrees();

    /**
     * @brief Read the angle in radians (0-2π)
     * @return Angle in radians
     */
    float readAngleRadians();

    /**
     * @brief Read the magnitude value
     * @return 14-bit magnitude value
     */
    uint16_t readMagnitude();

    /**
     * @brief Read the automatic gain control value
     * @return 8-bit AGC value (0-255)
     */
    uint8_t readAGC();

    /**
     * @brief Read the diagnostic flags
     * @return Byte containing diagnostic flags
     */
    uint8_t readDiagnostics();

    /**
     * @brief Check if there's a magnetic field strength issue
     * @return 0: No issue, 1: Field too strong, -1: Field too weak
     */
    int checkMagneticField();

    /**
     * @brief Clear error flags
     */
    void clearErrors();

    /**
     * @brief Set the current position as the zero position (temporary)
     */
    void setZeroPosition();

    /**
     * @brief Program the current position as the zero position (permanent in OTP)
     * @warning This operation can only be performed once!
     * @return True if programming succeeded, false otherwise
     */
    bool programZeroPosition();

    /**
     * @brief Check if the last SPI command resulted in an error
     * @return True if an error occurred, false otherwise
     */
    bool hasError();

    /**
     * @brief Get PWM period in microseconds
     * @return PWM period in microseconds
     */
    uint32_t getPWMPeriod();

private:
    uint8_t _csPin;           // Chip select pin
    SPIClass* _spi;           // SPI object
    uint16_t _lastError;      // Last error value
    uint16_t _zeroPosition;   // Current zero position offset

    /**
     * @brief Calculate even parity bit for a 15-bit value
     * @param value 15-bit value (excluding parity bit)
     * @return Parity bit (0 or 1)
     */
    uint16_t calculateParity(uint16_t value);

    /**
     * @brief Send a SPI command and read the response
     * @param command Command to send (address with read/write bit)
     * @return Response from the sensor
     */
    uint16_t spiTransfer(uint16_t command);

    /**
     * @brief Read a register value
     * @param registerAddress Register address to read
     * @return Register value
     */
    uint16_t readRegister(uint16_t registerAddress);

    /**
     * @brief Write a value to a register
     * @param registerAddress Register address to write to
     * @param value Value to write
     * @return Response from the write operation
     */
    uint16_t writeRegister(uint16_t registerAddress, uint16_t value);
};

#endif // AS5048A_H