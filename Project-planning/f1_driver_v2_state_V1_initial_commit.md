# Traktor Kontrol F1 Driver V2 - MIDI Hardware Interface

C++ project using HIDAPI and RtMidi to create a "dumb" hardware driver for the Traktor Kontrol F1 device. Bidirectional MIDI communication: reads HID inputs → sends MIDI, receives MIDI → controls LEDs. Built with CMake, developed in VS Code on macOS/Ubuntu.

## Project Structure

```{}
f1_driver_v2/
├── src/
│   ├── main.cpp                    Rework required
│   ├── input_reader_base.cpp       Rework required
│   ├── input_reader_wheel.cpp      Rework required
│   ├── input_reader_knob.cpp       Rework required
│   ├── input_reader_fader.cpp      Rework required
│   ├── startup_sequence.cpp        Complete
│   ├── led_controller_base.cpp     Rework required
│   ├── led_controller_display.cpp  Rework required
│   └── midi_handler.cpp            Not existing
├── include/
│   ├── input_reader_base.h         Rework required
│   ├── input_reader_wheel.h        Rework required
│   ├── input_reader_knob.h         Rework required
│   ├── input_reader_fader.h        Rework required
│   ├── startup_sequence.h          Rework required
│   ├── led_controller_base.h       Rework required
│   ├── led_controller_display.h    Rework required
│   └── midi_handler.h              NEW - MIDI interface definitions
└── build/
    └── f1_driver                   Executable
```

## Technical Reference

### HID Communication

- **Input:** 22-byte reports (button states, analog values)  
- **Output:** 81-byte reports (LED colors, display segments)

#### Input Byte Layout (22 bytes total)

- **Input:** 22 bytes (ID=0x01, button data in bytes 1-4, analog in bytes 5-21)

- Byte 0: Report ID (always 0x01)
- Byte 1: Matrix buttons (1,1) to (4,2) - bits 7-0
- Byte 2: Matrix buttons (1,3) to (4,4) - bits 7-0
- Byte 3: Special buttons - bits 7-0 (SHIFT=0x80, BROWSE=0x08, etc.)
- Byte 4: Stop buttons + control buttons - bits 7-0
- Byte 5: Selector wheel state 0 - 255
- Bytes 6-13: Knobs 1-4 (2 bytes each, LSB first)
- Bytes 14-21: Faders 1-4 (2 bytes each, LSB first)

#### Output Byte Layout (81 bytes total)

- **Output:** 81 bytes

- Byte 0: Report ID (always 0x80)
- Bytes 1-8: Right 7-segment display
- Bytes 9-16: Left 7-segment display
- Bytes 17-24: Special buttons (single brightness each)
- Bytes 25-72: Matrix RGB LEDs (16 buttons × 3 colors BRG format)
- Bytes 73-80: Stop buttons (2 LEDs per stop button)

### Matrix Button Layout

- 4x4 grid using 1-4 indexing (row 1-4, col 1-4)
- Input mapping: byte 1 covers (1,1)-(4,2), byte 2 covers (1,3)-(4,4)
- LED mapping: 3 bytes per button (BRG format), starting at byte 25

### Button Categories

- **Special buttons (5):** BROWSE, SIZE, TYPE, REVERSE, SHIFT
- **Control buttons (3):** CAPTURE, QUANT, SYNC  
- **Stop buttons (4):** STOP1, STOP2, STOP3, STOP4
- **Matrix buttons (16):** 4x4 grid with RGB LEDs

### Analog Controls

- **Knobs (4):** 12-bit precision, bytes 6-13, LSB first
- **Faders (4):** 12-bit precision, bytes 14-21, LSB first

### MIDI Dependencies

- **RtMidi library** for cross-platform MIDI
- **Virtual MIDI ports** for communication with external software
- **Standard MIDI messages** (Note On/Off, Control Change)

### Build Requirements

```cmake
# CMakeLists.txt additions
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
target_link_libraries(f1_driver ${RTMIDI_LIBRARIES})
```

## Core Systems

### 1. Input reader System → MIDI Output

#### input_reader_base

- **Files:** `input_reader_base.h/cpp`
- **Capabilities:** Read 22-byte HID reports, detect all button presses. Sends MIDI for button presses
- **MIDI Output:** Notes for buttons, CC for continuous controls
- **Functions:** `readInputReport()`, `isSpecialButtonPressed()`, `isStopButtonPressed()`, `isControlButtonPressed()`, `isMatrixButtonPressed()`
- **Enums:** `SpecialButton`, `StopButton`, `ControlButton`

#### input_reader_wheel

- **Files:** `input_reader_wheel.h/cpp`  
- **Capabilities:**  Read 22-byte HID reports, detect infinite selector wheel state, detect clockwise or counter clockwise rotation. Send MIDI accordingly
- **MIDI Output:** Notes for buttons, CC for continuous controls
- **Functions:** `initialize()`, `calculateDirection()`, `checkWheelRotation()`

#### input_reader_fader

- **Key Features:** LSB-first byte extraction from bytes 6-13 (knobs) and 14-21 (faders)
- **Files:** `input_reader_knob.h/cpp`
- **Capabilities:** Read all 4 knobs with 12-bit precision, normalized 0-127 output, change detection
- **Functions:** `getKnobValue()`, `hasKnobChanged()`, `updateKnobStates()`, `getRawKnobValue()`, `printKnobValues()`

#### input_rader_knobs

