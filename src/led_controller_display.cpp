#include "include/led_controller_display.h"
#include "include/led_controller_base.h"

#include <iostream>
#include <hidapi/hidapi.h>

// =============================================================================
// CONSTANTS - 7-SEGMENT DIGIT PATTERNS
// =============================================================================

/*
* 7-segment display patterns for digits 0-9 and blank
* Each pattern is an array of 7 values corresponding to the segments:
* [middle, lower_right, upper_right, top, upper_left, lower_left, bottom]
* Value 127 = full brightness, 0 = off
*/
const uint8_t DIGIT_PATTERNS[11][7] = {
    {0, 127, 127, 127, 127, 127, 127},    // 0
    {0, 127, 127, 0, 0, 0, 0},            // 1
    {127, 0, 127, 127, 0, 127, 127},      // 2
    {127, 127, 127, 127, 0, 0, 127},      // 3
    {127, 127, 127, 0, 127, 0, 0},        // 4
    {127, 127, 0, 127, 127, 0, 127},      // 5
    {127, 127, 0, 127, 127, 127, 127},    // 6
    {0, 127, 127, 127, 0, 0, 0},          // 7
    {127, 127, 127, 127, 127, 127, 127},  // 8
    {127, 127, 127, 127, 127, 0, 127},    // 9
    {0, 0, 0, 0, 0, 0, 0}                 // 10 = blank
};

// =============================================================================
// MAIN LED CONTROLLER DISPLAY CLASS IMPLEMENTATION
// =============================================================================

/*
* Set the display number (1-99)
*
* @param number: Number to display (1-99)
*/
void DisplayController::setDisplayNumber(int number) {
    // Step 1: Clamp to valid range
    if (number < 1) number = 1;
    if (number > 99) number = 99;

    // Step 2: Split number into digits
    int left_digit = (number >= 10) ? (number / 10) : 10; // 10 = blank index
    int right_digit = number % 10;
    
    // Step 3: Set each display
    setDisplaySegment(1, left_digit);  // Left display
    setDisplaySegment(2, right_digit); // Right display
}

/*
* Set or clear the decimal dot on a display
*
* @param display: 1 for left, 2 for right
* @param on: true to turn on, false to turn off
*/

void DisplayController::setDisplayDot(int display, bool on) {
    // Step 1: Access the external led_buffer and device from led_controller
    extern unsigned char led_buffer[81];
    extern hid_device* current_device;
    
    // Step 2: Set brightness based on on/off state
    uint8_t brightness = on ? 127 : 0;
    
    // Step 3: Set the appropriate dot byte
    if (display == 1) {
        // Left display dot (byte 9)
        led_buffer[9] = brightness;
    } else if (display == 2) {
        // Right display dot (byte 1) 
        led_buffer[1] = brightness;
    }

    // Step 4: Send updated buffer to device
    if (current_device != nullptr) {
        hid_write(current_device, led_buffer, 81);
    }
}


/*
* Private helper function to set display segments
*
* @param display: 1 for left, 2 for right  
* @param digit: Digit to display (0-9, or 10 for blank)
*/
void DisplayController::setDisplaySegment(int display, int digit) {
    // Step 1: Validate inputs
    if (digit < 0 || digit > 10) {
        std::cout << "Error: Invalid digit in setDisplaySegment()" << std::endl;
        return; // Invalid digit
    }

    // Step 2: Access the external led_buffer and device from led_controller
    extern unsigned char led_buffer[81];
    extern hid_device* current_device;

    // Step 3: Calculate byte positions
    int base_byte;
    if (display == 1) {
        // Left display: bytes 10-16 (skip dot at byte 9)
        base_byte = 10;
    } else if (display == 2) {
        // Right display: bytes 2-8 (skip dot at byte 1)
        base_byte = 2;
    } else {
         std::cout << "Error: Invalid display number in setDisplaySegment()" << std::endl;
        return; // Invalid display
    }
    
    // Step 4: Set 7 segments using the digit patterns
    // Pattern order: [middle, lower_right, upper_right, top, upper_left, lower_left, bottom]
    for (int i = 0; i < 7; i++) {
        led_buffer[base_byte + i] = DIGIT_PATTERNS[digit][i];
    }

    // Step 5: Send updated buffer to device
    if (current_device != nullptr) {
        hid_write(current_device, led_buffer, 81);
    }
}