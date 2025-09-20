#include "include/startup_sequence.h"       // Include header file
#include "include/led_controller_base.h"         // Include LED controller header file

#include <iostream>      // iostream gives std::cout
#include <unistd.h>      // unistd.h gives usleep() — sleep for microseconds
#include <cstring>       // cstring gives memset() — memory manipulation
#include <thread>               // For std::this_thread::sleep_for (modern C++ sleep)
#include <chrono>               // For std::chrono::milliseconds
// #include <hidapi/hidapi.h>   // included already in header


// =============================================================================
// START UP SEQUENCE
// =============================================================================

/*
* Runs a LED wave animation on the F1 matrix buttons
* Creates a diagonal wave pattern that spreads across the 4x4 matrix
* 
* @param device: Pointer to the opened HID device
*/

void startupSequence(hid_device* device) {
    
    // Step 1: Check if device is valid
    if (device == nullptr) {
        std::cerr << "Error: Invalid device handle for startup sequence" << std::endl;
        return;
    }
    
    // Step 2: Animation timing configuration
    const int step_delay_ms = 50;  // 50ms between each animation step
    
    // Step 3: Start the animation
    std::cout << "  - Running startup LED sequence..." << std::endl;
    
    // Animation creates a diagonal wave pattern:
    // The pattern moves from bottom-right to top-left in diagonal waves
    // Each step lights up LEDs along diagonal lines, creating a wave effect
    
    // =============================================================================
    // ANIMATION STEP 1-2: First diagonal
    // =============================================================================
    
    // Step 1: Start with single LED at (4,4) - dim green
    setMatrixButtonLED(4, 4, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 2:
    setMatrixButtonLED(4, 4, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 3-4: Second diagonal
    // =============================================================================
    
    // Step 3:
    setMatrixButtonLED(3, 4, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(4, 3, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 4:
    setMatrixButtonLED(3, 4, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(4, 3, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 5-6: Third diagonal
    // =============================================================================
    
    // Step 5:
    setMatrixButtonLED(4, 4, LEDColor::green, 0.5f, false);  // Fade corner
    setMatrixButtonLED(2, 4, LEDColor::green, 0.5f, false);  // New LEDs dim
    setMatrixButtonLED(3, 3, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(4, 2, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 6:
    setMatrixButtonLED(4, 4, LEDColor::black, 0.0f, false); // Turn off
    setMatrixButtonLED(2, 4, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(3, 3, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(4, 2, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 7-8: Fourth diagonal (main diagonal)
    // =============================================================================
    
    // Step 7:
    setMatrixButtonLED(3, 4, LEDColor::green, 0.5f, false);  // Fade second diagonal
    setMatrixButtonLED(4, 3, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(1, 4, LEDColor::green, 0.5f, false);  // New main diagonal dim
    setMatrixButtonLED(2, 3, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(3, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(4, 1, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 8:
    setMatrixButtonLED(3, 4, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(4, 3, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(1, 4, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(2, 3, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(3, 2, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(4, 1, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 9-10: Fifth diagonal
    // =============================================================================
    
    // Step 9:
    setMatrixButtonLED(2, 4, LEDColor::green, 0.5f, false);  // Fade third diagonal
    setMatrixButtonLED(3, 3, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(4, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(1, 3, LEDColor::green, 0.5f, false);  // New fifth diagonal dim
    setMatrixButtonLED(2, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(3, 1, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 10:
    setMatrixButtonLED(2, 4, LEDColor::black, 0.0f, false);  // Turn off third diagonal
    setMatrixButtonLED(3, 3, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(4, 2, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(1, 3, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(2, 2, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(3, 1, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 11-12: Sixth diagonal
    // =============================================================================
    
    // Step 11:
    setMatrixButtonLED(1, 4, LEDColor::green, 0.5f, false);  // Fade main diagonal
    setMatrixButtonLED(2, 3, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(3, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(4, 1, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(1, 2, LEDColor::green, 0.5f, false);  // New sixth diagonal dim
    setMatrixButtonLED(2, 1, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 12:
    setMatrixButtonLED(1, 4, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(2, 3, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(3, 2, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(4, 1, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(1, 2, LEDColor::green, 1.0f, false);
    setMatrixButtonLED(2, 1, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 13-14: Seventh diagonal (top-left corner)
    // =============================================================================
    
    // Step 13:
    setMatrixButtonLED(1, 3, LEDColor::green, 0.5f, false);  // Fade fifth diagonal
    setMatrixButtonLED(2, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(3, 1, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(1, 1, LEDColor::green, 0.5f, false);  // Final corner dim
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 14:
    setMatrixButtonLED(1, 3, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(2, 2, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(3, 1, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(1, 1, LEDColor::green, 1.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // =============================================================================
    // ANIMATION STEP 15-18: Wave fade out
    // =============================================================================
    
    // Step 15:
    setMatrixButtonLED(1, 2, LEDColor::green, 0.5f, false);
    setMatrixButtonLED(2, 1, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 16:
    setMatrixButtonLED(1, 2, LEDColor::black, 0.0f, false);
    setMatrixButtonLED(2, 1, LEDColor::black, 0.0f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));
    
    // Step 17: Final fade - corner dims
    setMatrixButtonLED(1, 1, LEDColor::green, 0.5f, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));

    // Step 18: Final fade - all dims
    setMatrixButtonLED(1, 1, LEDColor::black, 0.0f, false);

    // =============================================================================
    // FINAL STATE: Turn on all LEDs at specified brightness
    // =============================================================================
    
    std::cout << "  - Animation complete, setting final LED state..." << std::endl;
    
    // Set all special button LEDs
    setSpecialButtonLED(SpecialLEDButton::BROWSE, 0.1f, true);
    setSpecialButtonLED(SpecialLEDButton::SIZE, 0.1f, true);
    setSpecialButtonLED(SpecialLEDButton::TYPE, 0.1f, true);
    setSpecialButtonLED(SpecialLEDButton::REVERSE, 0.1f, true);
    setSpecialButtonLED(SpecialLEDButton::SHIFT, 0.1f, true);

    // Set all control buttons
    setControlButtonLED(ControlLEDButton::CAPTURE, 0.1f, true);
    setControlButtonLED(ControlLEDButton::QUANT, 0.1f, true);
    setControlButtonLED(ControlLEDButton::SYNC, 0.1f, true);

    // Set all stop button LEDs
    setStopButtonLED(StopLEDButton::STOP1, 0.1f, true);
    setStopButtonLED(StopLEDButton::STOP2, 0.1f, true);
    setStopButtonLED(StopLEDButton::STOP3, 0.1f, true);
    setStopButtonLED(StopLEDButton::STOP4, 0.1f, true);
 
    std::cout << "  - Startup sequence completed!" << std::endl;
}