/**
 * @file AS5048A.cpp
 * @brief Implementation for the AS5048A magnetic rotary encoder library
 * @date April 2025
 */

#include "AS5048.h"

// Constructor
AS5048A::AS5048A(uint8_t csPin, SPIClass* spi) {
    _csPin = csPin;
    _spi = spi;
    _lastError = 0;
    _zeroPosition = 0;
}

// Initialize the sensor
bool AS5048A::begin(float speedMHz) {
    // Set up CS pin as output and set HIGH
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    
    // Initialize SPI
    _spi->begin();
    
    // Perform a test read to see if the sensor is responding
    uint16_t diagValue = readDiagnostics();
    
    // Check if the OCF (Offset Compensation Finished) bit is set
    // This indicates the sensor has completed its startup procedure
    return (diagValue & DIAG_OCF) != 0;
}

// Calculate even parity bit
uint16_t AS5048A::calculateParity(uint16_t value) {
    uint16_t parity = 0;
    
    // Calculate even parity for the 15 bits
    for (int i = 0; i < 15; i++) {
        parity ^= (value >> i) & 0x01;
    }
    
    return parity;
}

// Send SPI command and get response
uint16_t AS5048A::spiTransfer(uint16_t command) {
    // Add parity bit (MSB)
    uint16_t parityBit = calculateParity(command);
    command |= (parityBit << 15);
    
    // Begin SPI transaction
    _spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    
    // Select the chip
    digitalWrite(_csPin, LOW);
    
    // Send the command (16 bits)
    uint16_t high = _spi->transfer((command >> 8) & 0xFF);
    uint16_t low = _spi->transfer(command & 0xFF);
    
    // Combine high and low bytes into 16-bit response
    uint16_t response = (high << 8) | low;
    
    // Deselect the chip
    digitalWrite(_csPin, HIGH);
    
    // End SPI transaction
    _spi->endTransaction();
    
    // Check error flag (bit 14)
    _lastError = (response >> 14) & 0x01;
    
    // Return the 14-bit data part of the response
    return response & 0x3FFF;
}

// Read a register
uint16_t AS5048A::readRegister(uint16_t registerAddress) {
    // First command: Request read from register
    spiTransfer(CMD_READ | registerAddress);
    
    // Second command: NOP to get the response
    return spiTransfer(REG_NOP);
}

// Write a value to a register
uint16_t AS5048A::writeRegister(uint16_t registerAddress, uint16_t value) {
    // First command: Request write to register
    spiTransfer(CMD_WRITE | registerAddress);
    
    // Second command: Send the data
    return spiTransfer(value & 0x3FFF);  // Ensure only 14 bits are sent
}

// Read the raw angle value
uint16_t AS5048A::readRawAngle() {
    uint16_t rawAngle = readRegister(REG_ANGLE);
    return rawAngle;
}

// Read the angle in degrees
float AS5048A::readAngleDegrees() {
    uint16_t rawAngle = readRawAngle();
    // Convert 14-bit value (0-16383) to degrees (0-359.99)
    return (float)rawAngle * 360.0 / 16384.0;
}

// Read the angle in radians
float AS5048A::readAngleRadians() {
    uint16_t rawAngle = readRawAngle();
    // Convert 14-bit value (0-16383) to radians (0-2π)
    return (float)rawAngle * 2.0 * PI / 16384.0;
}

// Read the magnitude value
uint16_t AS5048A::readMagnitude() {
    return readRegister(REG_MAGNITUDE);
}

// Read AGC value
uint8_t AS5048A::readAGC() {
    uint16_t diagAgc = readRegister(REG_DIAG_AGC);
    return diagAgc & 0xFF;  // AGC value is in the lower 8 bits
}

// Read diagnostic flags
uint8_t AS5048A::readDiagnostics() {
    uint16_t diagAgc = readRegister(REG_DIAG_AGC);
    // Extract the diagnostic flags from bits 8-11
    return (diagAgc >> 8) & 0x0F;
}

// Check magnetic field strength
int AS5048A::checkMagneticField() {
    uint8_t diag = readDiagnostics();
    
    if (diag & DIAG_COMP_HIGH) {
        return 1;   // Field too strong
    } else if (diag & DIAG_COMP_LOW) {
        return -1;  // Field too weak
    } else {
        return 0;   // Field strength OK
    }
}

// Clear error flags
void AS5048A::clearErrors() {
    readRegister(REG_CLEAR_ERROR_FLAG);
}

// Set the current position as the zero position (temporary)
void AS5048A::setZeroPosition() {
    _zeroPosition = readRawAngle();
}

// Program the current position as the zero position (permanent)
bool AS5048A::programZeroPosition() {
    uint16_t rawAngle = readRawAngle();
    
    // Split the 14-bit angle value into two parts
    uint16_t highPart = (rawAngle >> 6) & 0x7F;  // Bits 6-12
    uint16_t lowPart = rawAngle & 0x3F;          // Bits 0-5
    
    // First clear any old zero position
    writeRegister(REG_OTP_ZERO_POS_HI, 0);
    writeRegister(REG_OTP_ZERO_POS_LOW, 0);
    
    // Enable programming
    writeRegister(REG_PROG_CTRL, PROG_ENABLE);
    
    // Write high part
    writeRegister(REG_OTP_ZERO_POS_HI, highPart);
    
    // Write low part
    writeRegister(REG_OTP_ZERO_POS_LOW, lowPart);
    
    // Burn the OTP
    writeRegister(REG_PROG_CTRL, PROG_ENABLE | PROG_BURN);
    
    // Verify the OTP
    writeRegister(REG_PROG_CTRL, PROG_VERIFY);
    
    // Read the angle to check if zero position was set correctly
    // After programming, the angle should be 0
    uint16_t newAngle = readRawAngle();
    
    return newAngle < 10;  // Allow for small deviation due to noise
}

// Check if the last operation resulted in an error
bool AS5048A::hasError() {
    return _lastError != 0;
}

// Get PWM period in microseconds
uint32_t AS5048A::getPWMPeriod() {
    // The AS5048A has a 1kHz PWM frequency (according to datasheet)
    return 1000;
}