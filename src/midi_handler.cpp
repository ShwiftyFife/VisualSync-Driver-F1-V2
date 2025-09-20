#include "include/midi_handler.h"
#include "include/input_reader_base.h"  // For matrix button checking functions
#include "include/input_reader_knob.h"  // For knob input reading
#include "include/input_reader_fader.h" // For fader input reading

// =============================================================================
// CONSTRUCTOR AND DESTRUCTOR
// =============================================================================

MidiHandler::MidiHandler() : midi_out(nullptr), midi_in(nullptr), initialized(false) {
    // Constructor initializes pointers to null and sets initialized to false
}

MidiHandler::~MidiHandler() {
    // Destructor ensures cleanup is called
    cleanup();
}

// =============================================================================
// INITIALIZATION AND CLEANUP
// =============================================================================

bool MidiHandler::initializeMIDI() {
    try {
        // Step 1: Create MIDI output port
        std::cout << "- Initializing MIDI output..." << std::endl;
        midi_out = new RtMidiOut();
        
        // Create virtual MIDI output port
        midi_out->openPort(0);
        std::cout << "- Created MIDI output port: 0" << std::endl;
        //midi_out->openVirtualPort("F1_Controller_Out");
        //std::cout << "- Created virtual MIDI output port: F1_Controller_Out" << std::endl;
        
        // Step 2: Create MIDI input port (for future LED control)
        //std::cout << "- Initializing MIDI input..." << std::endl;
        //midi_in = new RtMidiIn();
        
        // Create virtual MIDI input port
        //midi_in->openVirtualPort("F1_Controller_In");
        //std::cout << "- Created virtual MIDI input port: F1_Controller_In" << std::endl;
        
        // Step 3: Print MIDI mappings for reference
        std::cout << "- Matrix buttons → MIDI notes 36-51" << std::endl;
        std::cout << "- Knobs 1-4 → MIDI CC 1-4" << std::endl;
        std::cout << "- Faders 1-4 → MIDI CC 5-8" << std::endl;
        initialized = true;
        std::cout << "- MIDI initialization successful!" << std::endl;
        
        return true;
        
    } catch (RtMidiError &error) {
        std::cerr << "MIDI initialization error: " << error.getMessage() << std::endl;
        cleanup();
        return false;
    }
}

