// On Linux: build with control+shift+b (builds with cmake, declared in tasks.json)
// On Mac: build with command+shift+b (builds with cmake, declared in tasks.json)
// run in terminal with ./main


#include <iostream>			// For standard input output operations
#include <stdio.h> 			// stdio.h gives printf() — basic text output (like print() in R).
#include <wchar.h> 			// wchar.h gives wide-char support (wchar_t) used because USB string descriptors are UTF-16/wide.
#include <unistd.h> 		// unistd.h gives access to the POSIX operating system API.
#include <stdint.h> 		// stdint.h gives uint8_t support. uint8_t is an unsigned 8-bit integer type.
#include <hidapi/hidapi.h>  // HIDAPI library for USB HID device access
#include <fcntl.h>          // fcntl.h gives access to the POSIX file control API.
#include <cstring>          // string.h gives access to C-style string functions.
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>

#include "include/input_reader_base.h"				// Include input read module
#include "include/input_reader_fader.h"       // Include fader input read module
#include "include/input_reader_knob.h"				// Include knob input read module
#include "include/input_reader_wheel.h"				// Include wheel input read module

#include "include/led_controller_base.h"			// Include LED control module
#include "include/led_controller_display.h"		// Include display control module

#include "include/startup_sequence.h"					// Include startup effects module


//#include "include/led_controller_toggle.h"		// Include LED controller toggle module
//#include "include/led_scene_controller.h"     // Include LED scene controller module


// F1 device identifiers (same as before)
const unsigned short VENDOR_ID = 0x17cc;
const unsigned short PRODUCT_ID = 0x1120;


