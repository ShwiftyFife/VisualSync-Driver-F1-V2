#ifndef STARTUP_SEQUENCE_H
#define STARTUP_SEQUENCE_H

#include <hidapi/hidapi.h>
#include <stdint.h>

void startupSequence(hid_device *handle);

#endif