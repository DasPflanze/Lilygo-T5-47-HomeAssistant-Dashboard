#include "ha_client.h"
#include "../configurations.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Static HTTP client instances
static HTTPClient http;
static WiFiClientSecure client;

void initHAClient() {
    // Initialize HTTP client if needed
    Serial.println("HA Client initialized");
}

int checkOnOffState(String entity) {
    String api_url = ha_server + "/api/states/" + entity;
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.println("Error '" + String(code) + "' connecting to HA API: " + api_url);
        return entity_state::ERROR;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return entity_state::ERROR;
    }
    String state = doc["state"];
    Serial.println("  - " + entity + " state: " + state);
    if (state == "on") {
        return entity_state::ON;
    }
    if (state == "unavailable") {
        return entity_state::UNAVAILABLE;
    }
    return entity_state::OFF;
}

HAConfigurations getHaStatus() {
    HAConfigurations haConfigs;
    haConfigs.haStatus = "ERROR";
    haConfigs.timeZone = "ERROR";
    haConfigs.version = "ERROR";

    String api_url = ha_server + "/api/config";
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        http.end();
        Serial.println("Error '" + String(code) + "' connecting to HA API: " + api_url);
        return haConfigs;
    }
    DynamicJsonDocument doc(4096);
    StaticJsonDocument<64> filter;
    // Filter JSON data to save RAM
    filter["time_zone"] = true;
    filter["version"] = true;
    filter["state"] = true;
    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return haConfigs;
    }
    String state = doc["state"];
    String timeZone = doc["time_zone"];
    String version = doc["version"];
    haConfigs.haStatus = state;
    haConfigs.timeZone = timeZone;
    haConfigs.version = version;
    Serial.println("Home Assistant config: " + state + " - " + timeZone + " - " + version);
    return haConfigs;
}

String getSensorValue(String entity) {
    String api_url = ha_server + "/api/states/" + entity;
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.println("Error '" + String(code) + "' connecting to HA API for: " + api_url);
        return "";
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "";
    }
    String state = doc["state"];
    Serial.println("  - " + entity + " state: " + state);
    return state;
}

String getSensorAttributeValue(String entity, String attribute) {
    String api_url = ha_server + "/api/states/" + entity;
    http.begin(api_url);
    http.addHeader("Authorization", "Bearer " + ha_token);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.println("Error '" + String(code) + "' connecting to HA API for: " + api_url);
        return "";
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "";
    }
    // read attribute
    String attr = doc["attributes"][attribute];
    if (attr != NULL && attr != "" && attr != "null") {
        Serial.println("  - " + entity + ".Attributes[" + attribute + "]: " + attr);
        return attr;
    }
    // try entity properties
    String coreAttr = doc[attribute];
    if (coreAttr != NULL && coreAttr != "" && coreAttr != "null") {
        Serial.println("  - " + entity + "[" + attribute + "]: " + coreAttr);
        return coreAttr;
    }

    Serial.println("  - " + entity + ".Attributes[" + attribute + "]: " + attr);
    return "";
}

float getSensorFloatValue(String entity) {
    String state = getSensorValue(entity);
    return state.toFloat();
}