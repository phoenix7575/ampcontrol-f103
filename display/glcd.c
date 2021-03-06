#include "glcd.h"

#include "../pins.h"

#define STR_BUFSIZE             20

static char strbuf[STR_BUFSIZE + 1];    // String buffer

uint8_t unRleData[512];           // Storage for uncompressed image
tImage imgUnRle = {
    .data = unRleData,
    .width = 0,
    .height = 0,
    .size = 0,
    .rle = 0
};

static GlcdDriver *glcd;

void glcdInit(GlcdDriver *driver)
{
    glcd = driver;
}

void glcdClear(void)
{
    if (glcd->clear) {
        glcd->clear();
    }
}

void glcdWriteNum(int32_t number, uint8_t width, uint8_t lead, uint8_t radix)
{
    uint8_t numdiv;
    uint8_t sign = lead;
    int8_t i;
    uint32_t num = (uint32_t)number;

    if (number < 0 && radix == 10) {
        sign = '-';
        num = -number;
    }

    for (i = 0; i < width; i++)
        strbuf[i] = lead;
    strbuf[width] = '\0';
    i = width - 1;

    while (num > 0 || i == width - 1) {
        numdiv = num % radix;
        strbuf[i] = numdiv + 0x30;
        if (numdiv >= 10)
            strbuf[i] += 7;
        i--;
        num /= radix;
    }

    if (i >= 0)
        strbuf[i] = sign;

    glcdWriteString(strbuf);
}

void glcdSetFont(const tFont *font)
{
    glcd->font.tfont = font;
}

void glcdSetFontColor(uint16_t color)
{
    glcd->font.color = color;
}

void glcdSetFontAlign(uint8_t align)
{
    glcd->font.align = align;
}

void glcdSetCanvasColor(uint16_t color)
{
    glcd->canvas->color = color;
}

void glcdSetXY(int16_t x, int16_t y)
{
    glcd->canvas->x = x;
    glcd->canvas->y = y;
}

void glcdSetX(int16_t x)
{
    glcd->canvas->x = x;
}
void glcdSetY(int16_t y)
{
    glcd->canvas->y = y;
}

static int16_t findSymbolPos(int32_t code)
{
    int16_t bPos = -1;

    const tFont *font = glcd->font.tfont;
    for (uint16_t i = 0; i < font->length; i++) {
        if (font->chars[i].code == code) {
            return i;
        }
        if (font->chars[i].code == BLOCK_CHAR) {
            bPos = i;
        }
    }

    return bPos;
}

void glcdDrawImage(tImage *img)
{
    uint16_t w = img->width;
    uint16_t h = img->height;
    uint16_t color = glcd->font.color;
    uint16_t bgColor = glcd->canvas->color;

    for (uint16_t j = 0; j < (h + 7) / 8; j++) {
        for (uint16_t i = 0; i < w; i++) {
            uint8_t data = img->data[w * j + i];
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (8 * j + bit < h) {
                    glcd->drawPixel(glcd->canvas->x + i, glcd->canvas->y + (8 * j + bit),
                                    data & (1 << bit) ? color : bgColor);
                }
            }
        }
    }
}

void glcdWriteIcon(uint8_t num, const uint8_t *icons)
{
    tImage img;
    img.data = 0;

    if (icons == icons_24) {
        img.width = img.height = 24;
    } else if (icons == icons_32) {
        img.width = img.height = 32;
    } else {
        return;
    }

    img.data = icons + (img.width * img.height / 8 * num);

    glcd->drawImage(&img);
}

void glcdWriteChar(int32_t code)
{
    tImage *img = 0;

    int16_t pos = findSymbolPos(code);

    if (pos < 0)
        return;

    img = (tImage *)glcd->font.tfont->chars[pos].image;

    if (img->rle) {
        // Uncompress image to storage
        const uint8_t *inPtr = img->data;
        uint8_t *outPtr = unRleData;

        while (inPtr < img->data + img->size) {
            int8_t size = (int8_t)(*inPtr);
            inPtr++;
            if (size < 0) {
                for (uint8_t i = 0; i < -size; i++) {
                    *outPtr++ = *inPtr++;
                }
            } else if (size > 0) {
                uint8_t data = *inPtr;
                for (uint8_t i = 0; i < size; i++) {
                    *outPtr++ = data;
                }
                inPtr++;
            } else {
                return;
            }
        }
        imgUnRle.width = img->width;
        imgUnRle.height = img->height;
        imgUnRle.size = outPtr - unRleData;

        glcd->drawImage(&imgUnRle);
    } else {
        glcd->drawImage(img);
    }

    glcdSetX(glcd->canvas->x + img->width);
}

