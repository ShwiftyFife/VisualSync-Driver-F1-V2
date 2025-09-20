#ifndef LED_CONTROLLER_DISPLAY_H
#define LED_CONTROLLER_DISPLAY_H

#include <cstdint>                  // For uint8_t type
#include "led_controller_base.h"

// =============================================================================
// CONSTANTS - 7-Segment digit patterns lookup table
// =============================================================================

// 7-Segment digit patterns: [middle, lower_right, upper_right, top, upper_left, lower_left, bottom]
// 127 = full brightness, 0 = off
extern const uint8_t DIGIT_PATTERNS[11][7]; // 0-9 + blank (index 10)

// =============================================================================
// LED CONTROLLER DISPLAY CLASS
// =============================================================================

class DisplayController {
private:
    // Private helper function to set individual display segments
    void setDisplaySegment(int display, int digit);

public:
    // Main display functions
    void setDisplayNumber(int number);
    void setDisplayDot(int display, bool on);
};

#endif // LED_CONTROLLER_DISPLAY_H