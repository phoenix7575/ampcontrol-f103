#include "gc176x132.h"

static GlcdDriver *glcd;

static void displayTm(RTC_type *rtc, uint8_t tm)
{
    int8_t time = *((int8_t *)rtc + tm);

    glcdSetFontColor(LCD_COLOR_AQUA);
    if (rtc->etm == tm)
        glcdSetFontColor(LCD_COLOR_YELLOW);
    glcdWriteChar(LETTER_SPACE_CHAR);
    if (tm == RTC_YEAR) {
        glcdWriteString("20");
        glcdWriteChar(LETTER_SPACE_CHAR);
    }
    glcdWriteNum(time, 2, '0', 10);
    glcdWriteChar(LETTER_SPACE_CHAR);
    glcdSetFontColor(LCD_COLOR_AQUA);
    glcdWriteChar(LETTER_SPACE_CHAR);
}

static void displayShowBar(int16_t min, int16_t max, int16_t value)
{
    static const int16_t sc = 46; // Scale count
    static const uint8_t sw = 1; // Scale width

    if (min + max) { // Non-symmectic scale => rescale to 0..sl
        value = sc * (value - min) / (max - min);
    } else { // Symmetric scale => rescale to -sl/2..sl/2
        value = (sc / 2) * value / max;
    }

    for (uint16_t i = 0; i < sc; i++) {
        uint16_t color = LCD_COLOR_WHITE;

        if (min + max) { // Non-symmetric scale
            if (i >= value) {
                color = glcd->canvas->color;
            }
        } else { // Symmetric scale
            if ((value > 0 && i >= value + (sc / 2)) ||
                (value >= 0 && i < (sc / 2 - 1)) ||
                (value < 0 && i < value + (sc / 2)) ||
                (value <= 0 && i > (sc / 2))) {
                color = glcd->canvas->color;
            }
        }

        glcdDrawRect(i * (glcd->canvas->width / sc) + 1, 27, sw, 5, color);
        glcdDrawRect(i * (glcd->canvas->width / sc) + 1, 32, sw, 1, LCD_COLOR_WHITE);
        glcdDrawRect(i * (glcd->canvas->width / sc) + 1, 33, sw, 5, color);
    }
}

static void drawSpCol(uint16_t xbase, uint16_t ybase, uint8_t width, uint16_t value, uint16_t max)
{
    if (value > max)
        value = max;

    glcdDrawRect(xbase, ybase - value, width, value, LCD_COLOR_AQUA);
    glcdDrawRect(xbase, ybase - max, width, max - value, LCD_COLOR_BLACK);
}

static void showTime(RTC_type *rtc, char *wday)
{
    glcdSetXY(2, 4);
    glcdSetFont(&fontterminus24b);

    displayTm(rtc, RTC_HOUR);
    glcdWriteChar(':');
    displayTm(rtc, RTC_MIN);
    glcdWriteChar(':');
    displayTm(rtc, RTC_SEC);

    glcdSetXY(12, 64);
    glcdSetFont(&fontterminusdig30);

    displayTm(rtc, RTC_DATE);
    glcdWriteChar('.');
    displayTm(rtc, RTC_MONTH);
    glcdWriteChar('.');
    displayTm(rtc, RTC_YEAR);

    glcdSetFont(&fontterminus24b);
    glcdSetFontColor(LCD_COLOR_AQUA);
    glcdSetXY(24, 104);
    glcdWriteString(wday);
}

static void showParam(DispParam *dp)
{
    glcdSetFont(&fontterminus24b);
    glcdSetFontColor(LCD_COLOR_WHITE);

    glcdSetXY(0, 0);
    glcdWriteString((char *)dp->label);

    displayShowBar(dp->min, dp->max, dp->value);

    glcdSetXY(128, 30);
    glcdSetFontAlign(FONT_ALIGN_RIGHT);
    glcdWriteNum((dp->value * dp->step) / 8, 3, ' ', 10);

    glcdSetXY(104, 2);
    glcdWriteIcon(dp->icon, icons_24);
}

static void showSpectrum(SpectrumData *spData)
{
    uint8_t *buf;

    buf = spData[SP_CHAN_LEFT].show;
    for (uint16_t x = 0; x < (glcd->canvas->width + 1) / 2; x++) {
        uint16_t xbase = x * 2;
        uint16_t ybase = 66;
        uint16_t width = 1;
        uint16_t value = buf[x];
        uint16_t max = 65;

        drawSpCol(xbase, ybase, width, value + 1, max);
    }

    buf = spData[SP_CHAN_RIGHT].show;
    for (uint16_t x = 0; x < (glcd->canvas->width + 1) / 2; x++) {
        uint16_t xbase = x * 2;
        uint16_t ybase = 132;
        uint16_t width = 1;
        uint16_t value = buf[x];
        uint16_t max = 65;

        drawSpCol(xbase, ybase, width, value + 1, max);
    }
}

GlcdCanvas gc176x132 = {
    .width = 176,
    .height = 132,

    .showTime = showTime,
    .showParam = showParam,
    .showSpectrum = showSpectrum,
};

void gc176x132Init(GlcdDriver *driver)
{
    glcd = driver;
    glcd->canvas = &gc176x132;
    glcdInit(glcd);
}
