#include "include/led_controller_base.h"      // Include header file

#include <iostream>             // For std::cout and std::cerr
#include <iomanip>              // For std::hex (hexadecimal printing)
#include <cstring>              // For memset (clearing memory)
#include <unistd.h>             // For usleep (sleep function)
// #include <hidapi/hidapi.h>   // included already in header


// =============================================================================
// GLOBAL LED STATE BYTE BUFFER - Persistent byte buffer for all LED states
// =============================================================================

/*
* This byte buffer holds the current state of all LEDs on the F1.
* It's persistent, so changing one LED does not affect the others.
* The byte buffer is always ready to send to the F1 device.
*/
unsigned char led_buffer[LED_REPORT_SIZE];
hid_device* current_device = nullptr;  // Store device for automatic sending

// =============================================================================
// PARALLEL STATE STORAGE - Preserve original color/brightness values
// =============================================================================

/*
* Parallel state storage arrays
* 
* These arrays store the original color and brightness values that were passed
* to the LED functions, BEFORE conversion to 7-bit hardware format. This allows
* other modules, such as the toggle system, to restore exact original values.
*/
static LEDStateMatrix matrix_states[5][5];         // Original states for 5x5 matrix
static LEDState special_states[5];                 // Original states for 5 special buttons
static LEDState control_states[3];                 // Original states for 3 control buttons
static LEDState stop_states[4];                    // Original states for 4 stop buttons

// =============================================================================
// HELPER FUNCTIONS - Internal functions for color conversion and validation
// =============================================================================

/*
* Converts 8-bit color value (0-255) to 7-bit value (0-127) with brightness
* The F1 hardware only accepts 7-bit color values, so we must convert.
* 
* @param value_8bit: Original color value (0-255)
* @param brightness: Brightness multiplier (0.0 = off, 1.0 = full brightness)
* @return: 7-bit color value (0-127) with brightness applied
*/
static unsigned char convertTo7Bit(unsigned char value_8bit, float brightness) {
    // Step 1: Clamp brightness to valid range (0.0 to 1.0)
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;
    
    // Step 2: Convert 8-bit to 7-bit and apply brightness
    float result = (value_8bit * 127.0f / 255.0f) * brightness;
    
    // Step 3: Clamp result to valid 7-bit range
    if (result < 0.0f) result = 0.0f;
    if (result > 127.0f) result = 127.0f;
    
    // Step 4: Round to nearest integer and return
    return (unsigned char)(result + 0.5f);
}

/*
* Validates matrix button position (row and column)
* Matrix buttons are arranged in a 4x4 grid, rows 1-4, columns 1-4
* 
* @param row: Matrix row (should be 1-4)
* @param col: Matrix column (should be 1-4)  
* @return: true if position is valid, false if invalid
*/
static bool isValidMatrixPosition(int row, int col) {
    return (row >= 1 && row <= MATRIX_ROWS && col >= 1 && col <= MATRIX_COLS);
}

// =============================================================================
// STATE STORAGE ACCESS FUNCTIONS - Map button enums to array indices
// =============================================================================

// =======================================
// Get button index functions
// =======================================

/*
* Maps SpecialLEDButton enum to array index
* 
* This function provides explicit mapping from enum values to array indices.
* This is safer than relying on implicit enum values and makes the mapping
* clear and maintainable.
* 
* @param button: The special button enum value
* @return: Array index (0-7), or -1 if invalid button
*/
int getSpecialButtonIndex(SpecialLEDButton button) {
    switch (button) {
        case SpecialLEDButton::BROWSE:  return 0;    // Array index 0
        case SpecialLEDButton::SIZE:    return 1;    // Array index 1
        case SpecialLEDButton::TYPE:    return 2;    // Array index 2
        case SpecialLEDButton::REVERSE: return 3;    // Array index 3
        case SpecialLEDButton::SHIFT:   return 4;    // Array index 4
    }
    return -1; // Error case - should never happen with valid enum
}

/*
* Maps ControlLEDButton enum to array index
*
* This function provides explicit mapping from enum values to array indices.
* This is safer than relying on implicit enum values and makes the mapping
* clear and maintainable.
*
* @param button: The control button enum value
* @return: Array index (0-2), or -1 if invalid button
*/
int getControlButtonIndex(ControlLEDButton button) {
    switch (button) {
        case ControlLEDButton::CAPTURE: return 0;    // Array index 0
        case ControlLEDButton::QUANT:   return 1;    // Array index 1
        case ControlLEDButton::SYNC:    return 2;    // Array index 2
    }
    return -1; // Error case - should never happen with valid enum
}

