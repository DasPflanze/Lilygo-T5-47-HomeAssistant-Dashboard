#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Arduino.h>
#include "epd_driver.h"

// Display constants
#define White 0xFF
#define LightGrey 0xBB
#define Grey 0x88
#define DarkGrey 0x44
#define Black 0x00

// Hardware pins
#define BATT_PIN 36

// Tile dimensions
#define TILE_IMG_WIDTH  100
#define TILE_IMG_HEIGHT 100
#define TILE_WIDTH      160
#define TILE_HEIGHT     160
#define TILE_GAP        6
#define SENSOR_TILE_WIDTH      120
#define SENSOR_TILE_HEIGHT     110
#define SENSOR_TILE_IMG_WIDTH  64
#define SENSOR_TILE_IMG_HEIGHT 64
#define BOTTOM_TILE_WIDTH      240
#define BOTTOM_TILE_HEIGHT     90

// Plant watering limit
#define WATERING_SOIL_LIMIT 75

// Touch settings
#define TOUCH_DEBOUNCE_MS 1000

// Sleep configurations
#define DEFAULT_SLEEP_DURATION 60
#define DEFAULT_WAKEUP_HOUR 6
#define DEFAULT_SLEEP_HOUR 23

// Entity states and types (from configurations.h)
enum entity_state {ON, OFF, ERROR, UNAVAILABLE};
enum entity_type {SWITCH, LIGHT, EXFAN, FAN, AIRPURIFIER, WATERHEATER, PLUG, AIRCONDITIONER, PLANT, HIGROW};
enum entity_state_type {ONOFF, VALUE};
enum sensor_type {DOOR, WINDOW, MOTION, ENERGYMETER, TEMP, ENERGYMETERPWR};

// Structures
struct HAEntities {
    String entityName;
    String entityID;
    int entityType;
    int entityStateType;
};

struct HAConfigurations {
    String timeZone;
    String version;
    String haStatus;
};

struct TouchZone {
    int x, y, width, height;
    String entityID;
    int entityType;
    int entityIndex;
};

// Global strings
extern String str_unavail;

// External variables that modules need access to
extern uint8_t *framebuffer;
extern int vref;
extern int wifi_signal;

#endif // DEFINITIONS_H