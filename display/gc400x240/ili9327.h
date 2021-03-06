#ifndef ILI9327_H
#define ILI9327_H

#include <inttypes.h>

#include "gc400x240.h"

void ili9327Init(GlcdDriver **driver);
void ili9327Clear(void);
void ili9327BusIRQ(void);

void ili9327Sleep(void);
void ili9327Wakeup(void);

void ili9327DrawPixel(int16_t x, int16_t y, uint16_t color);
void ili9327DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

void ili9327DrawImage(tImage *img);

#endif // ILI9327_H
