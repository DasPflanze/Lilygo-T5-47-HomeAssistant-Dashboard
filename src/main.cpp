#include <Arduino.h>

// esp32 sdk imports
#include "esp_heap_caps.h"
#include "esp_log.h"

// epd
#include "epd_driver.h"

// battery
#include <driver/adc.h>
#include "esp_adc_cal.h"

// deepsleep
#include "esp_sleep.h"

// font
#include "opensans8b.h"
#include "opensans9b.h"
#include "opensans10b.h"
#include "opensans12b.h"
#include "opensans18b.h"
#include "opensans24b.h"

// wifi
#include <WiFi.h>

#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "touch.h"

#include "configurations.h"
#include "modules/ha_client.h"
#include "modules/ha_api.h"
#include "modules/display_ui.h"
#include "modules/touch_handler.h"
#include "epd_drawing.h"

// Icons for Home Assistant
#include "icons/waterheateron.h"
#include "icons/waterheateroff.h"
#include "icons/lightbulbon.h"
#include "icons/lightbulboff.h"
#include "icons/exhaustfanon.h"
#include "icons/exhaustfanoff.h"
#include "icons/fanoff.h"
#include "icons/fanon.h"
#include "icons/airpurifieron.h"
#include "icons/airpurifieroff.h"
#include "icons/plugon.h"
#include "icons/plugoff.h"
#include "icons/switchon.h"
#include "icons/switchoff.h"
#include "icons/airconditioneron.h"
#include "icons/airconditioneroff.h"
#include "icons/warning.h"
#include "icons/plantwateringlow.h"
#include "icons/plantwateringok.h"
#include "icons/batteryempty.h"

// sensor icons
#include "icons/dooropen.h"
#include "icons/doorclosed.h"
#include "icons/windowopen.h"
#include "icons/windowclosed.h"
#include "icons/motionsensoron.h"
#include "icons/motionsensoroff.h"
#include "icons/sensorerror.h"

#define White 0xFF
#define LightGrey 0xBB
#define Grey 0x88
#define DarkGrey 0x44
#define Black 0x00

#define BATT_PIN            36

// Touch screen pins are handled by the LilyGo EPD47 library internally

#define TILE_IMG_WIDTH  100
#define TILE_IMG_HEIGHT 100
#define TILE_WIDTH      160
#define TILE_HEIGHT     160
#define TILE_GAP        6
#define SENSOR_TILE_WIDTH      120
#define SENSOR_TILE_HEIGHT     110
#define SENSOR_TILE_IMG_WIDTH  64
#define SENSOR_TILE_IMG_HEIGHT 64
#define BOTTOM_TILE_WIDTH      240 // 4x = 240, 3x = 320
#define BOTTOM_TILE_HEIGHT     90

// if below, plant icon is changed to watering can icon
#define WATERING_SOIL_LIMIT 75

// deep sleep configurations
long SleepDuration   = 1; // Sleep time in minutes, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
int  WakeupHour      = 6;  // Wakeup after 06:00 to save battery power
int  SleepHour       = 23; // Sleep  after 23:00 to save battery power

long StartTime       = 0;
long SleepTimer      = 0;
int  CurrentHour = 0, CurrentMin = 0, CurrentSec = 0, CurrentDay = 0;

int vref = 1100; // default battery vref
int wifi_signal = 0;

// Touch and state tracking variables are now in modules

// required for NTP time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dateStamp;
String timeStamp;

// defualt strings
String str_unavail = "unavail.";

// Forward declarations
void DrawHAScreen();

uint8_t StartWiFi() {
  IPAddress dns(8, 8, 8, 8); // Use Google DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  unsigned long start = millis();
  bool AttemptConnection = true;
  while (AttemptConnection) {
    Serial.println("\r\nConnecting to: " + String(ssid));
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("STA: Failed to connect to: " + String(ssid));
      WiFi.disconnect(true); // delete SID/PWD
      delay(500);

      if (ssid2 != "") {
        Serial.println("\r\nConnecting to: " + String(ssid2));
        WiFi.begin(ssid2, password2);
        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
          Serial.println("STA: Failed to connect to: " + String(ssid2));
          WiFi.disconnect(true); // delete SID/PWD
          delay(500);
        }
        else {
          AttemptConnection = false;
        }
      }
      if (millis() > start + 15000) { // Wait 15-secs maximum
        AttemptConnection = false;
      }
    }
    else {
      AttemptConnection = false;
    }
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
    wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  }
  else 
  {
    wifi_signal = 0;
    Serial.println("WiFi connection *** FAILED ***");
  }
  return WiFi.status();
}