static int32_t findSymbolCode(char **string)
{
    int32_t code = 0;
    char sym;
    uint8_t curr = 0;

    char *str = *string;

    while (*str) {
        sym = *str++;

        if ((sym & 0xC0) == 0x80) {         // Not first byte
            code <<= 8;
            code |= sym;
            if (curr) {
                curr--;
            }
        } else {
            code = sym;
            if ((sym & 0x80) == 0x00) {         // one-byte symbol
                curr = 0;
            } else if ((sym & 0xE0) == 0xC0) {  // two-byte symbol
                curr = 1;
            } else if ((sym & 0xF0) == 0xE0) {  // three-byte symbol
                curr = 2;
            } else if ((sym & 0xF8) == 0xF0) {  // four-byte symbol
                curr = 3;
            } else {
                curr = 0;
            }
        }
        if (curr)
            continue;

        *string = str;
        return code;
    }

    *string = 0;
    return BLOCK_CHAR;
}

void glcdWriteString(char *string)
{
    int32_t code = 0;
    char *str = string;

    const tFont *font = glcd->font.tfont;

    if (glcd->font.align != FONT_ALIGN_LEFT) {
        uint16_t strLength = 0;
        int16_t pos = findSymbolPos(LETTER_SPACE_CHAR);
        uint16_t sWidth = font->chars[pos].image->width;

        while (*str) {
            code = findSymbolCode(&str);
            pos = findSymbolPos(code);
            strLength += font->chars[pos].image->width;
            if (*str) {
                strLength += sWidth;
            }
        }

        if (glcd->font.align == FONT_ALIGN_CENTER) {
            glcdSetX(glcd->canvas->x - strLength / 2);
        } else if (glcd->font.align == FONT_ALIGN_RIGHT) {
            glcdSetX(glcd->canvas->x - strLength);
        }

        // Reset align after string finished
        glcd->font.align = FONT_ALIGN_LEFT;
    }

    str = string;

    while (*str) {
        code = findSymbolCode(&str);

        glcdWriteChar(code);
        if (*str)
            glcdWriteChar(LETTER_SPACE_CHAR);
    }
}

void glcdDrawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (glcd->drawRectangle) {
        glcd->drawRectangle(x, y, w, h, color);
    } else {
        for (int16_t i = 0; i < w; i++) {
            for (int16_t j = 0; j < h; j++) {
                glcd->drawPixel(x + i, y + j, color);
            }
        }
    }
}

void glcdDrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    if (x0 == x1) {                 // Vertical
        if (y0 > y1) {              // Swap
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }
        glcdDrawRect(x0, y0, 1, y1 - y0 + 1, color);
    } else if (y0 == y1) {          // Horisontal
        if (x0 > x1) {              // Swap
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;
        }
        glcdDrawRect(x0, y0, x1 - x0 + 1, 1, color);
    } else {
        int16_t sX, sY, dX, dY, err, err2;

        sX = x0 < x1 ? 1 : -1;
        sY = y0 < y1 ? 1 : -1;
        dX = sX > 0 ? x1 - x0 : x0 - x1;
        dY = sY > 0 ? y1 - y0 : y0 - y1;
        err = dX - dY;

        while (x0 != x1 || y0 != y1) {
            glcd->drawPixel(x0, y0, color);
            err2 = err * 2;
            if (err2 > -dY / 2) {
                err -= dY;
                x0 += sX;
            }
            if (err2 < dX) {
                err += dX;
                y0 += sY;
            }
        }
        glcd->drawPixel(x1, y1, color);
    }
}

void glcdDrawFrame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    glcdDrawLine(x0, y0, x0, y1, color);
    glcdDrawLine(x0, y1, x1, y1, color);
    glcdDrawLine(x1, y0, x1, y1, color);
    glcdDrawLine(x0, y0, x1, y0, color);
}

void glcdDrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    glcdDrawLine(x0 - r, y0, x0 + r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        glcdDrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        glcdDrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
        glcdDrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
        glcdDrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
    }
}

void glcdDrawRing(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    glcd->drawPixel(x0, y0 + r, color);
    glcd->drawPixel(x0, y0 - r, color);
    glcd->drawPixel(x0 + r, y0, color);
    glcd->drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        glcd->drawPixel(x0 + x, y0 + y, color);
        glcd->drawPixel(x0 - x, y0 + y, color);
        glcd->drawPixel(x0 + x, y0 - y, color);
        glcd->drawPixel(x0 - x, y0 - y, color);

        glcd->drawPixel(x0 + y, y0 + x, color);
        glcd->drawPixel(x0 - y, y0 + x, color);
        glcd->drawPixel(x0 + y, y0 - x, color);
        glcd->drawPixel(x0 - y, y0 - x, color);
    }
}

void glcdUpdate(void)
{
    if (glcd->updateFB) {
        glcd->updateFB();
    }
}
