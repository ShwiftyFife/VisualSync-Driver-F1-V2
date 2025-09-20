#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <rtmidi/RtMidi.h>    // RtMidi library for MIDI handling
#include <vector>             // For std::vector used in RtMidi
#include <iostream>           // For std::cout and std::cerr

// =============================================================================
// MIDI CONSTANTS - MIDI note mappings for F1 controller
// =============================================================================

// Matrix button MIDI note mapping (4x4 grid → Notes 36-51)
const int MIDI_NOTE_MATRIX_BASE = 36;  // Starting note for matrix (1,1)

// MIDI channel (0-based, so channel 1 = 0)
const int MIDI_CHANNEL = 0;

// MIDI message types
const unsigned char MIDI_NOTE_ON = 0x90;   // Note On message
const unsigned char MIDI_NOTE_OFF = 0x80;  // Note Off message
const unsigned char MIDI_VELOCITY_ON = 127; // Full velocity for button press
const unsigned char MIDI_VELOCITY_OFF = 0;  // Zero velocity for button release

// =============================================================================
// MATRIX BUTTON STATE TRACKING
// =============================================================================

struct MatrixButtonState {
    bool current_state[4][4];   // Current state of all matrix buttons
    bool previous_state[4][4];  // Previous state for change detection
    
    // Constructor to initialize all states to false
    MatrixButtonState() {
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                current_state[row][col] = false;
                previous_state[row][col] = false;
            }
        }
    }
};

// =============================================================================
// MIDI HANDLER CLASS
// =============================================================================

class MidiHandler {
private:
    RtMidiOut* midi_out;                    // MIDI output port
    RtMidiIn* midi_in;                      // MIDI input port (for future LED control)
    MatrixButtonState button_state;         // Track button states for change detection
    bool initialized;                       // Track initialization state
    
    // Helper functions
    int matrixPositionToMidiNote(int row, int col);
    void sendMidiMessage(std::vector<unsigned char>& message);
    
public:
    // Constructor and destructor
    MidiHandler();
    ~MidiHandler();
    
    // Initialization and cleanup
    bool initializeMIDI();
    void cleanup();
    
    // Matrix button MIDI functions
    void updateMatrixButtonStates(const unsigned char* input_buffer);
    void sendMatrixButtonPress(int row, int col);
    void sendMatrixButtonRelease(int row, int col);
};

#endif // MIDI_HANDLER_H