int main() {

		// =============================================================================
		// START UP SEQUENCE
		// =============================================================================

		// create start up message
		std::cout << "" << std::endl;
		std::cout << "=== Starting Visual Sync Kontrol F1 ===" << std::endl;
		std::cout << "" << std::endl;

		// Initialize HIDAPI, get return code, send error message if initialization fails
		int res = hid_init();
		if (res != 0) {
				std::cout << "- Failed to initialize HIDAPI" << std::endl;
				return 1;
		} else {
				std::cout << "- HID_API initialized successfully!" << std::endl;
		}

		// Open the device:
		// Declare pointer to HID device handle
		hid_device *device;
		// Declare button toggle system
		//ButtonToggleSystem btn_toggle_system;
		// Declare wheel reader system
		//WheelInputReader wheel_input_reader;
		// Declare knob input reader
		//KnobInputReader knob_input_reader;
		// Declare fader input reader
		//FaderInputReader fader_input_reader;
		// Declare current effects page variable
		//int current_effect_page = 1;
		// Delare display controller
		//DisplayController display_controller;
		// Declare LED scene controller
		//SceneController scene_controller;

		// Open the device using the VendorID, ProductID, and optionally the Serial number.
		// If the device is opened successfully, the pointer will not be null.
		device = hid_open(VENDOR_ID, PRODUCT_ID, NULL);
		if (device) {
				std::cout << "- Opening Traktor Kontrol F1..." << std::endl;

				// Initialize the LED controller
				//initializeLEDController(device);

				// Run startup sequence
				//startupSequence(device);

				// Initialize button toggle system
				//btn_toggle_system.initialize();

				// Initialize wheel input reader and set first page
				//wheel_input_reader.initialize();

				// Set first effects page on display
				// Turn on left dot to indicate page is loaded
				//display_controller.setDisplayNumber(current_effect_page);
				//display_controller.setDisplayDot(1, true);

				// Load first effects page scene
				//scene_controller.setEffectsPageScene(current_effect_page);

				// Send success message
				std::cout << "" << std::endl;
				std::cout << "- Traktor Kontrol F1 opened successfully!" << std::endl;
		} else {
				// Send error message and close program
				std::cout << "- Unable to open device..." << std::endl;
				std::cout << "Shutting down..." << std::endl;
				hid_exit();
						return 0;
		}

		// Send startup message how to terminate while(true) loop
		std::cout << "" << std::endl;
		std::cout << "+++ Press Ctrl+C to exit. +++" << std::endl;
		std::cout << "" << std::endl;


		// =============================================================================
		// STUFFs HAPPENIN HERE
		// =============================================================================

		while (true) {

				// =======================================
				// Read input report
				// =======================================
				unsigned char input_report_buffer[INPUT_REPORT_SIZE];
				if (!readInputReport(device, input_report_buffer)) {
						std::cerr << "Error, shutting down..." << std::endl;
						return -1;
				}

				// =======================================
				// Read and update Selector Wheel rotation
				// =======================================
				// Get wheel direction
				//WheelDirection selector_wheel_direction = wheel_input_reader.checkWheelRotation(input_report_buffer);

				// Select effects page accordingly
				//if (selector_wheel_direction == WheelDirection::CLOCKWISE) {
				//		// increase page by 1
        //		current_effect_page = std::min(current_effect_page + 1, 99);
				//		// Update display
				//		display_controller.setDisplayDot(1, false); // Turn off left dot when changing page
				//		display_controller.setDisplayNumber(current_effect_page);
				//	}
				//	else if (selector_wheel_direction == WheelDirection::COUNTER_CLOCKWISE) {
				//		// decrease page by 1
				//		current_effect_page = std::max(current_effect_page - 1, 1);
				//		// Update display
				//		display_controller.setDisplayDot(1, false); // Turn off left dot when changing page
				//		display_controller.setDisplayNumber(current_effect_page);
				//	}

				// Load effects page on selector wheel button press
				//if (isSpecialButtonPressed(input_report_buffer, SpecialButton::SELECTOR_WHEEL)) {
				//		// Turn on left dot to indicate page is loaded
				//		display_controller.setDisplayDot(1, true);
				//		// Load effects page scene
				//		scene_controller.setEffectsPageScene(current_effect_page);
				//		// Reset button states
				//		btn_toggle_system.resetAllToggleStates();
				//}

				// =======================================
				// Read and update Knob values
				// =======================================
				//knob_input_reader.printKnobValues(input_report_buffer);
				//float knob_value_1 = knob_input_reader.getKnobValue(input_report_buffer, 1);
				//float knob_value_2 = knob_input_reader.getKnobValue(input_report_buffer, 2);
				//float knob_value_3 = knob_input_reader.getKnobValue(input_report_buffer, 3);
				//float knob_value_4 = knob_input_reader.getKnobValue(input_report_buffer, 4);

				// =======================================
				// Read and update Fader values
				// =======================================
				//fader_input_reader.printFaderValues(input_report_buffer);
				//float fader_value_1 = fader_input_reader.getFaderValue(input_report_buffer, 1);
				//float fader_value_2 = fader_input_reader.getFaderValue(input_report_buffer, 2);
				//float fader_value_3 = fader_input_reader.getFaderValue(input_report_buffer, 3);
				//float fader_value_4 = fader_input_reader.getFaderValue(input_report_buffer, 4);

				// Print knob and fader values for debugging
				//std::cout << "Knob Values: "
				//				<< "1: " << std::fixed << std::setprecision(3) << knob_value_1 << " | "
				//				<< "2: " << std::fixed << std::setprecision(3) << knob_value_2 << " | "
				//				<< "3: " << std::fixed << std::setprecision(3) << knob_value_3 << " | "
				//				<< "4: " << std::fixed << std::setprecision(3) << knob_value_4 << " || "
				//				<< "Fader Values: "
				//				<< "1: " << std::fixed << std::setprecision(3) << fader_value_1 << " | "
				//				<< "2: " << std::fixed << std::setprecision(3) << fader_value_2 << " | "
				//				<< "3: " << std::fixed << std::setprecision(3) << fader_value_3 << " | "
				//				<< "4: " << std::fixed << std::setprecision(3) << fader_value_4 << " || "
				//				<< "        \r"; // Carriage return to overwrite the line
				//std::cout.flush();

				// =======================================
				// Check for button toggles
				// =======================================

				// Special button toggles (only triggers on press, not hold)
				//btn_toggle_system.shouldToggleSpecialButton(input_report_buffer, SpecialButton::SHIFT, SpecialLEDButton::SHIFT);
				//btn_toggle_system.shouldToggleSpecialButton(input_report_buffer, SpecialButton::REVERSE, SpecialLEDButton::REVERSE);
				//btn_toggle_system.shouldToggleSpecialButton(input_report_buffer, SpecialButton::TYPE, SpecialLEDButton::TYPE);
				//btn_toggle_system.shouldToggleSpecialButton(input_report_buffer, SpecialButton::SIZE, SpecialLEDButton::SIZE);
				//btn_toggle_system.shouldToggleSpecialButton(input_report_buffer, SpecialButton::BROWSE, SpecialLEDButton::BROWSE);

				// Control button toggles
				//btn_toggle_system.shouldToggleControlButton(input_report_buffer, ControlButton::CAPTURE, ControlLEDButton::CAPTURE);
				//btn_toggle_system.shouldToggleControlButton(input_report_buffer, ControlButton::QUANT, ControlLEDButton::QUANT);
				//btn_toggle_system.shouldToggleControlButton(input_report_buffer, ControlButton::SYNC, ControlLEDButton::SYNC);

				// Stop button toggles
				//btn_toggle_system.shouldToggleStopButton(input_report_buffer, StopButton::STOP1, StopLEDButton::STOP1);
				//btn_toggle_system.shouldToggleStopButton(input_report_buffer, StopButton::STOP2, StopLEDButton::STOP2);
				//btn_toggle_system.shouldToggleStopButton(input_report_buffer, StopButton::STOP3, StopLEDButton::STOP3);
				//btn_toggle_system.shouldToggleStopButton(input_report_buffer, StopButton::STOP4, StopLEDButton::STOP4);

				// Matrix button toggles
				//for (int row = 1; row <= 4; row++) {
				//	for (int col = 1; col <= 4; col++) {
				//		btn_toggle_system.shouldToggleMatrixButton(input_report_buffer, row, col);
				//	}
				//}

				// Update button states for next frame
				//btn_toggle_system.updateButtonStates(input_report_buffer);

		}

	// =============================================================================
	// CODE CLOSES
	//=============================================================================
	// Close the device
	hid_close(device);

	// Finalize the hidapi library
	res = hid_exit();

	return 0;
}