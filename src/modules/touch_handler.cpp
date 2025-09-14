#include "touch_handler.h"
#include "../configurations.h"
#include "ha_api.h"
#include "display_ui.h"
#include <Wire.h>

// Touch system variables
TouchClass touch;
TouchZone touchZones[12];
bool touchEnabled = false;
bool touchActive = false;

void scanI2C() {
    Serial.println("Scanning I2C bus...");

    int deviceCount = 0;
    for (byte i = 8; i < 120; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0) {
            Serial.println("I2C device found at 0x" + String(i, HEX));
            deviceCount++;
        }
    }

    if (deviceCount == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.println("Found " + String(deviceCount) + " I2C devices");
    }
}

bool initializeTouchScreen() {
    Serial.println("Initializing touch screen...");

    // Initialize I2C with correct pins for LilyGo T5 4.7"
    // INT=IO13 (Pin 16), SCL=IO14 (Pin 13), SDA=IO15 (Pin 23)
    Wire.begin(15, 14); // SDA=IO15, SCL=IO14
    delay(100);
    Serial.println("I2C initialized: SDA=15, SCL=14");

    scanI2C();

    if (touch.begin()) {
        touchEnabled = true;
        Serial.println("Touch screen initialized successfully");
        setupTouchZones();
        return true;
    } else {
        Serial.println("Touch screen initialization failed - trying different addresses");

        // Try different I2C addresses (0x5A is default, try 0x5D, 0x14, 0x38)
        if (touch.begin(Wire, 0x5D)) {
            touchEnabled = true;
            Serial.println("Touch screen initialized at address 0x5D");
            setupTouchZones();
            return true;
        } else if (touch.begin(Wire, 0x14)) {
            touchEnabled = true;
            Serial.println("Touch screen initialized at address 0x14");
            setupTouchZones();
            return true;
        } else if (touch.begin(Wire, 0x38)) {
            touchEnabled = true;
            Serial.println("Touch screen initialized at address 0x38");
            setupTouchZones();
            return true;
        } else {
            touchEnabled = false;
            Serial.println("Touch screen initialization failed completely - running without touch");
            return false;
        }
    }
}

void setupTouchZones() {
    int x = 3;
    int y = 23;
    int index = 0;

    for (int i = 0; i < haEntitiesCount; i++) {
        if (haEntities[i].entityName != "" && index < 12) {
            touchZones[index] = {
                x, y,
                TILE_WIDTH - TILE_GAP,
                TILE_HEIGHT - TILE_GAP,
                haEntities[i].entityID,
                haEntities[i].entityType,
                i
            };
            index++;
        }

        x += TILE_WIDTH;
        if (i == 5) {
            x = 3;
            y += TILE_HEIGHT;
        }
    }
    Serial.println("Touch zones configured for " + String(index) + " tiles");
}

bool isPointInZone(int px, int py, TouchZone zone) {
    return (px >= zone.x && px <= zone.x + zone.width &&
            py >= zone.y && py <= zone.y + zone.height);
}

void handleTouch() {
    if (!touchEnabled) {
        return;
    }

    if (touch.scanPoint()) {
        // Nur verarbeiten wenn Touch vorher NICHT aktiv war (neuer Touch-Start)
        if (touchActive) {
            // Touch ist noch vom letzten Mal aktiv, ignorieren
            return;
        }

        touchActive = true;  // Touch ist jetzt aktiv

        uint16_t x, y;
        touch.getPoint(x, y, 0);

        // Touch-Koordinaten korrigieren (vertikal spiegeln)
        y = EPD_HEIGHT - y;

        Serial.println("NEW Touch detected at: " + String(x) + ", " + String(y) + " (corrected)");

        Serial.println("Touch detected - refreshing screen");

        for (int i = 0; i < 12; i++) {
            if (touchZones[i].entityID != "" && isPointInZone(x, y, touchZones[i])) {
                Serial.println("Touch zone hit: " + touchZones[i].entityID);

                showTouchFeedback(touchZones[i].x, touchZones[i].y);
                toggleHomeAssistantEntity(touchZones[i].entityID, touchZones[i].entityType);

                // Display nach Toggle aktualisieren
                Serial.println("Updating display after toggle");
                delay(500);
                //DrawHAScreen();
                break;
            }
        }
    } else {
        // Kein Touch mehr erkannt - Touch als inaktiv markieren
        if (touchActive) {
            touchActive = false;
            Serial.println("Touch released");
        }
    }

    delay(200); // Debounce
}