void DrawTileHigrow(int x, int y, int width, int height, const uint8_t *image_data, String label, String soil, String temp, String batt)
{
  drawRect(x, y, width, height, Black);
  drawRect(x + 1, y + 1, width - 2, height - 2, Black);

    // this assumes images are 100x100px size. make sure images are cropped to 100x100 before converting
  int image_x = int((width - TILE_IMG_WIDTH)/2) + x; 
  int image_y = int((height - TILE_IMG_HEIGHT)/2) + y;
  drawImage(image_x, image_y, TILE_IMG_WIDTH, TILE_IMG_HEIGHT, image_data);

  int label_txt_cursor_x = int(width / 2) + x;
  int label_txt_cursor_y = y + 21;
  drawString(label_txt_cursor_x, label_txt_cursor_y, label, CENTER);
  
  if (batt != "-1")
  {
    int state_txt_cursor_x = width / 2 + x - 1;
    int state_txt_cursor_y = image_y + TILE_IMG_HEIGHT - 21;
    if (soil.toInt() < WATERING_SOIL_LIMIT)
      state_txt_cursor_x += 15;
    drawString(state_txt_cursor_x, state_txt_cursor_y, soil + "%", CENTER);

    state_txt_cursor_x = x + 5;
    state_txt_cursor_y = image_y + TILE_IMG_HEIGHT + 22;
    drawString(state_txt_cursor_x, state_txt_cursor_y, temp + "° C", LEFT);
  
    state_txt_cursor_x = x + width - 105;
    DrawBattery(state_txt_cursor_x, state_txt_cursor_y, batt.toInt());
    state_txt_cursor_x = x + width - 5;
    state_txt_cursor_y -= 20;
    GFXfont lastFont = currentFont;
    setFont(OpenSans8B);
    drawString(state_txt_cursor_x, state_txt_cursor_y, batt + "%", RIGHT);  
    setFont(lastFont);
  }
}

// this will place a tile on screen that includes icon, staus and name of the HA entity, temperature and battery level
void DrawTile(int x, int y, int width, int height, const uint8_t *image_data, String label, String state)
{
  drawRect(x, y, width, height, Black);
  drawRect(x + 1, y + 1, width - 2, height - 2, Black);

  // this assumes images are 100x100px size. make sure images are cropped to 100x100 before converting
  int image_x = int((width - TILE_IMG_WIDTH)/2) + x; 
  int image_y = int((height - TILE_IMG_HEIGHT)/2) + y;
  drawImage(image_x, image_y, TILE_IMG_WIDTH, TILE_IMG_HEIGHT, image_data);

  int label_txt_cursor_x = int(width / 2) + x;
  int label_txt_cursor_y = y + 21;
  drawString(label_txt_cursor_x, label_txt_cursor_y, label, CENTER);

  int state_txt_cursor_x = width / 2 + x;
  int state_txt_cursor_y = image_y + TILE_IMG_HEIGHT + 10 + 12;
  drawString(state_txt_cursor_x, state_txt_cursor_y, state, CENTER);
}

void DrawTempSensorTile(int x, int y, float temp, String label)
{
  int tile_width = SENSOR_TILE_WIDTH - TILE_GAP;
  int tile_height = SENSOR_TILE_HEIGHT - TILE_GAP;
  drawRect(x, y, tile_width, tile_height, Black);
  drawRect(x + 1, y + 1, tile_width - 2, tile_height - 2, Black);

  int temp_x = int(tile_width / 2) + x;
  int temp_y = int(tile_height / 2) + y + 10;
  if (temp != 0)
  {
    setFont(OpenSans18B);
    drawString(temp_x, temp_y, String(temp, 1) + "°", CENTER);
  }
  else
    drawString(temp_x, temp_y, str_unavail, CENTER);

  int txt_cursor_x = int(tile_width / 2) + x;
  int txt_cursor_y = y + 10 + SENSOR_TILE_IMG_HEIGHT + 10 + 12;
  setFont(OpenSans9B);
  drawString(txt_cursor_x, txt_cursor_y, label, CENTER);
}