/*
 * Maps StopLEDButton enum to array index
 *
 * This function provides explicit mapping from enum values to array indices.
 * This is safer than relying on implicit enum values and makes the mapping
 * clear and maintainable.
 *
 * @param button: The stop button enum value
 * @return: Array index (0-3), or -1 if invalid button
 */
int getStopButtonIndex(StopLEDButton button) {
    switch (button) {
        case StopLEDButton::STOP1: return 0;    // Array index 0
        case StopLEDButton::STOP2: return 1;    // Array index 1
        case StopLEDButton::STOP3: return 2;    // Array index 2
        case StopLEDButton::STOP4: return 3;    // Array index 3
    }
    return -1; // Error case - should never happen with valid enum
}

// =======================================
// Get button state functions
// =======================================

/*
* Gets the original state for a matrix button
* 
* Returns the original color and brightness that were set for this matrix
* button position, before any 7-bit conversion or hardware formatting.
* 
* @param row: Matrix row (1-4)
* @param col: Matrix column (1-4)
* @return: LEDState with original color and brightness, or error state if invalid position
*/
LEDStateMatrix getMatrixButtonState(int row, int col) {
    // Validate position first
    if (!isValidMatrixPosition(row, col)) {
        std::cerr << "Error: Invalid matrix position (" << row << "," << col 
                  << ") in getMatrixButtonState()" << std::endl;
        return {LEDColor::black, 0.0f}; // Return error state
    }
    
    // Return the stored original state
    return matrix_states[row][col];
}

/*
* Gets the original state for a special button
* 
* Returns the original brightness that was set for this special button,
* before any 7-bit conversion. Color is always stored as white for special
* buttons since they only have single-color LEDs.
* 
* @param button: The special button to query
* @return: LEDState with original brightness, or error state if invalid button
*/
LEDState getSpecialButtonState(SpecialLEDButton button) {
    // Map button to array index
    int index = getSpecialButtonIndex(button);
    
    // Validate index
    if (index < 0 || index > 4) {
        std::cerr << "Error: Invalid special button in getSpecialButtonState()" << std::endl;
        return {0.0f}; // Return error state
    }
    
    // Return the stored original state
    return special_states[index];
}

/*
 * Gets the original state for a control button
 *
 * Returns the original brightness that was set for this control button,
 * before any 7-bit conversion. Color is always stored as white for control
 * buttons since they only have single-color LEDs.
 *
 * @param button: The control button to query
 * @return: LEDState with original brightness, or error state if invalid button
 */
LEDState getControlButtonState(ControlLEDButton button) {
    // Map button to array index
    int index = getControlButtonIndex(button);
    
    // Validate index
    if (index < 0 || index > 2) {
        std::cerr << "Error: Invalid control button in getControlButtonState()" << std::endl;
        return {0.0f}; // Return error state
    }
    
    // Return the stored original state
    return control_states[index];
}

/*
 * Gets the original state for a special button
 *
 * Returns the original brightness that was set for this special button,
 * before any 7-bit conversion. Color is always stored as white for special
 * buttons since they only have single-color LEDs.
 *
 * @param button: The special button to query
 * @return: LEDState with original brightness, or error state if invalid button
 */
LEDState getStopButtonState(StopLEDButton button) {
    // Map button to array index
    int index = getStopButtonIndex(button);
    
    // Validate index
    if (index < 0 || index > 3) {
        std::cerr << "Error: Invalid stop button in getStopButtonState()" << std::endl;
        return {0.0f}; // Return error state
    }
    
    // Return the stored original state
    return stop_states[index];
}

// =============================================================================
// COLOR SYSTEM FUNCTIONS - Convert colors to BRG format
// =============================================================================

