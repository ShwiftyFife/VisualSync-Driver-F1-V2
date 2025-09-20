#ifndef INPUT_READER_KNOB_H
#define INPUT_READER_KNOB_H

#include <cstdint>                // For uint8_t, uint16_t types
#include <hidapi/hidapi.h>

// =============================================================================
// CONSTANTS - Knob input configuration
// =============================================================================

const int KNOB_BYTE_START = 6;              // Knobs start at byte 6
const int KNOB_COUNT = 4;                   // 4 knobs total
const int KNOB_BYTES_PER_KNOB = 2;          // 2 bytes per knob (LSB first)

const uint16_t KNOB_RAW_MIN = 0x000;        // Minimum raw value (12-bit)
const uint16_t KNOB_RAW_MAX = 0xFFF;        // Maximum raw value (12-bit)
const uint16_t KNOB_12BIT_MASK = 0x0FFF;    // Mask for 12-bit values

// =============================================================================
// KNOB INPUT READER CLASS
// =============================================================================

class KnobInputReader {
private:
    float previous_values[KNOB_COUNT];       // Previous knob values for change detection  // ==== UNUSED ====
    bool initialized;                        // Track if there is a baseline value  // ==== UNUSED ====

    // Helper function to extract raw 12-bit value from buffer
    uint16_t extractRawKnobValue(const unsigned char* buffer, int knob_number) const;

    // Helper function to convert raw value to normalized float
    float rawToNormalized(uint16_t raw_value) const;

    // Helper function to clamp raw values to valid 12-bit range
    uint16_t clampRawValue(uint16_t raw_value) const;

public:
    // Initialization
    bool initialize();  // ==== UNUSED ====
    
    // Main functions
    float getKnobValue(const unsigned char* buffer, int knob_number);
    bool hasKnobChanged(const unsigned char* buffer, int knob_number, float threshold = 0.01f); // ==== UNUSED ====
    
    // Update function - call this once per frame after reading all knob values
    void updateKnobStates(const unsigned char* buffer); // ==== UNUSED ====

    // Debug/utility functions
    uint16_t getRawKnobValue(const unsigned char* buffer, int knob_number); // ==== UNUSED ====
    void printKnobValues(const unsigned char* buffer);

};

#endif // INPUT_READER_KNOB_H