// this will place a tile on screen that includes icon, staus and name of the HA entity
void DrawSensorTile(int x, int y, int width, int height, const uint8_t* image_data, String label)
{
  drawRect(x, y, width, height, Black);
  drawRect(x+1, y+1, width-2, height-2, Black);

  // this assumes images are 128x128px size. make sure images are cropped to 128x128 before converting
  int image_x = int((width - SENSOR_TILE_IMG_WIDTH)/2) + x; 
  int image_y = y + 10;  
  drawImage(image_x, image_y, SENSOR_TILE_IMG_WIDTH, SENSOR_TILE_IMG_HEIGHT, image_data);

  int txt_cursor_x = int(width/2) + x;
  int txt_cursor_y = image_y + SENSOR_TILE_IMG_HEIGHT + 10 + 12;
  drawString(txt_cursor_x, txt_cursor_y, label, CENTER);
}

void DrawTile(int x, int y, int state, int type, String name, String value)
{
    int tile_width = TILE_WIDTH - TILE_GAP; 
    int tile_height = TILE_HEIGHT - TILE_GAP;

    String state_txt = "OFF";
    if (state == entity_state::ON) state_txt = "ON"; 
    else if (state == entity_state::UNAVAILABLE) state_txt = "UNAVAILABLE"; 
    switch (type)
    {
      case entity_type::SWITCH:
        if (state == entity_state::ON) DrawTile(x,y,tile_width,tile_height,switchon_data, name, state_txt); 
        else if (state == entity_state::OFF) DrawTile(x,y,tile_width,tile_height,switchoff_data, name, state_txt); 
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "SWITCH"); 
        break;
      case entity_type::LIGHT:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,lightbulbon_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,lightbulboff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "LIGHT"); 
        break;
      case entity_type::FAN:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,fanon_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,fanoff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "FAN"); 
        break;
    case entity_type::EXFAN:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,exhaustfanon_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,exhaustfanoff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "EXHAUST FAN"); 
        break;
      case entity_type::AIRPURIFIER:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,airpurifieron_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,airpurifieroff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "AIR PURIFIER"); 
        break;
      case entity_type::WATERHEATER:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,waterheateron_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,waterheateroff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "WATER HEATER"); 
        break;
      case entity_type::PLUG:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,plugon_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,plugoff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "PLUG"); 
        break;
      case entity_type::AIRCONDITIONER:
        if (state == entity_state::ON)  DrawTile(x,y,tile_width,tile_height,airconditioneron_data,name, state_txt);
        else if (state == entity_state::OFF)  DrawTile(x,y,tile_width,tile_height,airconditioneroff_data,name, state_txt);
        else DrawTile(x,y,tile_width,tile_height,warning_data, name, "AIR CONDITIONER"); 
        break;
      case entity_type::PLANT:
        if (value.toInt() >= WATERING_SOIL_LIMIT) DrawTile(x, y, tile_width, tile_height, plantwateringok_data, name, value + "%");
        else if (value.toInt() > 5) DrawTile(x, y, tile_width, tile_height, plantwateringlow_data, name, value + "%");
        else DrawTile(x, y, tile_width, tile_height, warning_data, name, value + "%");
        break;
      default:
        break;
    }
}

void DrawTileHigrow(int x, int y, int state, int type, String name, String soil, String temp, String batt)
{
    int tile_width = TILE_WIDTH - TILE_GAP; 
    int tile_height = TILE_HEIGHT - TILE_GAP;

    if (batt.toInt() == -1) DrawTileHigrow(x, y, tile_width, tile_height, batteryempty_data, name, soil, temp, batt);
    else if (soil.toInt() >= WATERING_SOIL_LIMIT) DrawTileHigrow(x, y, tile_width, tile_height, plantwateringok_data, name, soil, temp, batt);
    else if (soil.toInt() > 5) DrawTileHigrow(x, y, tile_width, tile_height, plantwateringlow_data, name, soil, temp, batt);
    else DrawTileHigrow(x, y, tile_width, tile_height, warning_data, name, soil, temp, batt);
}