void MidiHandler::cleanup() {
    std::cout << "- Cleaning up MIDI connections..." << std::endl;
    
    // Close and delete MIDI output
    if (midi_out) {
        midi_out->closePort();
        delete midi_out;
        midi_out = nullptr;
    }
    
    // Close and delete MIDI input
    if (midi_in) {
        midi_in->closePort();
        delete midi_in;
        midi_in = nullptr;
    }
    
    initialized = false;
    std::cout << "- MIDI cleanup complete." << std::endl;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

int MidiHandler::matrixPositionToMidiNote(int row, int col) {
    // Convert matrix position (1-4, 1-4) to MIDI note (36-51)
    // Matrix layout maps to sequential notes:
    // (1,1)=36, (2,1)=37, (3,1)=38, (4,1)=39
    // (1,2)=40, (2,2)=41, (3,2)=42, (4,2)=43
    // (1,3)=44, (2,3)=45, (3,3)=46, (4,3)=47
    // (1,4)=48, (2,4)=49, (3,4)=50, (4,4)=51
    
    // Convert to 0-based indices
    int row_index = row - 1;  // 1-4 → 0-3
    int col_index = col - 1;  // 1-4 → 0-3
    
    // Calculate MIDI note number
    int note = MIDI_NOTE_MATRIX_BASE + (col_index * 4) + row_index;
    
    return note;
}

void MidiHandler::sendMidiMessage(std::vector<unsigned char>& message) {
    if (!initialized || !midi_out) {
        std::cerr << "Error: MIDI not initialized, cannot send message" << std::endl;
        return;
    }
    
    try {
        midi_out->sendMessage(&message);
    } catch (RtMidiError &error) {
        std::cerr << "MIDI send error: " << error.getMessage() << std::endl;
    }
}

// =============================================================================
// MATRIX BUTTON MIDI FUNCTIONS
// =============================================================================

void MidiHandler::updateMatrixButtonStates(const unsigned char* input_buffer) {
    if (!initialized) {
        return; // Skip if MIDI not initialized
    }
    
    // Update button states and send MIDI for any changes
    for (int row = 1; row <= 4; row++) {
        for (int col = 1; col <= 4; col++) {
            // Convert to array indices (0-3)
            int row_index = row - 1;
            int col_index = col - 1;
            
            // Get current button state
            bool current_pressed = isMatrixButtonPressed(input_buffer, row, col);
            
            // Check if state has changed
            if (current_pressed != button_state.previous_state[row_index][col_index]) {
                if (current_pressed) {
                    // Button was just pressed
                    sendMatrixButtonPress(row, col);
                    std::cout << "Matrix button (" << row << "," << col << ") pressed - MIDI note " 
                             << matrixPositionToMidiNote(row, col) << std::endl;
                } else {
                    // Button was just released
                    sendMatrixButtonRelease(row, col);
                    std::cout << "Matrix button (" << row << "," << col << ") released - MIDI note " 
                             << matrixPositionToMidiNote(row, col) << std::endl;
                }
            }
            
            // Update states for next frame
            button_state.previous_state[row_index][col_index] = current_pressed;
            button_state.current_state[row_index][col_index] = current_pressed;
        }
    }
}

void MidiHandler::sendMatrixButtonPress(int row, int col) {
    // Create MIDI Note On message
    std::vector<unsigned char> message(3);
    
    message[0] = MIDI_NOTE_ON + MIDI_CHANNEL;           // Note On + channel
    message[1] = matrixPositionToMidiNote(row, col);    // Note number
    message[2] = MIDI_VELOCITY_ON;                      // Velocity (127)
    
    sendMidiMessage(message);
}

void MidiHandler::sendMatrixButtonRelease(int row, int col) {
    // Create MIDI Note Off message
    std::vector<unsigned char> message(3);
    
    message[0] = MIDI_NOTE_OFF + MIDI_CHANNEL;          // Note Off + channel
    message[1] = matrixPositionToMidiNote(row, col);    // Note number  
    message[2] = MIDI_VELOCITY_OFF;                     // Velocity (0)
    
    sendMidiMessage(message);
}

// =============================================================================
// ANALOG CONTROL MIDI FUNCTIONS
// =============================================================================

void MidiHandler::updateKnobStates(const unsigned char* input_buffer) {
    if (!initialized) {
        return; // Skip if MIDI not initialized
    }
    
    // Create knob input reader instance
    KnobInputReader knob_reader;
    
    // Update knob states and send MIDI for any changes
    for (int knob = 1; knob <= 4; knob++) {
        // Get current knob value (0-127)
        int current_value = (int)knob_reader.getKnobValue(input_buffer, knob);
        
        // Check if value has changed (allow for some tolerance)
        int previous_value = analog_state.previous_knob_values[knob - 1];
        
        if (previous_value == -1 || abs(current_value - previous_value) > 0) {
            // Send MIDI CC message for knob change
            sendKnobChange(knob, current_value);
            
            // Only print if significantly changed (reduce spam)
            if (previous_value == -1 || abs(current_value - previous_value) > 2) {
                std::cout << "Knob " << knob << " changed to " << current_value 
                         << " - MIDI CC " << (MIDI_CC_KNOB_BASE + knob - 1) << std::endl;
            }
        }
        
        // Update state for next frame
        analog_state.previous_knob_values[knob - 1] = current_value;
    }
}

void MidiHandler::updateFaderStates(const unsigned char* input_buffer) {
    if (!initialized) {
        return; // Skip if MIDI not initialized
    }
    
    // Create fader input reader instance
    FaderInputReader fader_reader;
    
    // Update fader states and send MIDI for any changes
    for (int fader = 1; fader <= 4; fader++) {
        // Get current fader value (0-127)
        int current_value = (int)fader_reader.getFaderValue(input_buffer, fader);
        
        // Check if value has changed (allow for some tolerance)
        int previous_value = analog_state.previous_fader_values[fader - 1];
        
        if (previous_value == -1 || abs(current_value - previous_value) > 0) {
            // Send MIDI CC message for fader change
            sendFaderChange(fader, current_value);
            
            // Only print if significantly changed (reduce spam)
            if (previous_value == -1 || abs(current_value - previous_value) > 2) {
                std::cout << "Fader " << fader << " changed to " << current_value 
                         << " - MIDI CC " << (MIDI_CC_FADER_BASE + fader - 1) << std::endl;
            }
        }
        
        // Update state for next frame
        analog_state.previous_fader_values[fader - 1] = current_value;
    }
}

void MidiHandler::sendKnobChange(int knob_number, int value) {
    // Create MIDI Control Change message
    std::vector<unsigned char> message(3);
    
    message[0] = MIDI_CC + MIDI_CHANNEL;                    // Control Change + channel
    message[1] = MIDI_CC_KNOB_BASE + knob_number - 1;      // CC number (1-4)
    message[2] = std::min(std::max(value, 0), 127);        // Clamp value to 0-127
    
    sendMidiMessage(message);
}

void MidiHandler::sendFaderChange(int fader_number, int value) {
    // Create MIDI Control Change message
    std::vector<unsigned char> message(3);
    
    message[0] = MIDI_CC + MIDI_CHANNEL;                    // Control Change + channel
    message[1] = MIDI_CC_FADER_BASE + fader_number - 1;    // CC number (5-8)
    message[2] = std::min(std::max(value, 0), 127);        // Clamp value to 0-127
    
    sendMidiMessage(message);
}

// =============================================================================
// STATUS FUNCTIONS
// =============================================================================
