#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include "../common/definitions.h"

// Drawing functions
void DrawBattery(int x, int y, uint8_t percentage);
void DrawBattery(int x, int y);
void DrawRSSI(int x, int y, int rssi);
void DrawTile(int x, int y, int width, int height, const uint8_t *image_data, String label, String state);
void DrawTile(int x, int y, int state, int type, String name, String value);
void DrawTileHigrow(int x, int y, int width, int height, const uint8_t *image_data, String label, String soil, String temp, String batt);
void DrawTileHigrow(int x, int y, int state, int type, String name, String soil, String temp, String batt);
void DrawSensorTile(int x, int y, int width, int height, const uint8_t* image_data, String label);
void DrawSensorTile(int x, int y, int state, int type, String name);
void DrawTempSensorTile(int x, int y, float temp, String label);
void DrawBottomTile(int x, int y, String value, String name);

// Screen sections
void DrawSwitchBar();
void DrawSensorBar();
void DrawBottomBar();
void DisplayStatusSection();
void DisplayGeneralInfoSection();

// Full screen functions
void DrawHAScreen();
void DrawWifiErrorScreen();

// Selective update functions
void updateSingleTile(int tileIndex);

// Visual feedback
void showTouchFeedback(int x, int y);

#endif // DISPLAY_UI_H