void DrawSensorTile(int x, int y, int state, int type, String name)
{
    int tile_width = SENSOR_TILE_WIDTH - TILE_GAP; 
    int tile_height = SENSOR_TILE_HEIGHT - TILE_GAP;
    switch (type)
    {
      case sensor_type::DOOR:
        if (state == entity_state::ON) DrawSensorTile(x,y,tile_width,tile_height,dooropen_data, name); 
        else if (state == entity_state::OFF) DrawSensorTile(x,y,tile_width,tile_height,doorclosed_data, name); 
        else DrawSensorTile(x,y,tile_width,tile_height,sensorerror_data, name); 
        break;
      case sensor_type::WINDOW:
        if (state == entity_state::ON) DrawSensorTile(x,y,tile_width,tile_height,windowopen_data, name); 
        else if (state == entity_state::OFF) DrawSensorTile(x,y,tile_width,tile_height,windowclosed_data, name); 
        else DrawSensorTile(x,y,tile_width,tile_height,sensorerror_data, name); 
        break;
      case sensor_type::MOTION:
        if (state == entity_state::ON) DrawSensorTile(x,y,tile_width,tile_height,motionsensoron_data, name); 
        else if (state == entity_state::OFF) DrawSensorTile(x,y,tile_width,tile_height,motionsensoroff_data, name); 
        else DrawSensorTile(x,y,tile_width,tile_height,sensorerror_data, name); 
        break;
      default:
        break;
    }
}

void DrawBottomTile(int x, int y, String value, String name)
{
    int tile_width = BOTTOM_TILE_WIDTH - TILE_GAP; 
    int tile_height = BOTTOM_TILE_HEIGHT - TILE_GAP;
    drawRect(x, y, tile_width, tile_height, Black);
    drawRect(x+1, y+1, tile_width-2, tile_height-2, Black);
    setFont(OpenSans24B);
    drawString(int(tile_width/2) + x, 508, value, CENTER);
    setFont(OpenSans9B);
    drawString(int(tile_width/2) + x, 532, name, CENTER);
}

void DrawBottomBar()
{
    int tiles = haFloatSensorsCount;
    float totalEnergy = 0;
    float totalPower  = 0;
    String totalEnergyName;
    String totaPowerName;
    for (int i = 0; i < haFloatSensorsCount; i++){
        if (haFloatSensors[i].entityType == sensor_type::ENERGYMETER)
        {
            totalEnergy = totalEnergy + getSensorFloatValue(haFloatSensors[i].entityID);
            totalEnergyName = haFloatSensors[i].entityName;
        }
        else if (haFloatSensors[i].entityType == sensor_type::ENERGYMETERPWR)
        {
            totalPower = totalPower + getSensorFloatValue(haFloatSensors[i].entityID);
            totaPowerName = haFloatSensors[i].entityName;
        }
    }
    int x = 3;
    int y = 456;
    // first one 
    if (totalEnergy != 0)
    {
        DrawBottomTile(x, y, String(totalEnergy) + " kWh", totalEnergyName);
        x = x + BOTTOM_TILE_WIDTH;
        tiles--;
    }
    if (totalPower != 0)
    {
        DrawBottomTile(x, y, String((int)totalPower) + " W", totaPowerName);
        x = x + BOTTOM_TILE_WIDTH;
        tiles--;
    }
    
    for (int i = 0; i < haFloatSensorsCount; i++){
        if (haFloatSensors[i].entityType == sensor_type::TEMP && tiles >= 1)
        {
            float temp = getSensorAttributeValue(haFloatSensors[i].entityID, "current_temperature").toFloat();
            if (temp == 0)
                temp = getSensorFloatValue(haFloatSensors[i].entityID);
            if (temp != 0)
              DrawBottomTile(x, y, String(temp, 1) + "° C", haFloatSensors[i].entityName);
            else
              DrawBottomTile(x, y, str_unavail, haFloatSensors[i].entityName);
            x = x + BOTTOM_TILE_WIDTH;
            tiles--;
        }
    }
}

