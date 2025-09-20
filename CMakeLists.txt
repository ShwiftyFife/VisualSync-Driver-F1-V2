cmake_minimum_required(VERSION 3.10)
project(traktor CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add some compiler flags for better debugging and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


# Find HIDAPI - more robust approach for macOS
if(APPLE)
    # Try to find via Homebrew paths
    find_path(HIDAPI_INCLUDE_DIR 
        NAMES hidapi/hidapi.h
        PATHS 
            /opt/homebrew/include
            /usr/local/include
        NO_DEFAULT_PATH
    )
    
    find_library(HIDAPI_LIBRARY 
        NAMES hidapi
        PATHS 
            /opt/homebrew/lib
            /usr/local/lib
        NO_DEFAULT_PATH
    )
else()
    # Linux approach
    find_path(HIDAPI_INCLUDE_DIR hidapi/hidapi.h)
    find_library(HIDAPI_LIBRARY NAMES hidapi-libusb hidapi)
endif()

# Check if we found everything
if(NOT HIDAPI_INCLUDE_DIR OR NOT HIDAPI_LIBRARY)
    message(FATAL_ERROR "HIDAPI not found. On macOS: brew install hidapi")
endif()

# Print what we found (helpful for debugging)
message(STATUS "HIDAPI include dir: ${HIDAPI_INCLUDE_DIR}")
message(STATUS "HIDAPI library: ${HIDAPI_LIBRARY}")

# Add executable
add_executable(f1_driver 
    main.cpp
    src/input_reader_base.cpp
    src/input_reader_fader.cpp
    src/input_reader_knob.cpp
    src/input_reader_wheel.cpp
    src/led_controller_base.cpp
    src/led_controller_display.cpp
    src/startup_sequence.cpp
)

# Include directories
target_include_directories(f1_driver PRIVATE 
    ${HIDAPI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link the HIDAPI library
target_link_libraries(f1_driver PRIVATE ${HIDAPI_LIBRARY})