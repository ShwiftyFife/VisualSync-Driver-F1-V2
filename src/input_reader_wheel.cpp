#include "include/input_reader_wheel.h"

#include <iostream>             // For std::cout and std::cerr
#include <iomanip>              // For std::hex (hexadecimal printing)
#include <cmath>                // For std::abs
// #include <hidapi/hidapi.h>   // included already in header

// =============================================================================
// WHEEL INPUT READER CLASS IMPLEMENTATION
// =============================================================================

/*
* Constructor/Initialization
*
* Initializes the wheel input reader and tracking variables.
*/
bool WheelInputReader::initialize() {
    previous_wheel_value = 0;
    initialized = false;
    return true;
}

// =============================================================================
// MAIN WheelInputReader CLASS FUNCTIONS
// =============================================================================

/*
* Main function to check wheel rotation
* Call this once per frame with the current input buffer
* 
* @param buffer: The 22-byte input report from readInputReport()
* @return: WheelDirection indicating if and how the wheel rotated
*/
WheelDirection WheelInputReader::checkWheelRotation(const unsigned char* buffer) {
    // Step 1: Check if device is valid
    // Checks if pointers are valid before using them! This prevents crashes.
    if (buffer == nullptr) {
        std::cerr << "WheelInputReader Error: Buffer is null" << std::endl;
        return WheelDirection::NONE;
    }

    // Step 2: Read the current wheel value from its position in the buffer
    unsigned char current_value = buffer[WHEEL_BYTE_POSITION];

    // Step 3: Check if this is the first reading - just store the value, no rotation detected
    if (!initialized) {
        previous_wheel_value = current_value;
        initialized = true;
        return WheelDirection::NONE;
    }
    
    // Step 4: Calculate direction based on current vs previous
    WheelDirection direction = calculateDirection(previous_wheel_value, current_value);

    // Step 5: Update previous value for next call
    previous_wheel_value = current_value;
    
    return direction;
}

/*
* Helper function to determine rotation direction between two wheel values
* Handles wraparound cases properly
* 
* @param old_value: Previous wheel value
* @param new_value: Current wheel value
* @return: Direction of rotation
*/
WheelDirection WheelInputReader::calculateDirection(unsigned char old_value, unsigned char new_value) const {
    // If the values are equal, no rotation is detected
    if (old_value == new_value) {
        return WheelDirection::NONE;
    }
    
    // Calculate raw difference
    int raw_diff = new_value - old_value;
    
    // Handle wraparound cases
    // If the difference is very large, it's likely a wraparound
    if (raw_diff > 127) {
        // Large positive diff suggests counter-clockwise wraparound (255->0)
        return WheelDirection::COUNTER_CLOCKWISE;
    } else if (raw_diff < -127) {
        // Large negative diff suggests clockwise wraparound (0->255)
        return WheelDirection::CLOCKWISE;
    } else if (raw_diff > 0) {
        // Normal clockwise rotation
        return WheelDirection::CLOCKWISE;
    } else {
        // Normal counter-clockwise rotation
        return WheelDirection::COUNTER_CLOCKWISE;
    }
}
