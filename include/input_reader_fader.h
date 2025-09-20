#ifndef INPUT_READER_FADER_H
#define INPUT_READER_FADER_H

#include <cstdint>                // For uint8_t, uint16_t types
#include <hidapi/hidapi.h>

// =============================================================================
// CONSTANTS - Fader input configuration
// =============================================================================

const int FADER_BYTE_START = 14;            // Faders start at byte 14
const int FADER_COUNT = 4;                  // 4 faders total
const int FADER_BYTES_PER_FADER = 2;        // 2 bytes per fader (LSB first)

const uint16_t FADER_RAW_MIN = 0x000;       // Minimum raw value (12-bit)
const uint16_t FADER_RAW_MAX = 0xFFF;       // Maximum raw value (12-bit)
const uint16_t FADER_12BIT_MASK = 0x0FFF;   // Mask for 12-bit values

// =============================================================================
// FADER INPUT READER CLASS
// =============================================================================

class FaderInputReader {
private:
    float previous_values[FADER_COUNT];      // Previous fader values for change detection  // ==== UNUSED ====
    bool initialized;                        // Track if there is a baseline value  // ==== UNUSED ====

    // Helper function to extract raw 12-bit value from buffer
    uint16_t extractRawFaderValue(const unsigned char* buffer, int fader_number) const;

    // Helper function to convert raw value to normalized float
    float rawToNormalized(uint16_t raw_value) const;

    // Helper function to clamp raw values to valid 12-bit range
    uint16_t clampRawValue(uint16_t raw_value) const;

public:
    // Initialization
    bool initialize();
    
    // Main functions
    float getFaderValue(const unsigned char* buffer, int fader_number);
    bool hasFaderChanged(const unsigned char* buffer, int fader_number, float threshold = 0.01f);  // ==== UNUSED ====
    
    // Update function - call this once per frame after reading all fader values
    void updateFaderStates(const unsigned char* buffer);  // ==== UNUSED ====
    
    // Utility functions
    uint16_t getRawFaderValue(const unsigned char* buffer, int fader_number);  // ==== UNUSED ====
    void printFaderValues(const unsigned char* buffer);
};

#endif // INPUT_READER_FADER_H