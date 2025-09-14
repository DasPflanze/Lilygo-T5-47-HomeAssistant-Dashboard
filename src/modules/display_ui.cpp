#include "display_ui.h"
#include "../configurations.h"
#include "homeassistantapi.h"
#include "epd_drawing.h"
#include <NTPClient.h>

// Include all icon headers
#include "../icons/waterheateron.h"
#include "../icons/waterheateroff.h"
#include "../icons/lightbulbon.h"
#include "../icons/lightbulboff.h"
#include "../icons/exhaustfanon.h"
#include "../icons/exhaustfanoff.h"
#include "../icons/fanoff.h"
#include "../icons/fanon.h"
#include "../icons/airpurifieron.h"
#include "../icons/airpurifieroff.h"
#include "../icons/plugon.h"
#include "../icons/plugoff.h"
#include "../icons/switchon.h"
#include "../icons/switchoff.h"
#include "../icons/airconditioneron.h"
#include "../icons/airconditioneroff.h"
#include "../icons/warning.h"
#include "../icons/plantwateringlow.h"
#include "../icons/plantwateringok.h"
#include "../icons/batteryempty.h"
#include "../icons/dooropen.h"
#include "../icons/doorclosed.h"
#include "../icons/windowopen.h"
#include "../icons/windowclosed.h"
#include "../icons/motionsensoron.h"
#include "../icons/motionsensoroff.h"
#include "../icons/sensorerror.h"

// External variables
extern int vref;
extern int wifi_signal;
extern NTPClient timeClient;
extern String formattedDate;
extern String dateStamp;
extern String timeStamp;
extern String str_unavail;

void DrawBattery(int x, int y, uint8_t percentage) {
    drawRect(x + 55, y - 15, 40, 15, Black);
    fillRect(x + 95, y - 9, 4, 6, Black);
    if (percentage > 0)
        fillRect(x + 57, y - 13, 36 * percentage / 100.0, 11, Black);
}

void DrawBattery(int x, int y) {
    uint8_t percentage = 100;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
    float voltage = analogRead(36) / 4096.0 * 6.566 * (vref / 1000.0);
    if (voltage > 1) {
        Serial.println("\\nVoltage = " + String(voltage));
        percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
        if (voltage >= 4.20) percentage = 100;
        if (voltage <= 3.20) percentage = 0;
        DrawBattery(x, y, percentage);
        drawString(x, y, String(percentage) + "%", LEFT);
        drawString(x + 130, y, String(voltage, 2) + "v", CENTER);
    }
}

void DrawRSSI(int x, int y, int rssi) {
    int WIFIsignal = 0;
    int xpos = 1;
    for (int _rssi = -100; _rssi <= rssi; _rssi = _rssi + 20) {
        if (_rssi <= -20)  WIFIsignal = 20;
        if (_rssi <= -40)  WIFIsignal = 16;
        if (_rssi <= -60)  WIFIsignal = 12;
        if (_rssi <= -80)  WIFIsignal = 8;
        if (_rssi <= -100) WIFIsignal = 4;

        if (rssi != 0)
            fillRect(x + xpos * 8, y - WIFIsignal, 6, WIFIsignal, Black);
        else
            drawRect(x + xpos * 8, y - WIFIsignal, 6, WIFIsignal, Black);
        xpos++;
    }
    if (rssi == 0)
        drawString(x, y, "x", LEFT);
}

void showTouchFeedback(int x, int y) {
    Serial.println("Touch feedback at: " + String(x) + ", " + String(y));
    drawRect(x-2, y-2, TILE_WIDTH+2, TILE_HEIGHT+2, Black);
    epd_update();
    delay(200);
}

// More functions to be added in subsequent files...