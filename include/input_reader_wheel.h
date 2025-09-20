#ifndef INPUT_READER_WHEEL_H
#define INPUT_READER_WHEEL_H

#include <hidapi/hidapi.h>

// =============================================================================
// CONSTANTS - Wheel input configuration
// =============================================================================

const int WHEEL_BYTE_POSITION = 5;          // Byte 5 contains wheel value (0-255)

// =============================================================================
// ENUMS - Wheel rotation direction
// =============================================================================

enum class WheelDirection {
    NONE,              // No rotation detected
    CLOCKWISE,         // Rotating clockwise (incrementing)
    COUNTER_CLOCKWISE  // Rotating counter-clockwise (decrementing)
};


// =============================================================================
// WHEEL INPUT READER CLASS
// =============================================================================

class WheelInputReader {
private:
    unsigned char previous_wheel_value;   // Previous wheel value for change detection
    bool initialized;                     // Track if there is a baseline value

    // Helper function to calculate direction between two values
    WheelDirection calculateDirection(unsigned char old_value, unsigned char new_value) const;

public:
    // Initialization
    bool initialize();
    
    // Main function - call this every frame
    WheelDirection checkWheelRotation(const unsigned char* buffer);
};

#endif // INPUT_READER_WHEEL_H