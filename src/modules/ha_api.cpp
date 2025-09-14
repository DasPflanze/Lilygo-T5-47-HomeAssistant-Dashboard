#include "ha_api.h"
#include "../configurations.h"
#include "homeassistantapi.h"
#include <WiFi.h>
#include <HTTPClient.h>

// State tracking for selective updates
String lastEntityStates[12];
bool entityStatesInitialized = false;

// Forward declaration
void updateSingleTile(int tileIndex);

void toggleHomeAssistantEntity(String entityID, int entityType) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No WiFi connection for HA API call");
        return;
    }

    HTTPClient http;
    String service = "";

    switch (entityType) {
        case SWITCH:
        case PLUG:
            service = "switch/toggle";
            break;
        case LIGHT:
            service = "light/toggle";
            break;
        case FAN:
        case EXFAN:
        case AIRPURIFIER:
            service = "fan/toggle";
            break;
        case WATERHEATER:
        case AIRCONDITIONER:
            service = "switch/toggle";
            break;
        default:
            Serial.println("Entity type not toggleable: " + String(entityType));
            return;
    }

    http.begin(ha_server + "/api/services/" + service);
    http.addHeader("Authorization", "Bearer " + ha_token);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"entity_id\":\"" + entityID + "\"}";
    int httpCode = http.POST(payload);

    Serial.println("HA API call: " + service + " -> " + entityID + " (Response: " + String(httpCode) + ")");

    if (httpCode == 200 || httpCode == 201) {
        Serial.println("Entity toggled successfully");
    } else {
        Serial.println("API call failed with code: " + String(httpCode));
    }

    http.end();
}

void checkAndUpdateChangedTiles() {
    bool anyChanges = false;

    for (int i = 0; i < 12; i++) {
        if (haEntities[i].entityName == "") continue;

        String currentState;
        if (haEntities[i].entityType == HIGROW) {
            currentState = getSensorValue(haEntities[i].entityID+"_soil") + "|" +
                          getSensorValue(haEntities[i].entityID+"_battery");
        } else {
            currentState = String(checkOnOffState(haEntities[i].entityID));
        }

        if (!entityStatesInitialized || lastEntityStates[i] != currentState) {
            Serial.println("State change detected for " + haEntities[i].entityName + ": " +
                          lastEntityStates[i] + " -> " + currentState);
            lastEntityStates[i] = currentState;
            updateSingleTile(i);
            anyChanges = true;
        }
    }

    if (!anyChanges && entityStatesInitialized) {
        Serial.println("No state changes detected");
    }

    entityStatesInitialized = true;
}