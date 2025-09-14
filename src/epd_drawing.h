#ifndef EPD_DRAWING_H
#define EPD_DRAWING_H

#include <Arduino.h>
#include "epd_driver.h"

enum alignment {LEFT, RIGHT, CENTER};

// External declarations
extern uint8_t *framebuffer;
extern GFXfont currentFont;

// Function declarations
void setFont(GFXfont const & font);
void drawString(int x, int y, String text, alignment align);
void fillCircle(int x, int y, int r, uint8_t color);
void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color);
void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawCircle(int x0, int y0, int r, uint8_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void drawPixel(int x, int y, uint8_t color);
void drawImage(int x, int y, int width, int height, const uint8_t* image_data);
void epd_update();

#endif // EPD_DRAWING_H
