#include "include/input_reader_knob.h"

#include <iostream>             // For std::cout and std::cerr
#include <iomanip>              // For std::hex (hexadecimal printing)
#include <cmath>                // For std::abs
// #include <hidapi/hidapi.h>   // included already in header
// #include <cstdint>          // included already in header

// =============================================================================
// KNOB INPUT READER CLASS IMPLEMENTATION
// =============================================================================

/*
* Constructor/Initialization
*
* Initializes the knob input reader and tracking variables.
*/
bool KnobInputReader::initialize() {
    // Initialize all previous values to 0.0
    for (int i = 0; i < KNOB_COUNT; i++) {
        previous_values[i] = 0.0f;
    }
    initialized = false;
    return true;
}

// =============================================================================
// MAIN KnobInputReader CLASS FUNCTIONS
// =============================================================================

/*
* Get the normalized knob value (0.000 to 1.000)
* 
* @param buffer: The 22-byte input report from readInputReport()
* @param knob_number: Which knob to read (1-4)
* @return: Normalized float value from 0.000 to 1.000
*/
float KnobInputReader::getKnobValue(const unsigned char* buffer, int knob_number) {
    // Step 1: Check if device buffer is valid
    if (buffer == nullptr) {
        std::cerr << "KnobInputReader Error: Buffer is null" << std::endl;
        return 0.0f;
    }

    // Step 2: Validate knob number range
    if (knob_number < 1 || knob_number > KNOB_COUNT) {
        std::cerr << "KnobInputReader Error: Invalid knob number " << knob_number 
                  << ". Must be 1-" << KNOB_COUNT << std::endl;
        return 0.0f;
    }

    // Step 3: Extract raw 12-bit value from buffer
    uint16_t raw_value = extractRawKnobValue(buffer, knob_number);

    // Step 4: Convert raw value to normalized float
    float normalized_value = rawToNormalized(raw_value);

    return normalized_value;
}

/*
* Check if a knob value has changed since the last frame
* 
* @param buffer: The 22-byte input report from readInputReport()
* @param knob_number: Which knob to check (1-4)
* @param threshold: Minimum change to consider significant (default 0.01 = 1%)
* @return: true if knob has changed significantly, false otherwise
*/
bool KnobInputReader::hasKnobChanged(const unsigned char* buffer, int knob_number, float threshold) {
    // Step 1: Check if buffer is valid
    if (buffer == nullptr) {
        std::cerr << "KnobInputReader Error: Buffer is null in hasKnobChanged()" << std::endl;
        return false;
    }

    // Step 2: Validate knob number range
    if (knob_number < 1 || knob_number > KNOB_COUNT) {
        std::cerr << "KnobInputReader Error: Invalid knob number " << knob_number 
                  << " in hasKnobChanged()" << std::endl;
        return false;
    }

    // Step 3: Check if we have previous values to compare against
    if (!initialized) {
        return false;  // No previous state to compare
    }

    // Step 4: Get current and previous values
    float current_value = getKnobValue(buffer, knob_number);
    float previous_value = previous_values[knob_number - 1];  // Convert to 0-3 indexing

    // Step 5: Calculate absolute difference and compare to threshold
    float difference = std::abs(current_value - previous_value);
    
    return difference > threshold;
}

/*
* Update stored knob states for next frame comparison
* Call this once per frame after reading all knob values you need
* 
* @param buffer: The 22-byte input report from readInputReport()
*/
void KnobInputReader::updateKnobStates(const unsigned char* buffer) {
    // Step 1: Check if buffer is valid
    if (buffer == nullptr) {
        std::cerr << "KnobInputReader Error: Buffer is null in updateKnobStates()" << std::endl;
        return;
    }

    // Step 2: Update all knob values for next frame
    for (int knob = 1; knob <= KNOB_COUNT; knob++) {
        previous_values[knob - 1] = getKnobValue(buffer, knob);
    }

    // Step 3: Mark as initialized
    initialized = true;
}

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

