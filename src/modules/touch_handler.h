#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include "../common/definitions.h"
#include <touch.h>

// Touch system variables
extern TouchClass touch;
extern TouchZone touchZones[12];
extern bool touchEnabled;
extern bool touchActive;

// Touch system functions
bool initializeTouchScreen();
void setupTouchZones();
void handleTouch();
bool isPointInZone(int px, int py, TouchZone zone);
void scanI2C();

#endif // TOUCH_HANDLER_H