void DrawSwitchBar()
{
    setFont(OpenSans9B);
    int x = 3;
    int y = 23;
    for (int i = 0; i < haEntitiesCount; i++){
        if (haEntities[i].entityName != "") {
          if (haEntities[i].entityType == entity_type::SWITCH ||
              haEntities[i].entityType == entity_type::LIGHT ||
              haEntities[i].entityType == entity_type::PLUG ||
              haEntities[i].entityType == entity_type::EXFAN ||
              haEntities[i].entityType == entity_type::FAN ||
              haEntities[i].entityType == entity_type::AIRPURIFIER ||
              haEntities[i].entityType == entity_type::WATERHEATER ||
              haEntities[i].entityType == entity_type::AIRCONDITIONER)
          {            
              DrawTile(x, y, checkOnOffState(haEntities[i].entityID), haEntities[i].entityType, haEntities[i].entityName, "");
          }
          else if (haEntities[i].entityType == entity_type::HIGROW)
          {
              String soilVal = getSensorValue(haEntities[i].entityID+"_soil");
              String tempVal = String(getSensorFloatValue(haEntities[i].entityID+"_temperature"), 1);
              String battVal = getSensorValue(haEntities[i].entityID+"_battery");

              String lastUpdate = getSensorValue(haEntities[i].entityID+"_updated");
              int splitT = lastUpdate.indexOf("T");
              String lastUpdateDate = lastUpdate.substring(0, splitT);
              String lastUpdateDayStr = lastUpdateDate.substring(8,10);
              int lastUpdateDay = lastUpdateDayStr.toInt();
              if (lastUpdateDay != CurrentDay && lastUpdateDay != CurrentDay-1) // todo: what about end of month? Let's ignore that for now
              {
                Serial.println("Batt of " + haEntities[i].entityID + " last value " + battVal + ", last update on day " + lastUpdateDay + " != today (" + CurrentDay + ") or yesterday (" + (CurrentDay - 1) + ") - battery might be empty");
                DrawTileHigrow(x, y, 0, haEntities[i].entityType, haEntities[i].entityName, soilVal, tempVal, "-1"); // presume battery empty
              }
              else
              {
                DrawTileHigrow(x, y, 0, haEntities[i].entityType, haEntities[i].entityName, soilVal, tempVal, battVal);
              }
          }
          else 
          {
              String val = getSensorValue(haEntities[i].entityID);
              DrawTile(x, y, 0, haEntities[i].entityType, haEntities[i].entityName, val);
          }
        }
      
        x = x + TILE_WIDTH; // move column right
        if (i == 5) { // move row down
            x = 3;
            y = y + TILE_HEIGHT;
        }
    }
}

void DrawSensorBar()
{
    setFont(OpenSans9B);
    int x = 3;
    int y = 345;
    for (int i = 0; i < haSensorsCount; i++){
        if ((haSensors[i].entityType == sensor_type::DOOR || 
            haSensors[i].entityType == sensor_type::WINDOW ||
            haSensors[i].entityType == sensor_type::MOTION ) && haSensors[i].entityName != "")
        {
            DrawSensorTile(x,y,checkOnOffState(haSensors[i].entityID),haSensors[i].entityType, haSensors[i].entityName);
        }
        if (haSensors[i].entityType == sensor_type::TEMP && haSensors[i].entityName != "")
        {
            float temp = getSensorAttributeValue(haSensors[i].entityID, "current_temperature").toFloat();
            if (temp == 0)
                temp = getSensorFloatValue(haSensors[i].entityID);
            DrawTempSensorTile(x, y, temp, haSensors[i].entityName);
        }
        x = x + SENSOR_TILE_WIDTH;
    }
}



void SetupTime()
{
    Serial.println("Getting time...");

    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dateStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    CurrentDay = dateStamp.substring(8,10).toInt();

    CurrentHour = timeClient.getHours();
    CurrentMin  = timeClient.getMinutes();
    CurrentSec  = timeClient.getSeconds();

    Serial.println("Current day: " + String(CurrentDay) + " hour: " + String(CurrentHour) + " min: " + String(CurrentMin) + " sec: " + String(CurrentSec));
}

void DisplayGeneralInfoSection()
{
    setFont(OpenSans8B);
    Serial.println("Getting haStatus...");
    HAConfigurations haConfigs = getHaStatus();
    Serial.println("drawing status line...");    
    drawString(EPD_WIDTH/2, 18, dateStamp + " - " +  timeStamp, CENTER);
    //drawString(EPD_WIDTH/2, 18, dateStamp + " - " +  timeStamp + " (HA Ver:" + haConfigs.version + "/" + haConfigs.haStatus + ", TZ:" + haConfigs.timeZone + ")", CENTER);
}

void DisplayStatusSection() {
  setFont(OpenSans8B);
  DrawBattery(5, 18);
  DrawRSSI(900, 18, wifi_signal);
}