- **Key Features:** LSB-first byte extraction from bytes 6-13 (knobs) and 14-21 (faders)
- **Files:** `input_reader_fader.h/cpp`
- **Capabilities:** Read all 4 faders with 12-bit precision, normalized 0-127 output, change detection
- **Functions:** `getFaderValue()`, `hasFaderChanged()`, `updateFaderStates()`, `getRawFaderValue()`, `printFaderValues()`

### 2. LED Controller System ← MIDI Input

#### led_controller_base

- **Files:** `led_controller.h/cpp`  
- **Capabilities:** Receive MIDI messages → Control all F1 LEDs with 18-color system, persistent LED buffer, LED state storage
- **Functions:** `setMatrixButtonLED()`, `setSpecialButtonLED()`, `setControlButtonLED()`, `setStopButtonLED()`
- **MIDI Input:** Note velocity = LED brightness, CC = display/parameters
- **Features:** BRG format conversion, 7-bit hardware conversion, automatic HID sending
- **Enums:** `LEDColor`, `SpecialLEDButton`, `ControlLEDButton`, `StopLEDButton`

#### led_controller_display

- **Files:** `led_controller_display.h/cpp`  
- **Capabilities:** Receive MIDI messages → Control dual 7-segment displays, set numbers 0-99, set individual segments, set decimal points
- **MIDI Input:** Note velocity = LED brightness, CC = display/parameters
- **Functions:** `setDisplayNumber()`, `setDisplaySegment()`, `setDisplayDot()`

### 3. Bidirectional MIDI Handler

- **Files:** `midi_handler.h/cpp`
- **Capabilities:**
  - Create virtual MIDI ports "F1_Controller_Out" & "F1_Controller_In"
  - Send input data as MIDI to external software
  - Receive LED commands as MIDI from external software
- **Functions:**
  - `initializeMIDI()` - Setup RtMidi ports
  - `sendButtonMIDI()`, `sendAnalogMIDI()` - Input → MIDI
  - `receiveLEDMIDI()` - MIDI callback for LED control
  - `cleanup()` - Close MIDI connections

### 4. Startup Sequence

- **Files:** `startup_sequence.h/cpp`
- **Function:** Device initialization and LED test pattern, On driver startup only

## MIDI Protocol

### MIDI Output (F1 → External Software)

#### Matrix Buttons

```{}
Matrix (1,1) to (4,4) = Notes 36-51

Press:   {0x90, note, 127}  // Note On
Release: {0x80, note, 0}    // Note Off
```

#### Other Buttons

```{}
Special Buttons (SHIFT, BROWSE, etc.)   = Notes 60-64
Control Buttons (CAPTURE, QUANT, SYNC)  = Notes 65-67
Stop Buttons    (STOP1-4)               = Notes 68-72

Press:   {0x90, note, 127}  // Note On
Release: {0x80, note, 0}    // Note Off
```

#### Continuous Controls

```{}
Knobs 1-4:      CC 1-4   {0xB0, cc_num, value}
Faders 1-4:     CC 5-8   {0xB0, cc_num, value}
Selector Wheel: CC 10    {0xB0, 10, direction}
```

### MIDI Input (External Software → F1)

#### LED Control - UNCLEAR

```{}
Matrix LED:     {0x90, note_36-51, brightness_0-127}
Special LED:    {0x90, note_60-64, brightness_0-127}
Control LED:    {0x90, note_65-67, brightness_0-127}
Stop LED:       {0x90, note_68-72, brightness_0-127}
```

#### Display Control

```{}
Display Number: {0xB0, 15, value_0-99}
Left Dot:       {0xB0, 16, on/off_0-127}
Right Dot:      {0xB0, 17, on/off_0-127}
```

## Main Loop Pattern

```cpp
// =============================================================================
// F1 MIDI driver identifiers
// =============================================================================
const unsigned short VENDOR_ID = 0x17cc;
const unsigned short PRODUCT_ID = 0x1120;

// =============================================================================
// START
// =============================================================================
int main() {
    // Initialize HIDAPI
    int res = hid_init();
    hid_device *device = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
    
    // Initialize MIDI (NEW)
    MidiHandler midi_handler;
    midi_handler.initializeMIDI();
    
    // Initialize input readers
    WheelInputReader wheel_input_reader;        // Declare wheel reader system
    KnobInputReader knob_input_reader;          // Declare knob reader system
    FaderInputReader fader_input_reader;        // Declare fader reader system

     // Initialize led systems
    initializeLEDController(device);
    DisplayController display_controller;       // Declare display controller
    int current_effect_page = 1;                // Declare current effects page variable
    display_controller.setDisplayNumber(current_effect_page);   // Set first effects page on display
    display_controller.setDisplayDot(1, true);  // Turn on left dot to indicate page is loaded

    // Play startup sequence
    startupSequence(device);
    
    // =============================================================================
    // MAIN LOOP - Pure Hardware Interface
    // =============================================================================
    while (true) {
        readInputReport(device, input_data);
        
        // Send ALL input changes as MIDI (no logic, no filtering)
        midi_handler.sendButtonChanges(input_data);
        midi_handler.sendAnalogChanges(input_data, knob_reader, fader_reader);
        midi_handler.sendWheelChanges(input_data, wheel_reader);
        
        // LED updates happen automatically via MIDI input callback
    }
    
    midi_handler.cleanup();
    hid_close(device);
    hid_exit();
    return 0;
}
```

## Removed Systems (Moved to External Software)

- ~~Button toggle logic~~ → External MIDI software
- ~~Scene controller~~ → External MIDI software  
- ~~Zone-based matrix control~~ → External MIDI software
- ~~Effects page management~~ → External MIDI software

## Development Notes

- none