/*
* Gets a color in BRG format with specified brightness
* All 18 colors are defined here with their RGB values, then converted to BRG
* 
* @param color: The color to get (using LEDColor enum)
* @param brightness: Brightness level (0.0 = off, 1.0 = full brightness)
* @return: BRGColor structure with blue, red, green values (7-bit each)
*/
BRGColor getColorWithBrightness(LEDColor color, float brightness) {

    // Initialize result color
    BRGColor result;
    
    // Define all colors in RGB format, then convert to BRG with brightness
    switch(color) {
        case LEDColor::black:           // NEW: Off/no color
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(0, brightness);
            break;
            
        case LEDColor::red:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(0, brightness);
            break;
            
        case LEDColor::orange:
            result.blue = convertTo7Bit(45, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(97, brightness);
            break;
            
        case LEDColor::lightorange:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(148, brightness);
            break;
            
        case LEDColor::warmyellow:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(213, brightness);
            break;
            
        case LEDColor::yellow:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
            
        case LEDColor::lime:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(144, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
            
        case LEDColor::green:
            result.blue = convertTo7Bit(0, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
            
        case LEDColor::mint:
            result.blue = convertTo7Bit(165, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
            
        case LEDColor::cyan:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
            
        case LEDColor::turquise:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(206, brightness);
            break;
            
        case LEDColor::blue:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(0, brightness);
            result.green = convertTo7Bit(49, brightness);
            break;
            
        case LEDColor::plum:
            result.blue = convertTo7Bit(218, brightness);
            result.red = convertTo7Bit(69, brightness);
            result.green = convertTo7Bit(49, brightness);
            break;
            
        case LEDColor::violet:
            result.blue = convertTo7Bit(217, brightness);
            result.red = convertTo7Bit(125, brightness);
            result.green = convertTo7Bit(41, brightness);
            break;
            
        case LEDColor::purple:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(229, brightness);
            result.green = convertTo7Bit(18, brightness);
            break;
            
        case LEDColor::magenta:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(0, brightness);
            break;
            
        case LEDColor::fuchsia:
            result.blue = convertTo7Bit(136, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(0, brightness);
            break;
            
        case LEDColor::white:
            result.blue = convertTo7Bit(255, brightness);
            result.red = convertTo7Bit(255, brightness);
            result.green = convertTo7Bit(255, brightness);
            break;
    }
    
    return result;
}

// =============================================================================
// MAIN LED SYSTEM FUNCTIONS
// =============================================================================

/*
* Initializes the LED controller system:
* 1. Stores the device for automatic sending
* 2. Initializes the LED buffer to default values (sets all LEDs to clear)
* 3. Initializes the state storage arrays to default values
*
* @param device: Pointer to the opened HID device
* @return: true if initialization successful, false if error
*/
bool initializeLEDController(hid_device* device) {
    // Step 1: Check if device is valid
    if (device == nullptr) {
        std::cerr << "Error: Device is null in initializeLEDController()" << std::endl;
        return false;
    }
    
    // Step 2: Store device for automatic sending
    current_device = device;
    
    // Step 3: Initialize LED buffer to all zeros (all LEDs off)
    memset(led_buffer, 0, LED_REPORT_SIZE);
    
    // Step 4: Set the report ID (first byte must be 0x80)
    led_buffer[0] = LED_REPORT_ID;

    // Step 5: Initialize state storage arrays to default values
    // SPECIAL BUTTONS: Initialize special button states to off (color irrelevant for special buttons)
    for (int i = 0; i < 5; i++) {
        special_states[i] = {0.0f};
    }
    // CONTROL BUTTONS: Initialize control button states to off (color irrelevant for control buttons)
    for (int i = 0; i < 3; i++) {
        control_states[i] = {0.0f};
    }
    // STOP BUTTONS: Initialize stop button states to off (color irrelevant for stop buttons)
    for (int i = 0; i < 4; i++) {
        stop_states[i] = {0.0f};
    }
    // MATRIX:Initialize matrix states to black/off
    for (int row = 1; row <= 4; row++) {
        for (int col = 1; col <= 4; col++) {
            matrix_states[row][col] = {LEDColor::black, 0.0f};
        }
    }

    // Step 6: Send initial empty report to turn off all LEDs
    bool success = sendLEDReport(device);
    
    if (success) {
        std::cout << "  - LED controller initialized successfully - all LEDs off, state storage ready" << std::endl;
    } else {
        std::cerr << "Error: Failed to send initial LED report" << std::endl;
    }
    
    return success;
}

/*
* Sends the current LED buffer to the F1 device
* This function actually communicates with the hardware
* 
* @param device: Pointer to the opened HID device
* @return: true if send successful, false if error
*/
bool sendLEDReport(hid_device* device) {
    // Step 1: Check if device is valid
    if (device == nullptr) {
        std::cerr << "Error: Device is null in sendLEDReport()" << std::endl;
        return false;
    }
    
    // Step 2: Send the 81-byte LED report to the F1
    int bytes_sent = hid_write(device, led_buffer, LED_REPORT_SIZE);
    
    // Step 3: Check if the send operation was successful
    if (bytes_sent < 0) {
        std::cerr << "Error: Failed to send LED report. Expected " 
                  << LED_REPORT_SIZE << " bytes, sent " << bytes_sent << " bytes" << std::endl;
        return false;
    }
    
    // Step 4: Verify correct number of bytes were sent
    if (bytes_sent != LED_REPORT_SIZE) {
        std::cerr << "Warning: Partial LED report sent. Expected " 
                  << LED_REPORT_SIZE << " bytes, sent " << bytes_sent << " bytes" << std::endl;
        return false;
    }
    
    // Step 5: Success!
    return true;
}

/*
* Clears all LEDs (turns them off) and sends the update to the F1
* Also clears the state storage for all LEDs
* This is useful for resetting the LED state
*/
void clearAllLEDs() {
    // Step 1: Clear the entire buffer except the report ID
    memset(led_buffer + 1, 0, LED_REPORT_SIZE - 1);  // Skip first byte (report ID)
    
    // Step 2: Clear state storage arrays to match
    // SPECIAL BUTTONS: Clear special button states to off (color irrelevant for special buttons)
    for (int i = 0; i < 5; i++) {
        special_states[i] = {0.0f};
    }
    // CONTROL BUTTONS: Clear control button states to off (color irrelevant for control buttons)
    for (int i = 0; i < 3; i++) {
        control_states[i] = {0.0f};
    }
    // STOP BUTTONS: Clear stop button states to off (color irrelevant for stop buttons)
    for (int i = 0; i < 4; i++) {
        stop_states[i] = {0.0f};
    }
    // MATRIX: Clear matrix states to black/off
    for (int row = 1; row <= 4; row++) {
        for (int col = 1; col <= 4; col++) {
            matrix_states[row][col] = {LEDColor::black, 0.0f};
        }
    }

    // Step 3: Send the cleared buffer to the F1
    if (current_device != nullptr) {
        sendLEDReport(current_device);
        std::cout << "All LEDs cleared" << std::endl;
    } else {
        std::cerr << "Warning: No device connected, LEDs cleared in buffer only" << std::endl;
    }
}

// =============================================================================
// MATRIX LED FUNCTIONS - Control the 4x4 RGB button grid
// =============================================================================

/*
* Sets a matrix button LED to a specific color and brightness
* Saves the original color and brightness in the state storage
* Matrix buttons are arranged in a 4x4 grid with RGB LEDs (BRG format)
* 
* @param row: Matrix row (1-4)
* @param col: Matrix column (1-4)
* @param color: Color to set (using LEDColor enum)
* @param brightness: Brightness level (0.0 = off, 1.0 = full brightness)
* @param store_led_state: Whether to store the LED state (original color/brightness)
* @return: true if successful, false if error
*/
bool setMatrixButtonLED(int row, int col, LEDColor color, float brightness, bool store_led_state) {
    // Step 1: Validate matrix position
    if (!isValidMatrixPosition(row, col)) {
        std::cerr << "Error: Invalid matrix position (" << row << "," << col 
                  << "). Must be 1-4 for both row and column." << std::endl;
        return false;
    }

    // Step 2: Clamp brightness to valid range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Step 3: Save the original color and brightness in the state storage
    // This happens BEFORE any conversion, preserving exact original values
    // Save state only if requested
    if (store_led_state) {
        matrix_states[row][col] = {color, brightness};
    }

    // Step 4: Calculate the byte position for this matrix button
    // Matrix mapping: (row-1) * 4 + (col-1) gives button index (0-15)
    // Each button has 3 bytes (BRG), so multiply by 3
    int button_index = (row - 1) * MATRIX_COLS + (col - 1);
    int base_byte = LED_BYTE_MATRIX_START + (button_index * MATRIX_LEDS_PER_BUTTON);
    
    // Step 5: Get the color values in BRG format with brightness
    BRGColor brg_color = getColorWithBrightness(color, brightness);
    
    // Step 6: Set the three LED bytes for this button (Blue, Red, Green order)
    led_buffer[base_byte]     = brg_color.blue;   // Blue LED
    led_buffer[base_byte + 1] = brg_color.red;    // Red LED  
    led_buffer[base_byte + 2] = brg_color.green;  // Green LED
    
    // Step 7: Send the updated buffer to the F1
    if (current_device != nullptr) {
        return sendLEDReport(current_device);
    } else {
        std::cerr << "Warning: No device connected, LED set in buffer only" << std::endl;
        return false;
    }
}

// =============================================================================
// SPECIAL BUTTON LED FUNCTIONS - Control single-brightness special buttons
// =============================================================================

/*
* Sets a special button LED to a specific brightness
* Saves the original brightness in state storage
* Special buttons only have single LEDs, so only brightness is controlled
* 
* @param button: Which special button to control (using enum)
* @param brightness: Brightness level (0.0 = off, 1.0 = full brightness)
* @param store_led_state: Whether to store the LED state (original brightness)
* @return: true if successful, false if error
*/
bool setSpecialButtonLED(SpecialLEDButton button, float brightness, bool store_led_state) {
    // Step 1: Get array index for this button
    int index = getSpecialButtonIndex(button);
    if (index < 0 || index > 4) {
        std::cerr << "Error: Invalid special button in setSpecialButtonLED()" << std::endl;
        return false;
    }

    // Step 2: Clamp brightness to valid range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Step 3: Save the original brightness in state storage
    // This happens BEFORE any conversion, preserving exact original value
    // Save only if requested
    if (store_led_state) {
        special_states[index] = {brightness};
    }

    // Step 4: Convert brightness to 7-bit value (F1 hardware requirement)
    unsigned char led_value = convertTo7Bit(255, brightness);  // Use max 7-bit value with brightness
    
    // Step 5: Determine which byte to set based on the button
    int byte_position;
    
    switch (button) {
        case SpecialLEDButton::BROWSE:
            byte_position = LED_BYTE_SPECIAL_START + LED_OFFSET_BROWSE;
            break;
        case SpecialLEDButton::SIZE:
            byte_position = LED_BYTE_SPECIAL_START + LED_OFFSET_SIZE;
            break;
        case SpecialLEDButton::TYPE:
            byte_position = LED_BYTE_SPECIAL_START + LED_OFFSET_TYPE;
            break;
        case SpecialLEDButton::REVERSE:
            byte_position = LED_BYTE_SPECIAL_START + LED_OFFSET_REVERSE;
            break;
        case SpecialLEDButton::SHIFT:
            byte_position = LED_BYTE_SPECIAL_START + LED_OFFSET_SHIFT;
            break;
        default:
            std::cerr << "Error: Unknown special button requested" << std::endl;
            return false;
    }
    
    // Step 6: Set the LED value in the buffer
    led_buffer[byte_position] = led_value;
    
    // Step 7: Send the updated buffer to the F1
    if (current_device != nullptr) {
        return sendLEDReport(current_device);
    } else {
        std::cerr << "Warning: No device connected, LED set in buffer only" << std::endl;
        return false;
    }
}

// =============================================================================
// CONTROL BUTTON LED FUNCTIONS - Control control button LEDs
// =============================================================================

/*
* Sets the LED of a control button to a specific brightness
* Saves the original brightness in state storage
* Control buttons only have single LEDs, so only brightness is controlled
*
* @param button: Which control button to control (using enum)
* @param brightness: Brightness level (0.0 = off, 1.0 = full brightness)
* @param store_led_state: Whether to store the LED state (original brightness)
* @return: true if successful, false if error
*/
bool setControlButtonLED(ControlLEDButton button, float brightness, bool store_led_state) {
    // Step 1: Get array index for this button
    int index = getControlButtonIndex(button);
    if (index < 0 || index > 2) {
        std::cerr << "Error: Invalid control button in setControlButtonLED()" << std::endl;
        return false;
    }

    // Step 2: Clamp brightness to valid range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Step 3: Save original brightness
    // This happens BEFORE any conversion, preserving exact original value
    // Save only if requested
    if (store_led_state) {
        control_states[index] = {brightness};
    }

    // Step 4: Convert brightness to 7-bit value
    unsigned char led_value = convertTo7Bit(255, brightness);

    // Step 5: Determine byte position for this control button
    int byte_position;
    
    switch (button) {
        case ControlLEDButton::CAPTURE:
            byte_position = LED_BYTE_CONTROL_START + LED_OFFSET_CAPTURE;
            break;
        case ControlLEDButton::QUANT:
            byte_position = LED_BYTE_CONTROL_START + LED_OFFSET_QUANT;
            break;
        case ControlLEDButton::SYNC:
            byte_position = LED_BYTE_CONTROL_START + LED_OFFSET_SYNC;
            break;
        default:
            std::cerr << "Error: Unknown control button requested" << std::endl;
            return false;
    }
    
    // Step 6: Set the LED value in the buffer
    led_buffer[byte_position] = led_value;
    
    // Step 7: Send the updated buffer to the F1
    if (current_device != nullptr) {
        return sendLEDReport(current_device);
    } else {
        std::cerr << "Warning: No device connected, LED set in buffer only" << std::endl;
        return false;
    }
}

// =============================================================================
// STOP BUTTON LED FUNCTIONS - Control stop button LEDs (2 LEDs per button)
// =============================================================================

/*
* Sets both LEDs of a stop button to a specific brightness
* Saves the original brightness in state storage
* Each stop button has 2 separate LEDs (left and right)
* 
* @param button: Which stop button to control (STOP1-STOP4)
* @param brightness: Brightness level (0.0 = off, 1.0 = full brightness)
* @param store_led_state: Whether to store the LED state (original brightness)
* @return: true if successful, false if error
*/
bool setStopButtonLED(StopLEDButton button, float brightness, bool store_led_state) {    
    // Step 1: Get array index for this button
    int index = getStopButtonIndex(button);
    if (index < 0 || index > 3) {
        std::cerr << "Error: Invalid stop button in setStopButtonLED()" << std::endl;
        return false;
    }

    // Step 2: Clamp brightness to valid range
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    // Step 3: Save original brightness
    // This happens BEFORE any conversion, preserving exact original value
    // Save only if requested
    if (store_led_state) {
        stop_states[index] = {brightness};
    }

    // Step 4: Convert brightness to 7-bit value
    unsigned char led_value = convertTo7Bit(255, brightness);

    // Step 5: Determine byte positions for both LEDs of this stop button
    int left_byte, right_byte;
    
    switch (button) {
        case StopLEDButton::STOP1:
            right_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP1_RIGHT;
            left_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP1_LEFT;
            break;
        case StopLEDButton::STOP2:
            right_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP2_RIGHT;
            left_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP2_LEFT;
            break;
        case StopLEDButton::STOP3:
            right_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP3_RIGHT;
            left_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP3_LEFT;
            break;
        case StopLEDButton::STOP4:
            right_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP4_RIGHT;
            left_byte = LED_BYTE_STOP_START + LED_OFFSET_STOP4_LEFT;
            break;
    }
    
    // Step 6: Set both LED values in the buffer
    led_buffer[right_byte] = led_value;
    led_buffer[left_byte] = led_value;
    
    // Step 7: Send the updated buffer to the F1
    if (current_device != nullptr) {
        return sendLEDReport(current_device);
    } else {
        std::cerr << "Warning: No device connected, LED set in buffer only" << std::endl;
        return false;
    }
}

// =============================================================================
// UTILITY FUNCTIONS FOR DEBUGGING
// =============================================================================

/*
* Prints the current LED state storage arrays
* Useful for debugging toggle system and verifying state storage accuracy
*/
void printLEDStates() {
    std::cout << "=== LED State Storage ===" << std::endl;
    
    // Print matrix button states
    std::cout << "Matrix button states (original values):" << std::endl;
    for (int row = 1; row <= 4; row++) {
        std::cout << "  Row " << row << ": ";
        for (int col = 1; col <= 4; col++) {
            LEDStateMatrix state = matrix_states[row][col];
            std::cout << "(" << (int)state.color << "," << std::fixed 
                      << std::setprecision(2) << state.brightness << ") ";
        }
        std::cout << std::endl;
    }
    
    // Print special button states
    std::cout << "Special button states (original brightness):" << std::endl;
    const char* button_names[] = {"BROWSE", "SIZE", "TYPE", "REVERSE", 
                                  "SHIFT"};
    for (int i = 0; i < 5; i++) {
        std::cout << "  " << button_names[i] << ": " << std::fixed 
                  << std::setprecision(2) << special_states[i].brightness << std::endl;
    }

    // Print control button states
    std::cout << "Control button states (original brightness):" << std::endl;
    const char* control_button_names[] = {"CAPTURE", "QUANT", "SYNC"};
    for (int i = 0; i < 3; i++) {
        std::cout << "  " << control_button_names[i] << ": " << std::fixed
                  << std::setprecision(2) << control_states[i].brightness << std::endl;
    }

    // Print stop button states
    std::cout << "Stop button states (original brightness):" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  STOP" << (i + 1) << ": " << std::fixed 
                  << std::setprecision(2) << stop_states[i].brightness << std::endl;
    }

    std::cout << "=========================" << std::endl;
}

/*
* Prints the current LED report buffer in hexadecimal format
* Useful for debugging and understanding what will be sent to the F1
*/
void printLEDReport() {
    std::cout << "Current LED Report (81 bytes):" << std::endl;
    
    // Print report ID
    std::cout << "Report ID: 0x" << std::hex << std::setfill('0') << std::setw(2) 
              << (int)led_buffer[0] << std::endl;
    
    // Print in groups for easier reading
    std::cout << "7-Seg Right (1-8):   ";
    for (int i = 1; i <= 8; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "7-Seg Left (9-16):   ";
    for (int i = 9; i <= 16; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Special (17-21):     ";
    for (int i = 17; i <= 21; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Control (22-24):     ";
    for (int i = 22; i <= 24; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
    }

    std::cout << "Matrix (25-72):      ";
    for (int i = 25; i <= 72; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
        if ((i - 24) % 12 == 0) std::cout << std::endl << "                     ";
    }
    std::cout << std::endl;
    
    std::cout << "Stop (73-80):        ";
    for (int i = 73; i <= 80; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) 
                  << (int)led_buffer[i] << " ";
    }
    std::cout << std::dec << std::endl;  // Return to decimal mode
}

/*
* Tests all LED types by briefly lighting them up
* Useful for verifying that all LEDs are working
* 
* @param device: Pointer to the opened HID device
*/
void testAllLEDs() {
    std::cout << "Testing all LEDs..." << std::endl;
    
    // Test matrix LEDs - cycle through colors
    std::cout << "Testing matrix LEDs with different colors..." << std::endl;
    LEDColor test_colors[] = {LEDColor::red, LEDColor::green, LEDColor::blue, LEDColor::white};
    
    for (int i = 0; i < 4; i++) {
        for (int row = 1; row <= 4; row++) {
            for (int col = 1; col <= 4; col++) {
                setMatrixButtonLED(row, col, test_colors[i], 0.5f, false);
                usleep(100000);  // Sleep for 100ms
            }
        }
        std::cout << "Matrix LEDs set to color " << i + 1 << "/4" << std::endl;
        // In a real application, you'd add a delay here
        usleep(500000);  // Sleep for 500ms to see the color
    }
    
    // Test special button LEDs
    std::cout << "Testing special button LEDs..." << std::endl;
    SpecialLEDButton special_buttons[] = {
        SpecialLEDButton::BROWSE, SpecialLEDButton::SIZE, SpecialLEDButton::TYPE,
        SpecialLEDButton::REVERSE, SpecialLEDButton::SHIFT
    };
    
    for (int i = 0; i < 5; i++) {
        setSpecialButtonLED(special_buttons[i], 0.8f, false);
        usleep(100000);  // Sleep for 100ms
    }
    std::cout << "All special button LEDs turned on" << std::endl;

    // Test control button LEDs
    std::cout << "Testing control button LEDs..." << std::endl;
    ControlLEDButton control_buttons[] = {
        ControlLEDButton::CAPTURE, ControlLEDButton::QUANT, ControlLEDButton::SYNC
    };
    for (int i = 0; i < 3; i++) {
        setControlButtonLED(control_buttons[i], 0.8f, false);
        usleep(100000);  // Sleep for 100ms
    }

    // Test stop button LEDs
    std::cout << "Testing stop button LEDs..." << std::endl;
    for (int i = 1; i <= 4; i++) {
        setStopButtonLED(static_cast<StopLEDButton>(i), 0.8f, false);
        usleep(100000);  // Sleep for 100ms
    }
    std::cout << "All stop button LEDs turned on" << std::endl;

    // Sleep to see all colors
    usleep(1000000);  // Sleep for 1 second

    // Clear all LEDs at the end
    std::cout << "Test complete - clearing all LEDs" << std::endl;
    clearAllLEDs();
}