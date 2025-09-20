#ifndef INPUT_READER_BASE_H
#define INPUT_READER_BASE_H

#include <hidapi/hidapi.h>

// =============================================================================
// CONSTANTS - These define the structure of the F1's input reports
// =============================================================================

// Input report structure
const int INPUT_REPORT_SIZE = 22;            // F1 always sends 22-byte reports
const unsigned char INPUT_REPORT_ID = 0x01;  // First byte is always 0x01

// Byte positions in the input report
const int BUTTON_BYTE_MATRIX_TOP = 1;       // Matrix buttons (1,1) to (4,2)
const int BUTTON_BYTE_MATRIX_BOTTOM = 2;    // Matrix buttons (1,3) to (4,4)
const int BUTTON_BYTE_SPECIAL = 3;          // Special buttons (shift, browse, etc.)
const int BUTTON_BYTE_STOP_AND_CONTROL = 4; // Stop buttons and control buttons

// Bit masks for special buttons (byte 3)
const unsigned char BIT_MASK_SHIFT = 0x80;    // Bit 7
const unsigned char BIT_MASK_REVERSE = 0x40;  // Bit 6
const unsigned char BIT_MASK_TYPE = 0x20;     // Bit 5
const unsigned char BIT_MASK_SIZE = 0x10;     // Bit 4
const unsigned char BIT_MASK_BROWSE = 0x08;   // Bit 3
const unsigned char BIT_MASK_SELECTOR_WHEEL = 0x04;  // Bit 2

// Bit masks for stop buttons (byte 4)
const unsigned char BIT_MASK_STOP1 = 0x80;    // Bit 7
const unsigned char BIT_MASK_STOP2 = 0x40;    // Bit 6
const unsigned char BIT_MASK_STOP3 = 0x20;    // Bit 5
const unsigned char BIT_MASK_STOP4 = 0x10;    // Bit 4

// Bit masks for control buttons (byte 4)
const unsigned char BIT_MASK_SYNC = 0x08;     // Bit 3
const unsigned char BIT_MASK_QUANT = 0x04;    // Bit 2
const unsigned char BIT_MASK_CAPTURE = 0x02;  // Bit 1

// Matrix button bit masks for byte 1 (top half - rows 1-2)
const unsigned char BIT_MASK_MATRIX_1_1 = 0x80;  // Bit 7
const unsigned char BIT_MASK_MATRIX_2_1 = 0x40;  // Bit 6
const unsigned char BIT_MASK_MATRIX_3_1 = 0x20;  // Bit 5
const unsigned char BIT_MASK_MATRIX_4_1 = 0x10;  // Bit 4
const unsigned char BIT_MASK_MATRIX_1_2 = 0x08;  // Bit 3
const unsigned char BIT_MASK_MATRIX_2_2 = 0x04;  // Bit 2
const unsigned char BIT_MASK_MATRIX_3_2 = 0x02;  // Bit 1
const unsigned char BIT_MASK_MATRIX_4_2 = 0x01;  // Bit 0

// Matrix button bit masks for byte 2 (bottom half - rows 3-4)
const unsigned char BIT_MASK_MATRIX_1_3 = 0x80;  // Bit 7
const unsigned char BIT_MASK_MATRIX_2_3 = 0x40;  // Bit 6
const unsigned char BIT_MASK_MATRIX_3_3 = 0x20;  // Bit 5
const unsigned char BIT_MASK_MATRIX_4_3 = 0x10;  // Bit 4
const unsigned char BIT_MASK_MATRIX_1_4 = 0x08;  // Bit 3
const unsigned char BIT_MASK_MATRIX_2_4 = 0x04;  // Bit 2
const unsigned char BIT_MASK_MATRIX_3_4 = 0x02;  // Bit 1
const unsigned char BIT_MASK_MATRIX_4_4 = 0x01;  // Bit 0

// =============================================================================
// ENUMS - These make the code more readable than using magic numbers
// =============================================================================

// Special buttons enum - makes code more readable
enum class SpecialButton {
    SHIFT,
    REVERSE,
    TYPE,
    SIZE,
    BROWSE,
    SELECTOR_WHEEL
};

// Stop buttons enum
enum class StopButton {
    STOP1 = 1,
    STOP2 = 2,
    STOP3 = 3,
    STOP4 = 4
};

// Control buttons enum
enum class ControlButton {
    SYNC,
    QUANT,
    CAPTURE
};

// =============================================================================
// FUNCTION DECLARATIONS - What functions are provided to other files
// =============================================================================

// Main input reading function
bool readInputReport(hid_device* device, unsigned char* buffer);

// Button checking functions
bool isSpecialButtonPressed(const unsigned char* buffer, SpecialButton button);
bool isStopButtonPressed(const unsigned char* buffer, StopButton button);
bool isControlButtonPressed(const unsigned char* buffer, ControlButton button);
bool isMatrixButtonPressed(const unsigned char* buffer, int row, int col);

// Utility functions for testing and debugging
void printRawInputReport(const unsigned char* buffer);

#endif // INPUT_READER_BASE_H