/*
* Extract raw 12-bit knob value from input buffer
* Handles LSB-first byte ordering and 12-bit masking
* 
* @param buffer: The 22-byte input report
* @param knob_number: Which knob to extract (1-4)
* @return: Raw 12-bit value, clamped to valid range
*/
uint16_t KnobInputReader::extractRawKnobValue(const unsigned char* buffer, int knob_number) const {
    // Step 1: Calculate byte positions for this knob
    // Knob 1: bytes 6-7, Knob 2: bytes 8-9, Knob 3: bytes 10-11, Knob 4: bytes 12-13
    int byte_offset = (knob_number - 1) * KNOB_BYTES_PER_KNOB;
    int lsb_position = KNOB_BYTE_START + byte_offset;      // LSB position
    int msb_position = KNOB_BYTE_START + byte_offset + 1;  // MSB position

    // Step 2: Extract bytes and reconstruct 16-bit value (LSB first)
    uint16_t raw_value = buffer[lsb_position] | (buffer[msb_position] << 8);
    /*
    Here's how it works:
    buffer[lsb_position] retrieves the LSB.
    buffer[msb_position] << 8 shifts the MSB left by 8 bits, moving it to the upper half of the 16-bit value.
    The bitwise OR (|) combines these two bytes into one 16-bit value.
    */

    // Step 3: Apply 12-bit mask (upper 4 bits should be zero anyway)
    raw_value &= KNOB_12BIT_MASK;
    /*
    The bitwise AND assignment (&=) operation sets all bits outside the lowest 12 to zero, 
    guaranteeing that raw_value contains only valid 12-bit data.
    */

    // Step 4: Clamp to valid range and return
    return clampRawValue(raw_value);
}

/*
* Convert raw 12-bit value to normalized float (0.000 to 1.000)
* 
* @param raw_value: Raw 12-bit knob value (0-4095)
* @return: Normalized float value
*/
float KnobInputReader::rawToNormalized(uint16_t raw_value) const {
    // Simple division to get 0.000 to 1.000 range
    float normalized_value = (float)raw_value / (float)KNOB_RAW_MAX;
    
    // round to 3 decimal places for cleaner output
    normalized_value = std::round(normalized_value * 1000.0f) / 1000.0f;

    // Return the normalized value
    return normalized_value;
}

/*
* Clamp raw value to valid 12-bit range (simple calibration)
* 
* @param raw_value: Raw value to clamp
* @return: Clamped value within valid range
*/
uint16_t KnobInputReader::clampRawValue(uint16_t raw_value) const {
    if (raw_value < KNOB_RAW_MIN) {
        return KNOB_RAW_MIN;
    }
    if (raw_value > KNOB_RAW_MAX) {
        return KNOB_RAW_MAX;
    }
    return raw_value;
}

// =============================================================================
// DEBUG/UTILITY FUNCTIONS
// =============================================================================

/*
* Get the raw 12-bit knob value (for debugging or advanced use)
* 
* @param buffer: The 22-byte input report from readInputReport()
* @param knob_number: Which knob to read (1-4)
* @return: Raw 12-bit value (0-4095)
*/
uint16_t KnobInputReader::getRawKnobValue(const unsigned char* buffer, int knob_number) {
    // Step 1: Check if buffer is valid
    if (buffer == nullptr) {
        std::cerr << "KnobInputReader Error: Buffer is null in getRawKnobValue()" << std::endl;
        return 0;
    }

    // Step 2: Validate knob number range
    if (knob_number < 1 || knob_number > KNOB_COUNT) {
        std::cerr << "KnobInputReader Error: Invalid knob number " << knob_number 
                  << " in getRawKnobValue()" << std::endl;
        return 0;
    }

    // Step 3: Extract and return raw value
    return extractRawKnobValue(buffer, knob_number);
}

/*
* Print all knob values in a single line (utility for debugging/monitoring)
* 
* @param buffer: The 22-byte input report from readInputReport()
*/
void KnobInputReader::printKnobValues(const unsigned char* buffer) {
    // Step 1: Check if buffer is valid
    if (buffer == nullptr) {
        std::cerr << "KnobInputReader Error: Buffer is null in printKnobValues()" << std::endl;
        return;
    }

    // Step 2: Get all knob values
    float knob_value_1 = getKnobValue(buffer, 1);
    float knob_value_2 = getKnobValue(buffer, 2);
    float knob_value_3 = getKnobValue(buffer, 3);
    float knob_value_4 = getKnobValue(buffer, 4);

    // Step 3: Print values with consistent formatting
    std::cout << "Knob Values: "
              << "K1: " << std::fixed << std::setprecision(3) << knob_value_1 << " | "
              << "K2: " << std::fixed << std::setprecision(3) << knob_value_2 << " | "
              << "K3: " << std::fixed << std::setprecision(3) << knob_value_3 << " | "
              << "K4: " << std::fixed << std::setprecision(3) << knob_value_4
              << "        \r"; // Carriage return to overwrite the line
    std::cout.flush();
}