void DrawWifiErrorScreen()
{
    epd_clear();
    DisplayStatusSection();
    epd_update();
}







void updateSingleTile(int tileIndex) {
    if (tileIndex < 0 || tileIndex >= haEntitiesCount) return;

    int x = 3 + (tileIndex % 6) * TILE_WIDTH;
    int y = 23 + (tileIndex / 6) * TILE_HEIGHT;

    // Clear tile area
    int tile_width = TILE_WIDTH - TILE_GAP;
    int tile_height = TILE_HEIGHT - TILE_GAP;
    fillRect(x, y, tile_width, tile_height, White);

    // Redraw tile
    if (haEntities[tileIndex].entityType == HIGROW) {
        String soilVal = getSensorValue(haEntities[tileIndex].entityID+"_soil");
        String tempVal = String(getSensorFloatValue(haEntities[tileIndex].entityID+"_temperature"), 1);
        String battVal = getSensorValue(haEntities[tileIndex].entityID+"_battery");
        DrawTileHigrow(x, y, 0, haEntities[tileIndex].entityType, haEntities[tileIndex].entityName, soilVal, tempVal, battVal);
    } else {
        DrawTile(x, y, checkOnOffState(haEntities[tileIndex].entityID), haEntities[tileIndex].entityType, haEntities[tileIndex].entityName, "");
    }

    // Partial update only this tile area
    epd_update();
    Serial.println("Updated tile " + String(tileIndex) + ": " + haEntities[tileIndex].entityName);
}


void DrawHAScreen()
{
    epd_clear();

    DisplayStatusSection();
    DisplayGeneralInfoSection();
    Serial.println("Drawing (large icon) switchBar...");
    DrawSwitchBar();
    Serial.println("Drawing (small icon) sensorBar...");
    DrawSensorBar();
    Serial.println("Drawing (wide value) bottomBar...");
    DrawBottomBar();

    epd_update();

    // Initialize state tracking after full screen draw
    checkAndUpdateChangedTiles();
}

void InitialiseSystem() {
  StartTime = millis();
  Serial.begin(115200);
  while (!Serial);
  Serial.println(String(__FILE__) + "\nStarting...");
  epd_init();
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) Serial.println("Memory alloc failed!");
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  setFont(OpenSans9B);

  // Initialize touch screen using module
  initializeTouchScreen();
}

void BeginSleep() {
  epd_poweroff_all();
  SleepTimer = (SleepDuration * 60 - ((CurrentMin % SleepDuration) * 60 + CurrentSec));
  esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL); // in Secs, 1000000LL converts to Secs as unit = 1uSec
  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Entering " + String(SleepTimer) + " (secs) of sleep time");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}

void setup() {
  InitialiseSystem();

  if (StartWiFi() == WL_CONNECTED) {
      SetupTime();

      bool WakeUp = false;
      if (WakeupHour > SleepHour)
        WakeUp = (CurrentHour >= WakeupHour || CurrentHour <= SleepHour);
      else
        WakeUp = (CurrentHour >= WakeupHour && CurrentHour <= SleepHour);
      if (WakeUp) {
          DrawHAScreen();
      }
  }
  else {
    DrawWifiErrorScreen();
  }
  // BeginSleep(); // Deactivated for testing
}

void loop() {
  // Deep sleep disabled for testing - handle touch continuously
  static unsigned long lastSerial = 0;
  static unsigned long lastTileCheck = 0;
  static int loopCount = 0;

  handleTouch();

  loopCount++;

  // Check for tile state changes every 10 seconds
  if (millis() - lastTileCheck > 10000) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Checking for Home Assistant state changes...");
      checkAndUpdateChangedTiles();
      lastTileCheck = millis();
    } else {
      Serial.println("WiFi disconnected, skipping HA state check");
      lastTileCheck = millis();
    }
  }

  // Debug output every 5 seconds to keep serial alive
  if (millis() - lastSerial > 5000) {
    int nextCheck = (10000 - (millis() - lastTileCheck)) / 1000;
    Serial.println("Loop #" + String(loopCount) + " - WiFi: " + String(WiFi.status() == WL_CONNECTED ? "OK" : "FAIL") + ", Next check: " + String(nextCheck) + "s");
    lastSerial = millis();
  }

  delay(50);
}

